/*

  This file is a part of the Nova Physics Engine
  project and distributed under the MIT license.

  Copyright © Kadir Aksoy
  https://github.com/kadir014/nova-physics

*/

#include <stdlib.h>
#include "novaphysics/space.h"
#include "novaphysics/constants.h"
#include "novaphysics/body.h"
#include "novaphysics/collision.h"
#include "novaphysics/contact.h"
#include "novaphysics/solver.h"
#include "novaphysics/math.h"


/**
 * space.c
 * 
 * Space
 */


nv_Space *nv_Space_new() {
    nv_Space *space = (nv_Space *)malloc(sizeof(nv_Space));

    space->bodies = nv_BodyArray_new();
    space->attractors = nv_BodyArray_new();

    space->gravity = (nv_Vector2){0.0, NV_GRAV_EARTH};

    space->sleeping = false;

    space->callback_user_data = NULL;
    space->before_collision = NULL;
    space->after_collision = NULL;

    return space;
}

void nv_Space_free(nv_Space *space) {
    nv_BodyArray_free(space->bodies);
    space->bodies = NULL;
    // attractor bodies should be freed by now so don't use nv_BodyArray_free
    free(space->attractors->data);
    free(space->attractors);
    free(space);
}

void nv_Space_add(nv_Space *space, nv_Body *body) {
    nv_BodyArray_add(space->bodies, body);
    body->space = space;
}

void nv_Space_step(
    nv_Space *space,
    double dt,
    int iterations,
    int substeps
) {
    /*
        Simulation route
        ----------------
        1. Integrate accelerations
        2. Broad-phase
        3. Narrow-phase
        4. Apply impulses
        5. Integrate velocities


        Semi-implicit Euler integration
        -------------------------------
        Linear:
        v = a * Δt
        x = v * Δt

        Angular:
        ω = α * Δt
        θ = ω * Δt
    */

    size_t n = space->bodies->size;
    size_t i, j, k;

    dt /= substeps;

    for (k = 0; k < substeps; k++) {

        /*
            1. Integrate accelerations
            --------------------------
            Apply forces and gravity, then integrate accelerations (update velocities)
        */
        for (i = 0; i < n; i++) {
            nv_Body *body = space->bodies->data[i];
            if (space->sleeping && body->is_sleeping) continue;

            // Apply attractive forces
            for (j = 0; j < space->attractors->size; j++) {
                nv_Body *attractor = space->attractors->data[j];
                
                // Same body
                if (body == attractor) continue;

                nv_Body_apply_attraction(body, space->attractors->data[j]);
            }
            
            nv_Body_integrate_accelerations(body, space->gravity, dt);
        }

        /*
            2. Broad-phase
            --------------
            Generate possible collided pairs with fast-ish AABB collisions
        */
        nv_BodyPairArray *pairs = nv_BodyPairArray_new();

        nv_Space_broadphase(space, pairs);

        /*
            3. Narrow-phase
            ---------------
            Generate collision resolutions with expensive polygon collisions

            We also solve positional correction in this phase
        */
        nv_ResolutionArray *res_arr = nv_Space_narrowphase(space, pairs);

        // Call callback before resolving impulses
        if (space->before_collision != NULL)
            space->before_collision(res_arr, space->callback_user_data);

        /*
            4. Apply impulses
            -----------------
            Sequential impulses are applied within iterations
        */
        for (i = 0; i < iterations; i++) {
            for (j = 0; j < res_arr->size; j++) {
                nv_resolve_collision(res_arr->data[j]);
            }
        }

        // Call callback after resolving impulses
        if (space->after_collision != NULL)
            space->after_collision(res_arr, space->callback_user_data);

        // Sleep bodies
        if (space->sleeping) {
            for (i = 0; i < n; i++) {
                nv_Body *body = space->bodies->data[i];
                
                double total_energy = nv_Body_get_kinetic_energy(body) +
                                    nv_Body_get_rotational_energy(body);

                if (total_energy <= 2.0) {
                    body->sleep_counter++;

                    if (body->sleep_counter > 60) {
                        nv_Body_sleep(body);
                        body->sleep_counter = 0;
                    }
                }
                else {
                    if (body->sleep_counter > 0) body->sleep_counter--;
                }
            }
        }

        /*
            5. Integrate velocities
            -----------------------
            Apply damping and integrate velocities (update positions)
        */
        for (i = 0; i < n; i++) {
            nv_Body *body = space->bodies->data[i];
            nv_Body_integrate_velocities(body, dt);
        }

        // Free the space allocated in this step
        nv_BodyPairArray_free(pairs);
        nv_ResolutionArray_free(res_arr);
    }
}


