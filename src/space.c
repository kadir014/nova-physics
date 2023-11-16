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


/**
 * @file space.c
 * 
 * @brief Space struct and its methods.
 */


/* Helper hashing functions for space hash maps */

static nv_uint64 spacehash(void *item) {
    nv_Resolution *res = (nv_Resolution *)item;
    if (res->a == NULL || res->b == NULL) return 0;
    return (nv_uint64)nv_hash(nv_pair(res->a->id, res->b->id));
}

static nv_uint64 pairhash(void *item) {
    nv_BroadPhasePair *pair = item;
    return (nv_uint64)nv_hash(pair->id_pair);
}


nv_Space *nv_Space_new() {
    nv_Space *space = NV_NEW(nv_Space);
    if (!space) return NULL;

    space->bodies = nv_Array_new();
    space->attractors = nv_Array_new();
    space->constraints = nv_Array_new();

    space->_removed_bodies = nv_Array_new();
    space->_killed_bodies = nv_Array_new();

    space->res = nv_HashMap_new(sizeof(nv_Resolution), 0, spacehash);

    space->gravity = NV_VEC2(0.0, NV_GRAV_EARTH);

    space->sleeping = false;
    space->sleep_energy_threshold = 0.02;
    space->wake_energy_threshold = space->sleep_energy_threshold / 1.3;
    space->sleep_timer_threshold = 60;

    space->warmstarting = true;
    space->baumgarte = NV_BAUMGARTE;
    space->collision_persistence = NV_COLLISION_PERSISTENCE;

    space->pairs = nv_HashMap_new(sizeof(nv_BroadPhasePair), 0, pairhash);
    nv_Space_set_broadphase(space, nv_BroadPhase_SPATIAL_HASH_GRID);

    space->kill_bounds = (nv_AABB){-1e4, -1e4, 1e4, 1e4};
    space->use_kill_bounds = true;

    space->mix_restitution = nv_CoefficientMix_SQRT;
    space->mix_friction = nv_CoefficientMix_SQRT;

    space->callback_user_data = NULL;
    space->before_collision = NULL;
    space->after_collision = NULL;

    nv_Profiler_reset(&space->profiler);
    #ifdef NV_WINDOWS
    nv_set_windows_timer_resolution();
    #endif

    space->_id_counter = 0;

    return space;
}

