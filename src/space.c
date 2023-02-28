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
 * space.c
 * 
 * Space
 */


nv_Space *nv_Space_new() {
    nv_Space *space = (nv_Space *)malloc(sizeof(nv_Space));

    space->bodies = nv_Array_new();
    space->attractors = nv_Array_new();
    space->constraints = nv_Array_new();

    space->res = nv_Array_new();

    space->gravity = NV_VEC2(0.0, NV_GRAV_EARTH);

    space->sleeping = false;
    space->sleep_energy_threshold = 0.4;
    space->sleep_timer_threshold = 20;

    space->warmstarting = true;
    space->baumgarte = 0.1;

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
    nv_Array_free(space->res);

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
    size_t n0 = space->bodies->size;
    size_t n1 = space->attractors->size;
    size_t n2 = space->constraints->size;
    size_t n3 = space->res->size;

    for (i = 0; i < n0; i++) {
        free(nv_Array_pop(space->bodies, 0));
    }

    // Don't free individual attractors because they are freed before
    for (i = 0; i < n1; i++) {
        nv_Array_pop(space->attractors, 0);
    }

    for (i = 0; i < n2; i++) {
        free(nv_Array_pop(space->constraints, 0));
    }

    for (i = 0; i < n3; i++) {
        free(nv_Array_pop(space->res, 0));
    }

    /*
        We can set array->max to 0 and reallocate but
        not doing it might be more efficient for the developer
        since they will probably fill the array up again.
    */
}

void nv_Space_add(nv_Space *space, nv_Body *body) {
    nv_Array_add(space->bodies, body);
    body->space = space;
}

void nv_Space_add_constraint(nv_Space *space, nv_Constraint *cons) {
    nv_Array_add(space->constraints, cons);
}

