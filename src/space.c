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


/**
 * @file space.c
 * 
 * @details Space is the container of everything. Handles simulation.
 * 
 *          Function documentations are in novaphysics/space.h
 */


nv_Space *nv_Space_new() {
    nv_Space *space = (nv_Space *)malloc(sizeof(nv_Space));

    space->bodies = nv_Array_new();
    space->attractors = nv_Array_new();
    space->constraints = nv_Array_new();

    space->res = nv_HashMap_new();

    space->gravity = NV_VEC2(0.0, NV_GRAV_EARTH);

    space->sleeping = false;
    space->sleep_energy_threshold = 0.75;
    space->wake_energy_threshold = space->sleep_energy_threshold / 1.3;
    space->sleep_timer_threshold = 60;

    space->warmstarting = true;
    space->baumgarte = 0.15;

    space->broadphase_algorithm = nv_BroadPhase_SPATIAL_HASH_GRID;
    space->shg = nv_SHG_new(
        (nv_AABB){
            0, 0,
            128, 72
        },
        128/2, 72/2
    );

    space->mix_restitution = nv_CoefficientMix_MIN;
    space->mix_friction = nv_CoefficientMix_SQRT;

    space->callback_user_data = NULL;
    space->before_collision = NULL;
    space->after_collision = NULL;

    return space;
}

void nv_Space_free(nv_Space *space) {
    nv_Space_clear(space);
    nv_Array_free(space->bodies);
    nv_Array_free(space->attractors);
    nv_Array_free(space->constraints);
    nv_HashMap_free(space->res, NULL);

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

void nv_Space_clear(nv_Space *space) {
    size_t i;

    while (space->bodies->size > 0) {
        nv_Body_free(nv_Array_pop(space->bodies, 0));
    }

    // Don't free individual attractors because they are freed before
    for (i = 0; i < space->attractors->size; i++) {
        nv_Array_pop(space->attractors, 0);
    }

    for (i = 0; i < space->constraints->size; i++) {
        nv_Constraint_free(nv_Array_pop(space->constraints, 0));
    }

    nv_HashMap_clear(space->res, NULL);

    /*
        We can set array->max to 0 and reallocate but
        not doing it might be more efficient for the developer
        since they will probably fill the array up again.
    */
}

void nv_Space_add(nv_Space *space, nv_Body *body) {
    nv_Array_add(space->bodies, body);
    body->space = space;
    body->id = space->bodies->size;
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
        3. Solve collisions
        4. Solve constraints
        5. Sleep bodies
        6. Integrate velocities


        Nova Physics uses semi-implicit Euler integration:

        Linear:
        v = a * Δt
        x = v * Δt

        Angular:
        ω = α * Δt
        θ = ω * Δt
    */

    size_t n = space->bodies->size;

    size_t i, j, k;

    dt /= (nv_float)substeps;
    nv_float inv_dt = 1.0 / dt;

    for (k = 0; k < substeps; k++) {

        /*
            1. Integrate accelerations
            --------------------------
            Apply forces and gravity, then integrate accelerations (update velocities)
        */
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

        /*
            2. Broad-phase & Narrow-phase
            -----------------------------
            Generate possible collision pairs with the choosen broad-phase
            algorithm and create collision resolutions with the more
            expensive narrow-phase calculations
        */

        switch (space->broadphase_algorithm) {
            /*
                Brute-force algorithm checks every AABB with the other, thus
                becomes way slower with more bodies

                O(n^2)
            */
            case nv_BroadPhase_BRUTE_FORCE:
                nv_BroadPhase_brute_force(space);
                break;

            /*
                Spatial hash grid works by dividing space into a grid and
                adds bodies to grid's 1D hashed version. Ten every body's
                neighbor cells are checked in collision detection

                O(nlog(n))
            */
            case nv_BroadPhase_SPATIAL_HASH_GRID:
                nv_BroadPhase_spatial_hash_grid(space);
                break;
        }

        /*
            3. Solve collisions
            -------------------
            Solve collisions and apply sequential impulses
        */

        // Call callback before resolving collisions
        if (space->before_collision != NULL)
            space->before_collision(space->res, space->callback_user_data);

        // Prepare collision resolutions
        nv_HashMapIterator iterator = nv_HashMapIterator_new(space->res);
        while (nv_HashMapIterator_next(&iterator)) {
            nv_prestep_collision(
                space,
                (nv_Resolution *)iterator.value,
                inv_dt
            );
        }

        // Solve positions (pseudo-velocities) iteratively
        for (i = 0; i < position_iters; i++) {
            iterator = nv_HashMapIterator_new(space->res);
            while (nv_HashMapIterator_next(&iterator)) {
                nv_solve_position((nv_Resolution *)iterator.value);
            }
        }

        // Solve velocities iteratively
        for (i = 0; i < velocity_iters; i++) {
            iterator = nv_HashMapIterator_new(space->res);
            while (nv_HashMapIterator_next(&iterator)) {
                nv_solve_velocity((nv_Resolution *)iterator.value);
            }
        }

        // Call callback after resolving collisions
        if (space->after_collision != NULL)
            space->after_collision(space->res, space->callback_user_data);

        /*
            4. Solve constraints
            --------------------
            Solve constraints and apply sequential impulses
        */

        // Prepare constraints
        for (i = 0; i < space->constraints->size; i++) {
            nv_prestep_constraint(
                (nv_Constraint *)space->constraints->data[i],
                inv_dt,
                space->baumgarte
            );
        }

        // Solve constraints iteratively
        for (i = 0; i < constraint_iters; i++) {
            for (j = 0; j < space->constraints->size; j++) {
                nv_solve_constraint((nv_Constraint *)space->constraints->data[j]);
            }
        }

        /*
            5. Sleep bodies
            ---------------
            Detect bodies with mimimal energy and rest them
        */
        if (space->sleeping) {
            for (i = 0; i < n; i++) {
                nv_Body *body = (nv_Body *)space->bodies->data[i];

                nv_float total_energy =
                nv_Vector2_len(body->linear_velocity) + body->angular_velocity;

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

        /*
            6. Integrate velocities
            -----------------------
            Apply damping and integrate velocities (update positions)
        */
        for (i = 0; i < n; i++) {
            nv_Body *body = (nv_Body *)space->bodies->data[i];
            if (space->sleeping && body->is_sleeping) continue;
            nv_Body_integrate_velocities(body, dt);
        }
    }
}

void nv_Space_enable_sleeping(nv_Space *space) {
    space->sleeping = true;
}

void nv_Space_disable_sleeping(nv_Space *space) {
    space->sleeping = false;
    for (size_t i = 0; i < space->bodies->size; i++)
        nv_Body_awake((nv_Body *)space->bodies->data[i]);
}