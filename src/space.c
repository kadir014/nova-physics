/*

  This file is a part of the Nova Physics Engine
  project and distributed under the MIT license.

  Copyright © Kadir Aksoy
  https://github.com/kadir014/nova-physics

*/

#include "novaphysics/internal.h"
#include "novaphysics/space.h"
#include "novaphysics/constants.h"
#include "novaphysics/body.h"
#include "novaphysics/collision.h"
#include "novaphysics/contact.h"
#include "novaphysics/contact_solver.h"
#include "novaphysics/math.h"
#include "novaphysics/constraint.h"
#include "novaphysics/narrowphase.h"
#include "novaphysics/debug.h"
#include "novaphysics/space_step.h"


/**
 * @file space.c
 * 
 * @brief Space struct and its methods.
 */


#define ITER_BODIES(iter) for (size_t iter = 0; iter < space->bodies->size; iter++)


nvSpace *nvSpace_new() {
    nvSpace *space = NV_NEW(nvSpace);
    if (!space) return NULL;

    space->bodies = nvArray_new();
    space->constraints = nvArray_new();
    space->removed_bodies = nvArray_new();

    nvSpace_set_gravity(space, NV_VEC2(0.0, NV_GRAV_EARTH));

    space->settings = (nvSpaceSettings){
        .baumgarte = 0.2,
        .penetration_slop = 0.05,
        .position_correction = nvPositionCorrection_BAUMGARTE,
        .velocity_iterations = 8,
        .position_correction = 4,
        .substeps = 1,
        .linear_damping = 0.002,
        .angular_damping = 0.002,
        .warmstarting = true,
        .sleeping = false,
        .restitution_mix = nvCoefficientMix_SQRT,
        .friction_mix = nvCoefficientMix_SQRT
    };

    nvSpace_set_broadphase(space, nvBroadPhaseAlg_SHG);

    space->broadphase_pairs = nvArray_new();
    space->contacts = nvHashMap_new(
        sizeof(nvPersistentContactPair), 0, nvPersistentContactPair_hash);

    space->kill_bounds = (nvAABB){-1e4, -1e4, 1e4, 1e4};
    space->use_kill_bounds = true;

    nvProfiler_reset(&space->profiler);

    space->id_counter = 0;

    return space;
}

void nvSpace_free(nvSpace *space) {
    if (!space) return;

    nvSpace_clear(space, true);
    nvArray_free(space->bodies);
    nvArray_free(space->constraints);

    free(space);
}

void nvSpace_set_gravity(nvSpace *space, nvVector2 gravity) {
    space->gravity = gravity;
}

nvVector2 nvSpace_get_gravity(const nvSpace *space) {
    return space->gravity;
}

void nvSpace_set_broadphase(nvSpace *space, nvBroadPhaseAlg broadphase_alg_type) {
    switch (broadphase_alg_type) {
        case nvBroadPhaseAlg_BRUTE_FORCE:
            space->broadphase_algorithm = nvBroadPhaseAlg_BRUTE_FORCE;
            return;

        case nvBroadPhaseAlg_SHG:
            space->broadphase_algorithm = nvBroadPhaseAlg_SHG;

            // Default SHG configuration
            nvAABB bounds = {.min_x=0.0, .min_y=0.0, .max_x=128.0, .max_y=72.0};
            nv_float cell_size = 3.5;

            //nvSpace_set_SHG(space, bounds, cell_size, cell_size);

            return;

        case nvBroadPhaseAlg_BVH:
            space->broadphase_algorithm = nvBroadPhaseAlg_BVH;
            return;
    }
}

nvBroadPhaseAlg nvSpace_get_broadphase(const nvSpace *space) {
    return space->broadphase_algorithm;
}

int nvSpace_clear(nvSpace *space, nv_bool free_all) {
    if (free_all) {
        if (nvArray_clear(space->bodies, nvRigidBody_free)) return 1;
        //if (nvArray_clear(space->constraints, nvConstraint_free)) return 1;
    }
    else {
        if (nvArray_clear(space->bodies, NULL)) return 1;
        if (nvArray_clear(space->constraints, NULL)) return 1;
    }
    return 0;
}