void nv_Space_step(
    nv_Space *space,
    nv_float dt,
    int velocity_iters,
    int position_iters,
    int substeps
) {
    /*
        Simulation route
        ----------------
        1. Integrate accelerations
        2. Broad-phase
        3. Narrow-phase
        4. Solve collisions
        5. Solve constraints
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
            2. Broad-phase
            --------------
            Generate possible collision pairs with fast-ish AABB collisions
            still O(N^2) though
        */

        //nv_Array *pairs = nv_Space_broadphase(space);

        /*
            3. Narrow-phase
            ---------------
            Generate collision resolutions with expensive polygon collisions
        */
        //nv_Space_narrowphase(space, pairs);
        nv_Space_narrowphase2(space);

        /*
            4. Solve collisions
            -------------------
            Solve collisions and apply sequential impulses
        */

        // Call callback before resolving collisions
        if (space->before_collision != NULL)
            space->before_collision(space->res, space->callback_user_data);

        // Prepare collision resolutions
        for (i = 0; i < space->res->size; i++) {
            nv_prestep_collision(
                space,
                (nv_Resolution *)space->res->data[i],
                inv_dt
            );
        }

        // Solve positions (pseudo-velocities) iteratively
        for (i = 0; i < position_iters; i++) {
            for (j = 0; j < space->res->size; j++) {
                nv_solve_position((nv_Resolution *)space->res->data[j]);
            }
        }

        // Solve velocities iteratively
        for (i = 0; i < velocity_iters; i++) {
            for (j = 0; j < space->res->size; j++) {
                nv_solve_velocity((nv_Resolution *)space->res->data[j]);
            }
        }

        // Call callback after resolving collisions
        if (space->after_collision != NULL)
            space->after_collision(space->res, space->callback_user_data);

        /*
            5. Solve constraints
            --------------------
            Solve constraints and apply sequential impulses
        */

        for (j = 0; j < space->constraints->size; j++) {
            nv_prestep_constraint(
                (nv_Constraint *)space->constraints->data[j],
                inv_dt,
                space->baumgarte
            );
        }

        // Solve constraints
        for (i = 0; i < 1; i++) {
            for (j = 0; j < space->constraints->size; j++) {
                nv_solve_constraint((nv_Constraint *)space->constraints->data[j]);
            }
        }

        // Sleep bodies
        if (space->sleeping) {
            for (i = 0; i < n; i++) {
                nv_Body *body = (nv_Body *)space->bodies->data[i];
                
                nv_float total_energy = nv_Body_get_kinetic_energy(body) +
                                    nv_Body_get_rotational_energy(body);

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

        // Free the space allocated in this step
        //nv_Array_free_each(pairs, free);
        //nv_Array_free(pairs);
    }
}


void nv_Space_narrowphase2(nv_Space *space) {
    nv_Array *pairs = nv_Array_new();

    for (size_t i = 0; i < space->bodies->size; i++) {
        nv_Body *a = (nv_Body *)space->bodies->data[i];
        nv_AABB abox = nv_Body_get_aabb(a);

        for (size_t j = 0; j < space->bodies->size; j++) {
            nv_Body *b = (nv_Body *)space->bodies->data[j];

            // Same body
            if (a == b)
                continue;

            if (!a->collision || !b->collision)
                continue;

            // Two static bodies do not need to interact
            if (a->type == nv_BodyType_STATIC && b->type == nv_BodyType_STATIC)
                continue;

            // Both bodies are asleep
            if (space->sleeping) {
                if (a->is_sleeping && b->is_sleeping)
                    continue;

                if ((a->is_sleeping && b->type == nv_BodyType_STATIC) ||
                    (b->is_sleeping && a->type == nv_BodyType_STATIC))
                    continue;
            }

            // Check if a resolution between bodies already exists
            bool exists = false;
            size_t exist_i = 0;
            for (size_t j = 0; j < space->res->size; j++) {
                nv_Resolution *res2 = (nv_Resolution *)space->res->data[j];

                if ((a == res2->a && b == res2->b) ||
                    (b == res2->a && a == res2->b)
                ) {
                    exists = true;
                    exist_i = j;
                    break;
                }
            }
            
            // Check for existing pair
            nv_BodyPair pair = (nv_BodyPair){a, b};
            bool skip_pair = false;

            // Don't create a pair if it already exists
            for (size_t k = 0; k < pairs->size; k++) {
                if ((pair.a == ((nv_BodyPair *)pairs->data[k])->a &&
                        pair.b == ((nv_BodyPair *)pairs->data[k])->b) ||
                    (pair.a == ((nv_BodyPair *)pairs->data[k])->b &&
                        pair.b == ((nv_BodyPair *)pairs->data[k])->a)) {
                    skip_pair = true;
                    break;
                }
            }

            if (skip_pair) continue;

            nv_AABB bbox = nv_Body_get_aabb(b);

            if (nv_collide_aabb_x_aabb(abox, bbox)) {
                
                nv_BodyPair *pair_heap = NV_NEW(nv_BodyPair);
                pair_heap->a = pair.a;
                pair_heap->b = pair.b;

                nv_Array_add(pairs, pair_heap);

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

                    if (space->sleeping) {
                        if (a->is_sleeping && (!b->is_sleeping && b->type != nv_BodyType_STATIC)) {
                            nv_float total_energy = nv_Body_get_kinetic_energy(b) +
                                                nv_Body_get_rotational_energy(b);

                            if (total_energy > space->sleep_energy_threshold)
                                nv_Body_awake(a);
                        }

                        if (b->is_sleeping && (!a->is_sleeping && a->type != nv_BodyType_STATIC)) {
                            nv_float total_energy = nv_Body_get_kinetic_energy(a) +
                                                nv_Body_get_rotational_energy(a);

                            if (total_energy > space->sleep_energy_threshold)
                                nv_Body_awake(b);
                        }
                    }

                    /*
                        If the resolution between bodies already exists then
                        just update it. Else, create a new resolution.
                    */
                    if (exists) {
                        nv_Resolution *resp = (nv_Resolution *)space->res->data[exist_i];
                        resp->normal = res.normal;
                        resp->depth = res.depth;
                        resp->collision = res.collision;
                        resp->contact_count = res.contact_count;
                        resp->contacts[0] = res.contacts[0];
                        resp->contacts[1] = res.contacts[1];
                    }
                    else {
                        nv_Resolution *res_heap = NV_NEW(nv_Resolution);
                        res_heap->a = res.a;
                        res_heap->b = res.b;
                        res_heap->normal = res.normal;
                        res_heap->depth = res.depth;
                        res_heap->collision = res.collision;
                        res_heap->contact_count = res.contact_count;
                        res_heap->contacts[0] = res.contacts[0];
                        res_heap->contacts[1] = res.contacts[1];
                        res_heap->jn[0] = 0.0;
                        res_heap->jn[1] = 0.0; 
                        res_heap->jt[0] = 0.0;
                        res_heap->jt[1] = 0.0; 
                        nv_Array_add(space->res, res_heap);
                    }
                }

                else {
                    // Remove seperated collision resolution from array
                    if (exists) {
                        free(nv_Array_pop(space->res, exist_i));
                    }
                }
            }

            else {
                // Remove seperated collision resolution from array and awake bodies
                if (exists) {
                    nv_Body_awake(((nv_Resolution *)space->res->data[exist_i])->a);
                    nv_Body_awake(((nv_Resolution *)space->res->data[exist_i])->b);
                    free(nv_Array_pop(space->res, exist_i));
                }
            }
        }
    }

    nv_Array_free(pairs);
}


