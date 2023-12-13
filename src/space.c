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

    nvSpace_set_broadphase(space, nvBroadPhaseAlg_BOUNDING_VOLUME_HIERARCHY);

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
    space->task_executor = NULL;
    space->res_mutex = NULL;
    //nvSpace_enable_multithreading(space, 0);

    space->_id_counter = 0;

    space->pairs = nvHashMap_new(sizeof(nvBroadPhasePair), 0, _nvSpace_broadphase_pair_hash);
    space->pairs0 = nvHashMap_new(sizeof(nvBroadPhasePair), 0, _nvSpace_broadphase_pair_hash);
    space->pairs1 = nvHashMap_new(sizeof(nvBroadPhasePair), 0, _nvSpace_broadphase_pair_hash);
    space->pairs2 = nvHashMap_new(sizeof(nvBroadPhasePair), 0, _nvSpace_broadphase_pair_hash);
    space->pairs3 = nvHashMap_new(sizeof(nvBroadPhasePair), 0, _nvSpace_broadphase_pair_hash);
    space->broadphase_pairs = nvHashMap_new(sizeof(nvBroadPhasePair), 0, _nvSpace_broadphase_pair_hash);
    space->broadphase_pairs0 = nvHashMap_new(sizeof(nvBroadPhasePair), 0, _nvSpace_broadphase_pair_hash);
    space->broadphase_pairs1 = nvHashMap_new(sizeof(nvBroadPhasePair), 0, _nvSpace_broadphase_pair_hash);
    space->broadphase_pairs2 = nvHashMap_new(sizeof(nvBroadPhasePair), 0, _nvSpace_broadphase_pair_hash);
    space->broadphase_pairs3 = nvHashMap_new(sizeof(nvBroadPhasePair), 0, _nvSpace_broadphase_pair_hash);
    space->split0 = nvArray_new();
    space->split1 = nvArray_new();
    space->split2 = nvArray_new();
    space->split3 = nvArray_new();

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
            return;

        case nvBroadPhaseAlg_BOUNDING_VOLUME_HIERARCHY:
            space->broadphase_algorithm = nvBroadPhaseAlg_BOUNDING_VOLUME_HIERARCHY;
            return;
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
    nvArray_clear(space->bodies, nvBody_free);
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
    int velocity_iters,
    int position_iters,
    int constraint_iters,
    int substeps
) {
    /*
        Simulation route
        ----------------
        1. Integrate accelerations
        2. Broad-phase
        3. Narrow-phase
        4. Solve collision constraints
        5. Solve constraints
        6. Integrate velocities
        7. Rest bodies


        Nova Physics uses semi-implicit Euler integration:

        Linear:
        v = a * Δt
        x = v * Δt

        Angular:
        ω = α * Δt
        θ = ω * Δt
    */

    NV_TRACY_ZONE_START;

    nvPrecisionTimer step_timer;
    nvPrecisionTimer_start(&step_timer);

    nvPrecisionTimer timer;

    size_t n = space->bodies->size;

    size_t i, j, k, l;
    void *map_val;

    dt /= (nv_float)substeps;
    nv_float inv_dt = 1.0 / dt;

    for (k = 0; k < substeps; k++) {
        //printf("\n\nNEW STEP\n\n");

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
            2. Broad-phase
            --------------
            Generate possible collision pairs with the choosen broad-phase algorithm.
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

            case nvBroadPhaseAlg_BOUNDING_VOLUME_HIERARCHY:
                nvBroadPhase_BVH(space);
                break;
        }
        space->profiler.broadphase = nvPrecisionTimer_stop(&timer);

        // Update resolutions from last frame
        l = 0;
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

            if (space->multithreading) {
                if (!nvHashMap_get(space->broadphase_pairs0, &(nvBroadPhasePair){.a=pair_a, .b=pair_b, .id_pair=id_pair})) {
                    nvResolution_update(space, res);
                    continue;
                }

                if (!nvHashMap_get(space->broadphase_pairs1, &(nvBroadPhasePair){.a=pair_a, .b=pair_b, .id_pair=id_pair})) {
                    nvResolution_update(space, res);
                    continue;
                }

                if (!nvHashMap_get(space->broadphase_pairs2, &(nvBroadPhasePair){.a=pair_a, .b=pair_b, .id_pair=id_pair})) {
                    nvResolution_update(space, res);
                    continue;
                }

                if (!nvHashMap_get(space->broadphase_pairs3, &(nvBroadPhasePair){.a=pair_a, .b=pair_b, .id_pair=id_pair})) {
                    nvResolution_update(space, res);
                    continue;
                }
            }
            else {
                if (!nvHashMap_get(space->broadphase_pairs, &(nvBroadPhasePair){.a=pair_a, .b=pair_b, .id_pair=id_pair})) {
                    nvResolution_update(space, res);
                    continue;
                }
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

        /*
            2. Narrow-phase
            ---------------
            Do narrow-phase checks between possible collision pairs and
            update collision resolutions.
        */
        nvPrecisionTimer_start(&timer);
        nv_narrow_phase(space);
        space->profiler.narrowphase = nvPrecisionTimer_stop(&timer);

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

    // Actually remove all killed & removed bodies from the arrays

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

    nvArray_clear(space->_removed_bodies, NULL);
    nvArray_clear(space->_killed_bodies, NULL);

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

void nvSpace_enable_multithreading(nvSpace *space, size_t threads) {
    if (space->multithreading) return;

    size_t thread_count = (threads == 0) ? nv_get_cpu_count() : threads;

    space->task_executor = nvTaskExecutor_new(thread_count);
    space->res_mutex = nvMutex_new();

    space->multithreading = true;
}

void nvSpace_disable_multithreading(nvSpace *space) {
    if (!space->multithreading) return;

    nvTaskExecutor_close(space->task_executor);
    nvTaskExecutor_free(space->task_executor);
    nvMutex_free(space->res_mutex);
    space->task_executor = NULL;
    space->res_mutex = NULL;

    space->multithreading = false;
}