int nvSpace_add_body(nvSpace *space, nvRigidBody *body) {
    if (body->space == space) {
        nv_set_error("Can't add same body to same space more than once.");
        return 2;
    }

    if (nvArray_add(space->bodies, body))
        return 1;

    body->space = space;
    body->id = space->id_counter++;

    return 0;
}

int nvSpace_remove_body(nvSpace *space, nvRigidBody *body) {
    return nvArray_add(space->removed_bodies, body);
}

int nvSpace_add_constraint(nvSpace *space, nvConstraint *cons) {
    return nvArray_add(space->constraints, cons);
}

void nvSpace_step(nvSpace *space, nv_float dt) {
    if (dt == 0.0 || space->settings.substeps <= 0) return;
    nv_uint32 substeps = space->settings.substeps;
    nv_uint32 velocity_iters = space->settings.velocity_iterations;
    nv_uint32 position_iters = space->settings.position_iterations;

    /*
        Simulation route
        ----------------
        1. Broadphase
        2. Narrowphase
        3. Integrate accelerations
        4. Solve contact velocity constraints (PGS [+ Baumgarte])
        5. Solve joint constraints (PGS + Baumgarte)
        6. Integrate velocities
        7. Contact position correction (NGS)
        8. Rest bodies
    */

    NV_TRACY_ZONE_START;

    nvPrecisionTimer step_timer;
    NV_PROFILER_START(step_timer);

    nvPrecisionTimer timer;

    size_t i, j, l;
    void *map_val;

    dt /= (nv_float)substeps;
    nv_float inv_dt = 1.0 / dt;

    for (nv_uint32 substep = 0; substep < substeps; substep++) {
        /*
            Broadphase
            ----------
            Generate possible collision pairs with the choosen broadphase algorithm.
        */
        NV_PROFILER_START(timer);
        switch (space->broadphase_algorithm) {
            case nvBroadPhaseAlg_BRUTE_FORCE:
                nv_broad_phase_brute_force(space);
                break;

            case nvBroadPhaseAlg_SHG:
                nv_broad_phase_SHG(space);
                break;

            case nvBroadPhaseAlg_BVH:
                nv_broad_phase_BVH(space);
                break;
        }
        NV_PROFILER_STOP(timer, space->profiler.broadphase);

        /*
            Narrowphase
            ------------
            Do narrow-phase checks between possible collision pairs and
            update contacts.
        */
        NV_PROFILER_START(timer);
        nv_narrow_phase(space);
        NV_PROFILER_STOP(timer, space->profiler.narrowphase);

        /*
            Integrate accelerations
            -----------------------
            Apply forces, gravity, integrate accelerations (update velocities) and apply damping.
        */
        NV_PROFILER_START(timer);
        ITER_BODIES(body_i) {
            nvRigidBody *body = (nvRigidBody *)space->bodies->data[body_i];

            if (body->type != nvRigidBodyType_STATIC) {
                body->cache_aabb = false;
                body->cache_transform = false;
            }

            nvRigidBody_integrate_accelerations(body, space->gravity, dt);
        }
        NV_PROFILER_STOP(timer, space->profiler.integrate_accelerations);

        // /*
        //     PGS / Projected Gauss-Seidel
        //     ----------------------------
        //     Prepare contact velocity constraints, warm-start and solve iteratively
        //     Use baumgarte depending on the position correction setting
        // */

        // // Prepare for solving contact constraints
        // l = 0;
        // NV_PROFILER_START(timer);
        // while (nvHashMap_iter(space->res, &l, &map_val)) {
        //     nvResolution *res = map_val;
        //     if (res->state == nvResolutionState_CACHED) continue;
        //     nv_presolve_contact(space, res, inv_dt);
        // }

        // // Apply accumulated impulses
        // l = 0;
        // while (nvHashMap_iter(space->res, &l, &map_val)) {
        //     nvResolution *res = map_val;
        //     if (res->state == nvResolutionState_CACHED) continue;
        //     nv_warmstart(space, res);
        // }
        // NV_PROFILER_STOP(timer, space->profiler.presolve_collisions);

        // // Solve velocity constraints iteratively
        // NV_PROFILER_START(timer);
        // for (i = 0; i < velocity_iters; i++) {
        //     l = 0;
        //     while (nvHashMap_iter(space->res, &l, &map_val)) {
        //         nvResolution *res = map_val;
        //         if (res->state == nvResolutionState_CACHED) continue;
        //         nv_solve_velocity(res);
        //     }
        // }
        // NV_PROFILER_STOP(timer, space->profiler.solve_velocities);

        // /*
        //     Solve joint constraints (PGS + Baumgarte)
        //     -----------------------------------
        //     Solve joint constraints iteratively, same as contact constraints.
        // */

        // // Prepare joint constraints for solving
        // NV_PROFILER_START(timer);
        // for (i = 0; i < space->constraints->size; i++) {
        //     nvConstraint_presolve(
        //         space,
        //         (nvConstraint *)space->constraints->data[i],
        //         inv_dt
        //     );
        // }
        // NV_PROFILER_STOP(timer, space->profiler.presolve_constraints);

        // // Solve joint constraints iteratively
        // NV_PROFILER_START(timer);
        // for (i = 0; i < constraint_iters; i++) {
        //     for (j = 0; j < space->constraints->size; j++) {
        //         nvConstraint_solve(
        //             (nvConstraint *)space->constraints->data[j],
        //             inv_dt
        //         );
        //     }
        // }
        // NV_PROFILER_STOP(timer, space->profiler.solve_constraints);

        /*
            Integrate velocities
            --------------------
            Integrate velocities (update positions) and check out-of-bound bodies.
        */
        NV_PROFILER_START(timer);
        ITER_BODIES(body_i) {
            nvRigidBody *body = (nvRigidBody *)space->bodies->data[body_i];

            nvRigidBody_integrate_velocities(body, dt);

            /*
                Assuming the dynamic bodies are small enough, we just check
                their positions against the kill boundaries.
                TODO: Check body AABB against boundary.
            */
            // if (
            //     space->use_kill_bounds &&
            //     !nv_collide_aabb_x_point(space->kill_bounds, body->position)
            // ) {
            //     nvSpace_remove_body(space, body);
            // }
        }
        NV_PROFILER_STOP(timer, space->profiler.integrate_velocities);

        // /*
        //     NGS / Non-Linear Gauss-Seidel
        //     -----------------------------
        //     If enabled, solve position (penetration) error with pseudo-velocities.
        // */
        // NV_PROFILER_START(timer);
        // if (space->position_correction == nvPositionCorrection_NGS) {
        //     for (i = 0; i < position_iters; i++) {
        //         l = 0;
        //         while (nvHashMap_iter(space->res, &l, &map_val)) {
        //             nvResolution *res = map_val;
        //             if (res->state == nvResolutionState_CACHED) continue;
        //             nv_solve_position(res);
        //         }
        //     }
        // }
        // NV_PROFILER_STOP(timer, space->profiler.solve_positions);
    }

    // // Now that the simulation is done we can remove bodies. 

    // NV_PROFILER_START(timer);

    // for (i = 0; i < space->removed_bodies->size; i++) {
    //     nvRigidBody *body = (nvRigidBody *)space->removed_bodies->data[i];

    //     l = 0;
    //     while (nvHashMap_iter(space->res, &l, &map_val)) {
    //         nvResolution *res = map_val;
    //         if (res->a == body) {
    //             nvHashMap_remove(space->res, res);
    //             l = 0;
    //         }
    //         else if (res->b == body) {
    //             nvHashMap_remove(space->res, res);
    //             l = 0;
    //         }
    //     }

    //     nvArray_remove(space->bodies, body);
    // }

    nvArray_clear(space->removed_bodies, NULL);

    NV_PROFILER_STOP(timer, space->profiler.remove_bodies);
    NV_PROFILER_STOP(step_timer, space->profiler.step);

    NV_TRACY_ZONE_END;
    NV_TRACY_FRAMEMARK;
}