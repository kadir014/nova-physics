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


nvSpace *nvSpace_new() {
    nvSpace *space = NV_NEW(nvSpace);
    if (!space) return NULL;

    space->bodies = nvArray_new();
    space->awake_bodies = nvArray_new();
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
    space->collision_persistence = NV_COLLISION_PERSISTENCE;
    space->position_correction = nvPositionCorrection_BAUMGARTE;

    space->shg = NULL;
    nvSpace_set_broadphase(space, nvBroadPhaseAlg_SHG);

    space->broadphase_pairs = nvHashMap_new(sizeof(nvBroadPhasePair), 0, _nvSpace_broadphase_pair_hash);

    space->kill_bounds = (nvAABB){-1e4, -1e4, 1e4, 1e4};
    space->use_kill_bounds = true;

    space->mix_restitution = nvCoefficientMix_SQRT;
    space->mix_friction = nvCoefficientMix_SQRT;

    space->callback_user_data = NULL;
    space->before_collision = NULL;
    space->after_collision = NULL;

    nvProfiler_reset(&space->profiler);

    space->multithreading = false;
    space->task_executor = NULL;
    space->mt_shg_bins = NULL;
    space->mt_shg_pairs = NULL;
    space->thread_count = 0;

    space->_id_counter = 0;

    return space;
}

void nvSpace_free(nvSpace *space) {
    nvSpace_clear(space);
    nvArray_free_each(space->bodies, nvBody_free);
    nvArray_free(space->bodies);
    nvArray_free(space->awake_bodies);
    nvArray_free(space->attractors);
    nvArray_free_each(space->constraints, nvConstraint_free);
    nvArray_free(space->constraints);
    nvHashMap_free(space->res);

    free(space);
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

            nvSpace_set_SHG(space, bounds, cell_size, cell_size);

            return;

        case nvBroadPhaseAlg_BVH:
            space->broadphase_algorithm = nvBroadPhaseAlg_BVH;
            return;
    }
}

void nvSpace_set_SHG(
    nvSpace *space,
    nvAABB bounds,
    nv_float cell_width,
    nv_float cell_height
) {
    if (space->broadphase_algorithm == nvBroadPhaseAlg_SHG) {
        nvSHG_free(space->shg);
        space->shg = nvSHG_new(bounds, cell_width, cell_height);
    }
}

