/*

  This file is a part of the Nova Physics Engine
  project and distributed under the MIT license.

  Copyright © Kadir Aksoy
  https://github.com/kadir014/nova-physics

*/

#include <stdlib.h>
#include "novaphysics/internal.h"
#include "novaphysics/space.h"
#include "novaphysics/constants.h"
#include "novaphysics/body.h"
#include "novaphysics/collision.h"
#include "novaphysics/contact.h"
#include "novaphysics/solver.h"
#include "novaphysics/math.h"
#include "novaphysics/constraint.h"
#include "novaphysics/debug.h"
#include "novaphysics/space_step.h"


/**
 * @file space.c
 * 
 * @brief Space struct and its methods.
 */


nvSpace *nvSpace_new() {
    nvSpace *space = NV_NEW(nvSpace);
    if (!space) return NULL;

    space->bodies = nvArray_new();
    space->attractors = nvArray_new();
    space->constraints = nvArray_new();

    space->_removed_bodies = nvArray_new();
    space->_killed_bodies = nvArray_new();

    space->res = nvHashMap_new(sizeof(nvResolution), 0, _nvSpace_resolution_hash);

    space->gravity = NV_VEC2(0.0, NV_GRAV_EARTH);

    space->sleeping = false;
    space->sleep_energy_threshold = 0.02;
    space->wake_energy_threshold = space->sleep_energy_threshold / 1.3;
    space->sleep_timer_threshold = 60;

    space->warmstarting = true;
    space->baumgarte = NV_BAUMGARTE;
    space->collision_persistence = NV_COLLISION_PERSISTENCE;

    space->pairs = nvHashMap_new(sizeof(nvBroadPhasePair), 0, _nvSpace_broadphase_pair_hash);
    nvSpace_set_broadphase(space, nvBroadPhaseAlg_SPATIAL_HASH_GRID);

    space->kill_bounds = (nvAABB){-1e4, -1e4, 1e4, 1e4};
    space->use_kill_bounds = true;

    space->mix_restitution = nv_CoefficientMix_SQRT;
    space->mix_friction = nv_CoefficientMix_SQRT;

    space->callback_user_data = NULL;
    space->before_collision = NULL;
    space->after_collision = NULL;

    nvProfiler_reset(&space->profiler);
    #ifdef NV_WINDOWS
    nv_set_windows_timer_resolution();
    #endif

    space->multithreading = false;
    space->res_mutex = nvMutex_new();

    space->_id_counter = 0;

    return space;
}

void nvSpace_free(nvSpace *space) {
    nvSpace_clear(space);
    nvArray_free(space->bodies);
    nvArray_free(space->attractors);
    nvArray_free(space->constraints);
    nvHashMap_free(space->res);
    nvHashMap_free(space->pairs);

    space->gravity = nvVector2_zero;
    space->sleeping = false;
    space->sleep_energy_threshold = 0.0;
    space->sleep_timer_threshold = 0.0;
    space->warmstarting = false;
    space->baumgarte = 0.0;
    space->callback_user_data = NULL;
    space->before_collision = NULL;
    space->after_collision = NULL;

    free(space);
}

void nvSpace_set_broadphase(nvSpace *space, nvBroadPhaseAlg broadphase_alg_type) {
    if (space->broadphase_algorithm == nvBroadPhaseAlg_SPATIAL_HASH_GRID)
        nvSHG_free(space->shg);
    
    switch (broadphase_alg_type) {
        case nvBroadPhaseAlg_BRUTE_FORCE:
            space->broadphase_algorithm = nvBroadPhaseAlg_BRUTE_FORCE;
            return;

        case nvBroadPhaseAlg_SPATIAL_HASH_GRID:
            space->broadphase_algorithm = nvBroadPhaseAlg_SPATIAL_HASH_GRID;
            space->shg = nvSHG_new((nvAABB){0, 0, 128.0, 72.0}, 3.5, 3.5);
    }
}

void nvSpace_set_SHG(
    nvSpace *space,
    nvAABB bounds,
    nv_float cell_width,
    nv_float cell_height
) {
    nvSHG_free(space->shg);

    space->shg = nvSHG_new(bounds, cell_width, cell_height);
}

void nvSpace_clear(nvSpace *space) {
    while (space->bodies->size > 0) {
        nvBody_free(nvArray_pop(space->bodies, 0));
    }

    while (space->attractors->size > 0) {
        // Don't free individual attractors because they are freed before
        nvArray_pop(space->attractors, 0);
    }

    while (space->constraints->size > 0) {
        nvConstraint_free(nvArray_pop(space->constraints, 0));
    }

    nvHashMap_clear(space->res);

    /*
        We can set array->max to 0 and reallocate but
        not doing it might be more efficient for the developer
        since they will probably fill the array up again.
        Maybe a separate parameter for this?
    */
}