nv_Array *nv_Space_broadphase(nv_Space *space) {
    nv_Array *pairs = nv_Array_new();

    for (size_t i = 0; i < space->bodies->size; i++) {
        nv_Body *a = (nv_Body *)space->bodies->data[i];
        nv_AABB abox = nv_Body_get_aabb(a);

        for (size_t j = 0; j < space->bodies->size; j++) {
            nv_Body *b = (nv_Body *)space->bodies->data[j];

            // Same body
            if (a == b)
                continue;

            if (!a->collision || !b->collision)
                continue;

            // Two static bodies do not need to interact
            if (a->type == nv_BodyType_STATIC && b->type == nv_BodyType_STATIC)
                continue;

            // Both bodies are asleep
            if (space->sleeping) {
                if (a->is_sleeping && b->is_sleeping)
                    continue;
            }

            // Check if a resolution between bodies already exists
            bool exists = false;
            size_t exist_i = 0;
            for (size_t j = 0; j < space->res->size; j++) {
                nv_Resolution *res2 = (nv_Resolution *)space->res->data[j];

                if ((a == res2->a && b == res2->b) ||
                    (b == res2->a && a == res2->b)
                ) {
                    exists = true;
                    exist_i = j;
                    break;
                }
            }

            nv_AABB bbox = nv_Body_get_aabb(b);

            if (nv_collide_aabb_x_aabb(abox, bbox)) {
                nv_BodyPair pair = (nv_BodyPair){a, b};
                bool skip_pair = false;

                // Don't create a pair if it already exists
                for (size_t k = 0; k < pairs->size; k++) {
                    if ((pair.a == ((nv_BodyPair *)pairs->data[k])->a &&
                            pair.b == ((nv_BodyPair *)pairs->data[k])->b) ||
                        (pair.a == ((nv_BodyPair *)pairs->data[k])->b &&
                            pair.b == ((nv_BodyPair *)pairs->data[k])->a)) {
                        skip_pair = true;
                        break;
                    }
                }

                if (skip_pair) continue;

                nv_BodyPair *pair_heap = NV_NEW(nv_BodyPair);
                pair_heap->a = pair.a;
                pair_heap->b = pair.b;

                nv_Array_add(pairs, pair_heap);
            }
            else {
                // Remove seperated collision resolution from array
                if (exists) {
                    free(nv_Array_pop(space->res, exist_i));
                }
            }
        }
    }

    return pairs;
}

void nv_Space_narrowphase(nv_Space *space, nv_Array *pairs) {
    nv_Array *res_arr = space->res;

    for (size_t i = 0; i < pairs->size; i++) {
        nv_Body *a = (nv_Body *)((nv_BodyPair *)pairs->data[i])->a;
        nv_Body *b = (nv_Body *)((nv_BodyPair *)pairs->data[i])->b;

        // Check if a resolution between bodies already exists
        bool exists = false;
        size_t exist_i = 0;
        for (size_t j = 0; j < res_arr->size; j++) {
            nv_Resolution *res2 = (nv_Resolution *)res_arr->data[j];

            if ((a == res2->a && b == res2->b) ||
                (b == res2->a && a == res2->b)
            ) {
                exists = true;
                exist_i = j;
                break;
            }
        }

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

            if (space->sleeping) {
                if (a->is_sleeping && !b->is_sleeping) {
                    nv_float total_energy = nv_Body_get_kinetic_energy(b) +
                                        nv_Body_get_rotational_energy(b);

                    if (total_energy > 1.0)
                        nv_Body_awake(a);
                }

                if (b->is_sleeping && !a->is_sleeping) {
                    nv_float total_energy = nv_Body_get_kinetic_energy(a) +
                                        nv_Body_get_rotational_energy(a);

                    if (total_energy > 1.0)
                        nv_Body_awake(b);
                }
            }

            /*
                If the resolution between bodies already exists then
                just update it. Else, create a new resolution.
            */
            if (exists) {
                nv_Resolution *resp = (nv_Resolution *)res_arr->data[exist_i];
                resp->normal = res.normal;
                resp->depth = res.depth;
                resp->collision = res.collision;
                resp->contact_count = res.contact_count;
                resp->contacts[0] = res.contacts[0];
                resp->contacts[1] = res.contacts[1];
            }
            else {
                nv_Resolution *res_heap = NV_NEW(nv_Resolution);
                res_heap->a = res.a;
                res_heap->b = res.b;
                res_heap->normal = res.normal;
                res_heap->depth = res.depth;
                res_heap->collision = res.collision;
                res_heap->contact_count = res.contact_count;
                res_heap->contacts[0] = res.contacts[0];
                res_heap->contacts[1] = res.contacts[1];
                res_heap->jn[0] = 0.0;
                res_heap->jn[1] = 0.0; 
                res_heap->jt[0] = 0.0;
                res_heap->jt[1] = 0.0; 
                nv_Array_add(res_arr, res_heap);
            }
        }

        else {
            // Remove seperated collision resolution from array
            if (exists) {
                free(nv_Array_pop(res_arr, exist_i));
            }
        }
    }
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