void nv_Space_free(nv_Space *space) {
    nv_Space_clear(space);
    nv_Array_free(space->bodies);
    nv_Array_free(space->attractors);
    nv_Array_free(space->constraints);
    nv_HashMap_free(space->res);
    nv_HashMap_free(space->pairs);

    space->gravity = nv_Vector2_zero;
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

void nv_Space_set_broadphase(nv_Space *space, nv_BroadPhase broadphase_type) {
    if (space->broadphase_algorithm == nv_BroadPhase_SPATIAL_HASH_GRID)
        nv_SHG_free(space->shg);
    
    switch (broadphase_type) {
        case nv_BroadPhase_BRUTE_FORCE:
            space->broadphase_algorithm = nv_BroadPhase_BRUTE_FORCE;
            return;

        case nv_BroadPhase_SPATIAL_HASH_GRID:
            space->broadphase_algorithm = nv_BroadPhase_SPATIAL_HASH_GRID;
            space->shg = nv_SHG_new((nv_AABB){0, 0, 128.0, 72.0}, 3.5, 3.5);
    }
}

void nv_Space_set_SHG(
    nv_Space *space,
    nv_AABB bounds,
    nv_float cell_width,
    nv_float cell_height
) {
    nv_SHG_free(space->shg);

    space->shg = nv_SHG_new(bounds, cell_width, cell_height);
}

void nv_Space_clear(nv_Space *space) {
    while (space->bodies->size > 0) {
        nv_Body_free(nv_Array_pop(space->bodies, 0));
    }

    while (space->attractors->size > 0) {
        // Don't free individual attractors because they are freed before
        nv_Array_pop(space->attractors, 0);
    }

    while (space->constraints->size > 0) {
        nv_Constraint_free(nv_Array_pop(space->constraints, 0));
    }

    nv_HashMap_clear(space->res);

    /*
        We can set array->max to 0 and reallocate but
        not doing it might be more efficient for the developer
        since they will probably fill the array up again.
        Maybe a separate parameter for this?
    */
}

void nv_Space_add(nv_Space *space, nv_Body *body) {
    NV_ASSERT(body->space != space, "You can't add the same body to the same space multiple times.");

    nv_Array_add(space->bodies, body);
    body->space = space;
    body->id = space->_id_counter;
    space->_id_counter++;
}

void nv_Space_remove(nv_Space *space, nv_Body *body) {
    nv_Array_add(space->_removed_bodies, body);
}

void nv_Space_kill(nv_Space *space, nv_Body *body) {
    nv_Array_add(space->_killed_bodies, body);
}

void nv_Space_add_constraint(nv_Space *space, nv_Constraint *cons) {
    nv_Array_add(space->constraints, cons);
}

void nv_Space_step(
    nv_Space *space,
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

    size_t n = space->bodies->size;

    size_t i, j, k, l;
    void *map_val;

    dt /= (nv_float)substeps;
    nv_float inv_dt = 1.0 / dt;

    nv_PrecisionTimer step_timer;
    nv_PrecisionTimer timer;

    nv_PrecisionTimer_start(&step_timer);

    for (k = 0; k < substeps; k++) {

        /*
            1. Integrate accelerations
            --------------------------
            Apply forces and gravity, then integrate accelerations (update velocities).
        */
        nv_PrecisionTimer_start(&timer);
        for (i = 0; i < n; i++) {
            nv_Body *body = (nv_Body *)space->bodies->data[i];
            if (space->sleeping && body->is_sleeping) continue;

            // Apply attractive forces
            for (j = 0; j < space->attractors->size; j++) {
                nv_Body *attractor = (nv_Body *)space->attractors->data[j];
                
                // Same body
                if (body == attractor) continue;

                nv_Body_apply_attraction(body, attractor);
            }
            
            nv_Body_integrate_accelerations(body, space->gravity, dt);
        }
        space->profiler.integrate_accelerations = nv_PrecisionTimer_stop(&timer);

        /*
            2. Broad-phase & Narrow-phase
            -----------------------------
            Generate possible collision pairs with the choosen broad-phase
            algorithm and create collision resolutions with the more
            expensive narrow-phase calculations.
        */
        nv_PrecisionTimer_start(&timer);
        switch (space->broadphase_algorithm) {
            case nv_BroadPhase_BRUTE_FORCE:
                nv_BroadPhase_brute_force(space);
                break;

            case nv_BroadPhase_SPATIAL_HASH_GRID:
                nv_BroadPhase_spatial_hash_grid(space);
                break;
        }
        space->profiler.broadphase = nv_PrecisionTimer_stop(&timer);

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
        nv_PrecisionTimer_start(&timer);
        while (nv_HashMap_iter(space->res, &l, &map_val)) {
            nv_Resolution *res = map_val;
            if (res->state == nv_ResolutionState_CACHED) continue;
            nv_presolve_collision(space, res, inv_dt);
        }

        // Apply accumulated impulses
        l = 0;
        while (nv_HashMap_iter(space->res, &l, &map_val)) {
            nv_Resolution *res = map_val;
            if (res->state == nv_ResolutionState_CACHED) continue;
            nv_warmstart(space, res);
        }
        space->profiler.presolve_collisions = nv_PrecisionTimer_stop(&timer);

        // Solve positions (pseudo-velocities) constraints iteratively
        nv_PrecisionTimer_start(&timer);
        for (i = 0; i < position_iters; i++) {
            l = 0;
            while (nv_HashMap_iter(space->res, &l, &map_val)) {
                nv_Resolution *res = map_val;
                if (res->state == nv_ResolutionState_CACHED) continue;
                nv_solve_position(res);
            }
        }
        space->profiler.solve_positions = nv_PrecisionTimer_stop(&timer);

        // Solve velocity constraints iteratively
        nv_PrecisionTimer_start(&timer);
        for (i = 0; i < velocity_iters; i++) {
            l = 0;
            while (nv_HashMap_iter(space->res, &l, &map_val)) {
                nv_Resolution *res = map_val;
                if (res->state == nv_ResolutionState_CACHED) continue;
                nv_solve_velocity(res);
            }
        }
        space->profiler.solve_velocities = nv_PrecisionTimer_stop(&timer);

        // Call callback after resolving collisions
        if (space->after_collision != NULL)
            space->after_collision(space->res, space->callback_user_data);

        /*
            4. Solve constraints
            --------------------
            Solve constraints and apply sequential impulses.
        */

        // Prepare constraints for solving
        nv_PrecisionTimer_start(&timer);
        for (i = 0; i < space->constraints->size; i++) {
            nv_presolve_constraint(
                space,
                (nv_Constraint *)space->constraints->data[i],
                inv_dt
            );
        }
        space->profiler.presolve_constraints = nv_PrecisionTimer_stop(&timer);

        // Solve constraints iteratively
        nv_PrecisionTimer_start(&timer);
        for (i = 0; i < constraint_iters; i++) {
            for (j = 0; j < space->constraints->size; j++) {
                nv_solve_constraint((nv_Constraint *)space->constraints->data[j]);
            }
        }
        space->profiler.solve_constraints = nv_PrecisionTimer_stop(&timer);

        /*
            5. Integrate velocities
            -----------------------
            Apply damping and integrate velocities (update positions).
        */
        nv_PrecisionTimer_start(&timer);
        for (i = 0; i < n; i++) {
            nv_Body *body = (nv_Body *)space->bodies->data[i];
            if (space->sleeping && body->is_sleeping) continue;
            nv_Body_integrate_velocities(body, dt);

            // Since most kill boundaries in games are going to be out of the
            // display area just checking for body's center position
            // is sufficient.
            if (
                space->use_kill_bounds &&
                !nv_collide_aabb_x_point(space->kill_bounds, body->position)
            )
                nv_Space_kill(space, body);
        }
        space->profiler.integrate_velocities = nv_PrecisionTimer_stop(&timer);

        /*
            6. Rest bodies
            ---------------
            Detect bodies with mimimal energy and rest them.
        */
        if (space->sleeping) {
            for (i = 0; i < n; i++) {
                nv_Body *body = (nv_Body *)space->bodies->data[i];

                nv_float linear = nv_Vector2_len2(body->linear_velocity) * dt;
                nv_float angular = body->angular_velocity * dt;
                nv_float total_energy = linear + angular;

                if (total_energy <= space->sleep_energy_threshold / substeps) {
                    body->sleep_timer++;

                    if (body->sleep_timer > space->sleep_timer_threshold * substeps) {
                        nv_Body_sleep(body);
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

    nv_PrecisionTimer_start(&timer);

    for (i = 0; i < space->_removed_bodies->size; i++) {
        nv_Body *body = (nv_Body *)space->_removed_bodies->data[i];

        l = 0;
        while (nv_HashMap_iter(space->res, &l, &map_val)) {
            nv_Resolution *res = map_val;
            if (res->a == body) {
                nv_HashMap_remove(space->res, res);
                l = 0;
            }
            else if (res->b == body) {
                nv_HashMap_remove(space->res, res);
                l = 0;
            }
        }

        nv_Array_remove(space->bodies, body);
    }

    for (i = 0; i < space->_killed_bodies->size; i++) {
        nv_Body *body = (nv_Body *)space->_killed_bodies->data[i];

        l = 0;
        while (nv_HashMap_iter(space->res, &l, &map_val)) {
            nv_Resolution *res = map_val;
            if (res->a == body) {
                nv_HashMap_remove(space->res, res);
                l = 0;
            }
            else if (res->b == body) {
                nv_HashMap_remove(space->res, res);
                l = 0;
            }
        }

        nv_Array_remove(space->bodies, body);
        nv_Body_free(body);
    }

    // TODO: nv_Array_clear is needed...
    while (space->_removed_bodies->size > 0) nv_Array_pop(space->_removed_bodies, 0);
    while (space->_killed_bodies->size > 0) nv_Array_pop(space->_killed_bodies, 0);

    space->profiler.remove_bodies = nv_PrecisionTimer_stop(&timer);
    space->profiler.step = nv_PrecisionTimer_stop(&step_timer);
}

void nv_Space_enable_sleeping(nv_Space *space) {
    space->sleeping = true;
}

void nv_Space_disable_sleeping(nv_Space *space) {
    space->sleeping = false;
    for (size_t i = 0; i < space->bodies->size; i++)
        nv_Body_awake((nv_Body *)space->bodies->data[i]);
}