void nvSpace_add(nvSpace *space, nvBody *body) {
    NV_ASSERT(body->space != space, "You can't add the same body to the same space multiple times.");

    nvArray_add(space->bodies, body);
    body->space = space;
    body->id = space->_id_counter;
    space->_id_counter++;
}

void nvSpace_remove(nvSpace *space, nvBody *body) {
    nvArray_add(space->_removed_bodies, body);
}

void nvSpace_kill(nvSpace *space, nvBody *body) {
    nvArray_add(space->_killed_bodies, body);
}

void nvSpace_add_constraint(nvSpace *space, nvConstraint *cons) {
    nvArray_add(space->constraints, cons);
}

void nvSpace_step(
    nvSpace *space,
    nv_float dt,
    int velocity_iters,
    int position_iters,
    int constraint_iters,
    int substeps
) {
    /*
        Simulation route
        ----------------
        1. Integrate accelerations
        2. Broad-phase & Narrow-phase
        3. Solve collision constraints
        4. Solve constraints
        5. Integrate velocities
        6. Rest bodies


        Nova Physics uses semi-implicit Euler integration:

        Linear:
        v = a * Δt
        x = v * Δt

        Angular:
        ω = α * Δt
        θ = ω * Δt
    */

    NV_TRACY_ZONE_START;
    
    size_t n = space->bodies->size;

    size_t i, j, k, l;
    void *map_val;

    dt /= (nv_float)substeps;
    nv_float inv_dt = 1.0 / dt;

    nvPrecisionTimer step_timer;
    nvPrecisionTimer timer;

    nvPrecisionTimer_start(&step_timer);

    for (k = 0; k < substeps; k++) {

        /*
            1. Integrate accelerations
            --------------------------
            Apply forces, gravity, integrate accelerations (update velocities) and apply damping.
        */
        nvPrecisionTimer_start(&timer);
        #if defined(NV_AVX) && defined(NV_USE_SIMD)

            _nvSpace_integrate_accelerations_AVX(
                space,
                dt
            );

        #else

            for (i = 0; i < n; i++) {
                _nvSpace_integrate_accelerations(space, dt, i);
            }

        #endif
        space->profiler.integrate_accelerations = nvPrecisionTimer_stop(&timer);

        /*
            2. Broad-phase & Narrow-phase
            -----------------------------
            Generate possible collision pairs with the choosen broad-phase
            algorithm and create collision resolutions with the more
            expensive narrow-phase calculations.
        */
        nvPrecisionTimer_start(&timer);
        switch (space->broadphase_algorithm) {
            case nvBroadPhaseAlg_BRUTE_FORCE:
                nvBroadPhase_brute_force(space);
                break;

            case nvBroadPhaseAlg_SPATIAL_HASH_GRID:
                if (space->multithreading)
                    nvBroadPhase_SHG_multithreaded(space);

                else
                    nvBroadPhase_SHG(space);
                    
                break;
        }
        space->profiler.broadphase = nvPrecisionTimer_stop(&timer);

        /*
            3. Solve collisions
            -------------------
            Solve collisions and apply sequential impulses.
        */

        // Call callback before resolving collisions
        if (space->before_collision != NULL)
            space->before_collision(space->res, space->callback_user_data);

        // Prepare for solving collision constraints
        l = 0;
        nvPrecisionTimer_start(&timer);
        while (nvHashMap_iter(space->res, &l, &map_val)) {
            nvResolution *res = map_val;
            if (res->state == nvResolutionState_CACHED) continue;
            nv_presolve_collision(space, res, inv_dt);
        }

        // Apply accumulated impulses
        l = 0;
        while (nvHashMap_iter(space->res, &l, &map_val)) {
            nvResolution *res = map_val;
            if (res->state == nvResolutionState_CACHED) continue;
            nv_warmstart(space, res);
        }
        space->profiler.presolve_collisions = nvPrecisionTimer_stop(&timer);

        // Solve positions (pseudo-velocities) constraints iteratively
        nvPrecisionTimer_start(&timer);
        for (i = 0; i < position_iters; i++) {
            l = 0;
            while (nvHashMap_iter(space->res, &l, &map_val)) {
                nvResolution *res = map_val;
                if (res->state == nvResolutionState_CACHED) continue;
                nv_solve_position(res);
            }
        }
        space->profiler.solve_positions = nvPrecisionTimer_stop(&timer);

        // Solve velocity constraints iteratively
        nvPrecisionTimer_start(&timer);
        for (i = 0; i < velocity_iters; i++) {
            l = 0;
            while (nvHashMap_iter(space->res, &l, &map_val)) {
                nvResolution *res = map_val;
                if (res->state == nvResolutionState_CACHED) continue;
                nv_solve_velocity(res);
            }
        }
        space->profiler.solve_velocities = nvPrecisionTimer_stop(&timer);

        // Call callback after resolving collisions
        if (space->after_collision != NULL)
            space->after_collision(space->res, space->callback_user_data);

        /*
            4. Solve constraints
            --------------------
            Solve constraints and apply sequential impulses.
        */

        // Prepare constraints for solving
        nvPrecisionTimer_start(&timer);
        for (i = 0; i < space->constraints->size; i++) {
            nv_presolve_constraint(
                space,
                (nvConstraint *)space->constraints->data[i],
                inv_dt
            );
        }
        space->profiler.presolve_constraints = nvPrecisionTimer_stop(&timer);

        // Solve constraints iteratively
        nvPrecisionTimer_start(&timer);
        for (i = 0; i < constraint_iters; i++) {
            for (j = 0; j < space->constraints->size; j++) {
                nv_solve_constraint(
                    (nvConstraint *)space->constraints->data[j],
                    inv_dt
                );
            }
        }
        space->profiler.solve_constraints = nvPrecisionTimer_stop(&timer);

        /*
            5. Integrate velocities
            -----------------------
            Integrate velocities (update positions) and check out-of-bound bodies.
        */
        nvPrecisionTimer_start(&timer);
        #if defined(NV_AVX) && defined(NV_USE_SIMD)

            _nvSpace_integrate_velocities_AVX(space, dt);

        #else

            for (i = 0; i < n; i++) {
                _nvSpace_integrate_velocities(space, dt, i);
            }

        #endif
        space->profiler.integrate_velocities = nvPrecisionTimer_stop(&timer);

        /*
            6. Rest bodies
            ---------------
            Detect bodies with mimimal energy and rest (sleep) them.
        */
        if (space->sleeping) {
            for (i = 0; i < n; i++) {
                nvBody *body = (nvBody *)space->bodies->data[i];

                nv_float linear = nvVector2_len2(body->linear_velocity) * dt;
                nv_float angular = body->angular_velocity * dt;
                nv_float total_energy = linear + angular;

                if (total_energy <= space->sleep_energy_threshold / substeps) {
                    body->sleep_timer++;

                    if (body->sleep_timer > space->sleep_timer_threshold * substeps) {
                        nvBody_sleep(body);
                        body->sleep_timer = 0;
                    }
                }
                else {
                    if (body->sleep_timer > 0) body->sleep_timer--;
                }
            }
        }
    }

    // Actually remove all "removed" bodies.
    // TODO: This can be optimized I believe

    nvPrecisionTimer_start(&timer);

    for (i = 0; i < space->_removed_bodies->size; i++) {
        nvBody *body = (nvBody *)space->_removed_bodies->data[i];

        l = 0;
        while (nvHashMap_iter(space->res, &l, &map_val)) {
            nvResolution *res = map_val;
            if (res->a == body) {
                nvHashMap_remove(space->res, res);
                l = 0;
            }
            else if (res->b == body) {
                nvHashMap_remove(space->res, res);
                l = 0;
            }
        }

        nvArray_remove(space->bodies, body);
    }

    for (i = 0; i < space->_killed_bodies->size; i++) {
        nvBody *body = (nvBody *)space->_killed_bodies->data[i];

        l = 0;
        while (nvHashMap_iter(space->res, &l, &map_val)) {
            nvResolution *res = map_val;
            if (res->a == body) {
                nvHashMap_remove(space->res, res);
                l = 0;
            }
            else if (res->b == body) {
                nvHashMap_remove(space->res, res);
                l = 0;
            }
        }

        nvArray_remove(space->bodies, body);
        nvBody_free(body);
    }

    // TODO: nvArray_clear is needed...
    while (space->_removed_bodies->size > 0) nvArray_pop(space->_removed_bodies, 0);
    while (space->_killed_bodies->size > 0) nvArray_pop(space->_killed_bodies, 0);

    space->profiler.remove_bodies = nvPrecisionTimer_stop(&timer);
    space->profiler.step = nvPrecisionTimer_stop(&step_timer);

    NV_TRACY_ZONE_END;
    NV_TRACY_FRAMEMARK;
}

void nvSpace_enable_sleeping(nvSpace *space) {
    space->sleeping = true;
}

void nvSpace_disable_sleeping(nvSpace *space) {
    space->sleeping = false;
    for (size_t i = 0; i < space->bodies->size; i++)
        nvBody_awake((nvBody *)space->bodies->data[i]);
}