void nvSpace_clear(nvSpace *space) {
    nvArray_clear(space->bodies, nvBody_free);
    nvArray_clear(space->awake_bodies, NULL);
    nvArray_clear(space->attractors, NULL);
    nvArray_clear(space->constraints, nvConstraint_free);
    nvHashMap_clear(space->res);
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
    size_t velocity_iters,
    size_t position_iters,
    size_t constraint_iters,
    size_t substeps
) {
    /*
        Simulation route
        ----------------
        1. Integrate accelerations
        2. Broad-phase
        3. Update resolutions
        4. Narrow-phase
        5. Solve contact velocity constraints (PGS)
        6. Solve joint constraints (Baumgarte)
        7. Integrate velocities
        8. Position correction (NGS)
        9. Rest bodies


        Nova Physics uses semi-implicit Euler integration:

        Linear:
        v = a * Δt
        x = v * Δt

        Angular:
        ω = α * Δt
        θ = ω * Δt
    */

    if (dt == 0.0 || substeps <= 0) return;

    NV_TRACY_ZONE_START;

    nvPrecisionTimer step_timer;
    NV_PROFILER_START(step_timer);

    nvPrecisionTimer timer;

    size_t i, j, k, l;
    void *map_val;

    dt /= (nv_float)substeps;
    nv_float inv_dt = 1.0 / dt;

    for (k = 0; k < substeps; k++) {

        // TODO: Instead of clearing and filling this array every frame, update it when individual bodies are slept & awaken
        nvArray_clear(space->awake_bodies, NULL);
        for (i = 0; i < space->bodies->size; i++) {
            nvBody *body = space->bodies->data[i];

            if (!body->is_sleeping) {
                nvArray_add(space->awake_bodies, body);
            }
        }

        /*
            Integrate accelerations
            -----------------------
            Apply forces, gravity, integrate accelerations (update velocities) and apply damping.
        */
        NV_PROFILER_START(timer);
        #if defined(NV_AVX) && defined(NV_USE_SIMD)

            _nvSpace_integrate_accelerations_AVX(
                space,
                dt
            );

        #else

            for (i = 0; i < space->awake_bodies->size; i++) {
                _nvSpace_integrate_accelerations(space, dt, i);
            }

        #endif
        NV_PROFILER_STOP(timer, space->profiler.integrate_accelerations);

        /*
            Broad-phase
            -----------
            Generate possible collision pairs with the choosen broad-phase algorithm.
        */
        NV_PROFILER_START(timer);
        switch (space->broadphase_algorithm) {
            case nvBroadPhaseAlg_BRUTE_FORCE:
                nvBroadPhase_brute_force(space);
                break;

            case nvBroadPhaseAlg_SHG:
                if (space->multithreading)
                    nvBroadPhase_SHG_parallel(space);

                else
                    nvBroadPhase_SHG(space);
                    
                break;

            case nvBroadPhaseAlg_BVH:
                nvBroadPhase_BVH(space);
                break;
        }

        // Combine separate pairs from parallel broadphase into one
        if (space->multithreading) {
            nvHashMap_clear(space->broadphase_pairs);

            for (i = 0; i < space->thread_count; i++) {
                l = 0;
                while (nvHashMap_iter((nvHashMap *)space->mt_shg_pairs->data[i], &l, &map_val)) {
                    nvBroadPhasePair *pair = map_val;
                    nvHashMap_set(space->broadphase_pairs, pair);
                }
            }
        }
        NV_PROFILER_STOP(timer, space->profiler.broadphase);

        /*
            Update resolutions
            ------------------
            Update the collision resolutions from last frame for collision persistence.
        */
        l = 0;
        NV_PROFILER_START(timer);
        while (nvHashMap_iter(space->res, &l, &map_val)) {
            nvResolution *res = (nvResolution *)map_val;
            nvBody *a = res->a;
            nvBody *b = res->b;

            nvBody *pair_a, *pair_b;
            if (a->id < b->id) {
                pair_a = a;
                pair_b = b;
            }
            else {
                pair_a = b;
                pair_b = a;
            }
            nv_uint32 id_pair = nv_pair(pair_a->id, pair_b->id);

            if (!nvHashMap_get(space->broadphase_pairs, &(nvBroadPhasePair){.a=pair_a, .b=pair_b, .id_pair=id_pair})) {
                nvResolution_update(space, res);
                continue;
            }

            nvAABB abox = nvBody_get_aabb(a);
            nvAABB bbox = nvBody_get_aabb(b);

            // Even though the AABBs could be colliding, if the resolution is cached update it
            if (res->state == nvResolutionState_CACHED) {
                if (res->lifetime <= 0) {
                    nvHashMap_remove(space->res, &(nvResolution){.a=a, .b=b});
                }

                else {
                    res->lifetime--;
                }
            }

            else if (!nv_collide_aabb_x_aabb(abox, bbox)) {
                nvResolution_update(space, res);
            }
        }
        NV_PROFILER_STOP(timer, space->profiler.update_resolutions);

        /*
            Narrow-phase
            ------------
            Do narrow-phase checks between possible collision pairs and
            update collision resolutions.
        */
        NV_PROFILER_START(timer);
        nv_narrow_phase(space);
        NV_PROFILER_STOP(timer, space->profiler.narrowphase);

        /*
            PGS / Projected Gauss-Seidel
            ----------------------------
            Prepare contact velocity constraints, warm-start and solve iteratively
        */

        // Call callback before resolving collisions
        if (space->before_collision != NULL)
            space->before_collision(space, space->callback_user_data);

        // Prepare for solving contact constraints
        l = 0;
        NV_PROFILER_START(timer);
        while (nvHashMap_iter(space->res, &l, &map_val)) {
            nvResolution *res = map_val;
            if (res->state == nvResolutionState_CACHED) continue;
            nv_presolve_contact(space, res, inv_dt);
        }

        // Apply accumulated impulses
        l = 0;
        while (nvHashMap_iter(space->res, &l, &map_val)) {
            nvResolution *res = map_val;
            if (res->state == nvResolutionState_CACHED) continue;
            nv_warmstart(space, res);
        }
        NV_PROFILER_STOP(timer, space->profiler.presolve_collisions);

        // Solve velocity constraints iteratively
        NV_PROFILER_START(timer);
        for (i = 0; i < velocity_iters; i++) {
            l = 0;
            while (nvHashMap_iter(space->res, &l, &map_val)) {
                nvResolution *res = map_val;
                if (res->state == nvResolutionState_CACHED) continue;
                nv_solve_velocity(res);
            }
        }
        NV_PROFILER_STOP(timer, space->profiler.solve_velocities);

        // Call callback after resolving collisions
        if (space->after_collision != NULL)
            space->after_collision(space, space->callback_user_data);

        /*
            Solve joint constraints (Baumgarte)
            -----------------------------------
            Solve joint constraints iteratively and apply sequential impulses.
        */

        // Prepare joint constraints for solving
        NV_PROFILER_START(timer);
        for (i = 0; i < space->constraints->size; i++) {
            nvConstraint_presolve(
                space,
                (nvConstraint *)space->constraints->data[i],
                inv_dt
            );
        }
        NV_PROFILER_STOP(timer, space->profiler.presolve_constraints);

        // Solve joint constraints iteratively
        NV_PROFILER_START(timer);
        for (i = 0; i < constraint_iters; i++) {
            for (j = 0; j < space->constraints->size; j++) {
                nvConstraint_solve(
                    (nvConstraint *)space->constraints->data[j],
                    inv_dt
                );
            }
        }
        NV_PROFILER_STOP(timer, space->profiler.solve_constraints);

        /*
            Integrate velocities
            --------------------
            Integrate velocities (update positions) and check out-of-bound bodies.
        */
        NV_PROFILER_START(timer);
        #if defined(NV_AVX) && defined(NV_USE_SIMD)

            _nvSpace_integrate_velocities_AVX(space, dt);

        #else

            for (i = 0; i < space->awake_bodies->size; i++) {
                _nvSpace_integrate_velocities(space, dt, i);
            }

        #endif
        NV_PROFILER_STOP(timer, space->profiler.integrate_velocities);

        /*
            NGS / Non-Linear Gauss-Seidel
            -----------------------------
            Solve position error with pseudo-velocities.
        */
        NV_PROFILER_START(timer);
        if (space->position_correction == nvPositionCorrection_NGS) {
            for (i = 0; i < position_iters; i++) {
                l = 0;
                while (nvHashMap_iter(space->res, &l, &map_val)) {
                    nvResolution *res = map_val;
                    if (res->state == nvResolutionState_CACHED) continue;
                    nv_solve_position(res);
                }
            }
        }
        NV_PROFILER_STOP(timer, space->profiler.solve_positions);

        /*
            Rest bodies
            -----------
            Detect bodies with mimimal energy and rest (sleep) them.
        */
        if (space->sleeping) {
            for (i = 0; i < space->bodies->size; i++) {
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

    // Actually remove all killed & removed bodies from the arrays

    NV_PROFILER_START(timer);

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

    nvArray_clear(space->_removed_bodies, NULL);
    nvArray_clear(space->_killed_bodies, NULL);

    NV_PROFILER_STOP(timer, space->profiler.remove_bodies);
    NV_PROFILER_STOP(step_timer, space->profiler.step);

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

void nvSpace_enable_multithreading(nvSpace *space, size_t threads) {
    if (space->multithreading) return;

    size_t thread_count = (threads == 0) ? nv_get_cpu_count() : threads;
    space->thread_count = thread_count;

    space->task_executor = nvTaskExecutor_new(thread_count);

    space->mt_shg_bins = nvArray_new();
    space->mt_shg_pairs = nvArray_new();
    for (size_t i = 0; i < thread_count; i++) {
        nvArray_add(
            space->mt_shg_pairs,
            nvHashMap_new(sizeof(nvBroadPhasePair), 0, _nvSpace_broadphase_pair_hash)
        );

        nvArray_add(space->mt_shg_bins, nvArray_new());
    }

    space->multithreading = true;

    // Busy wait until all the task executor threads are initialized
    for (size_t i = 0; i < space->task_executor->threads->size; i++) {
        nvTaskExecutorData *data = space->task_executor->data->data[i];

        while (!data->is_active) {}
    }
}

void nvSpace_disable_multithreading(nvSpace *space) {
    if (!space->multithreading) return;

    nvTaskExecutor_close(space->task_executor);
    nvTaskExecutor_free(space->task_executor);
    space->task_executor = NULL;

    for (size_t i = 0; i < space->thread_count; i++) {
        nvHashMap_clear(space->mt_shg_pairs->data[i]);
        nvArray_clear(space->mt_shg_bins->data[i], NULL);
    }
    nvArray_free(space->mt_shg_bins);
    nvArray_free(space->mt_shg_pairs);
    space->mt_shg_bins = NULL;
    space->mt_shg_pairs = NULL;

    space->thread_count = 0;

    space->multithreading = false;
}