nv_BodyPair *nv_Space_broadphase(nv_Space *space, nv_BodyPairArray *pairs) {
    for (size_t i = 0; i < space->bodies->size; i++) {
        nv_Body *a = space->bodies->data[i];
        nv_AABB abox = nv_Body_get_aabb(a);

        for (size_t j = 0; j < space->bodies->size; j++) {
            nv_Body *b = space->bodies->data[j];

            // Same body
            if (a == b)
                continue;

            // Two static bodies do not need to interact
            if (a->type == nv_BodyType_STATIC && b->type == nv_BodyType_STATIC)
                continue;

            // Both bodies are asleep
            if (space->sleeping) {
                if (a->is_sleeping && b->is_sleeping)
                    continue;
            }

            nv_AABB bbox = nv_Body_get_aabb(b);

            if (nv_collide_aabb_x_aabb(abox, bbox)) {
                nv_BodyPair pair = (nv_BodyPair){a, b};
                bool skip_pair = false;

                // Don't create a pair if it already exists
                for (size_t k = 0; k < pairs->size; k++) {
                    if ((pair.a == pairs->data[k].a && pair.b == pairs->data[k].b) ||
                        (pair.a == pairs->data[k].b && pair.b == pairs->data[k].a))
                        skip_pair = true;
                        break;
                }

                if (skip_pair) continue;

                nv_BodyPairArray_add(pairs, pair);
            }
        }
    }
}

nv_ResolutionArray *nv_Space_narrowphase(
    nv_Space *space,
    nv_BodyPairArray *pairs
) {
    nv_ResolutionArray *res_arr = nv_ResolutionArray_new();

    for (size_t i = 0; i < pairs->size; i++) {
        nv_Body *a = pairs->data[i].a;
        nv_Body *b = pairs->data[i].b;

        nv_Resolution res;

        if (a->shape == nv_BodyShape_CIRCLE && b->shape == nv_BodyShape_CIRCLE)
            res = nv_collide_circle_x_circle(a, b);

        else if (a->shape == nv_BodyShape_CIRCLE && b->shape == nv_BodyShape_POLYGON)
            res = nv_collide_polygon_x_circle(b, a);

        else if (a->shape == nv_BodyShape_POLYGON && b->shape == nv_BodyShape_CIRCLE)
            res = nv_collide_polygon_x_circle(a, b);
    
        else if (a->shape == nv_BodyShape_POLYGON && b->shape == nv_BodyShape_POLYGON)
            res = nv_collide_polygon_x_polygon(a, b);

        if (res.collision) {
            if (a->shape == nv_BodyShape_CIRCLE && b->shape == nv_BodyShape_CIRCLE)
                nv_contact_circle_x_circle(&res);

            else if (a->shape == nv_BodyShape_CIRCLE && b->shape == nv_BodyShape_POLYGON)
                nv_contact_polygon_x_circle(&res);

            else if (a->shape == nv_BodyShape_POLYGON && b->shape == nv_BodyShape_CIRCLE)
                nv_contact_polygon_x_circle(&res);
        
            else if (a->shape == nv_BodyShape_POLYGON && b->shape == nv_BodyShape_POLYGON)
                 nv_contact_polygon_x_polygon(&res);

            nv_ResolutionArray_add(res_arr, res);

            nv_positional_correction(res);

            if (space->sleeping) {
                if (a->is_sleeping && !b->is_sleeping) {
                    double total_energy = nv_Body_get_kinetic_energy(b) +
                                        nv_Body_get_rotational_energy(b);

                    if (total_energy > 1.0)
                        nv_Body_awake(a);
                }

                if (b->is_sleeping && !a->is_sleeping) {
                    double total_energy = nv_Body_get_kinetic_energy(a) +
                                        nv_Body_get_rotational_energy(a);

                    if (total_energy > 1.0)
                        nv_Body_awake(b);
                }
            }
            
        }
    }

    return res_arr;
}


nv_BodyPairArray *nv_BodyPairArray_new() {
    nv_BodyPairArray *array = (nv_BodyPairArray *)malloc(sizeof(nv_BodyPairArray));

    array->size = 0;

    array->data = (nv_BodyPair *)malloc(sizeof(nv_BodyPair));

    return array;
}

void nv_BodyPairArray_free(nv_BodyPairArray *array) {
    free(array->data);
    array->data = NULL;
    array->size = 0;
    free(array);
}

void nv_BodyPairArray_add(nv_BodyPairArray *array, nv_BodyPair pair) {
    array->size += 1;

    array->data = (nv_BodyPair *)realloc(array->data, array->size * sizeof(nv_BodyPair));

    array->data[array->size - 1] = pair;
}