/*

  This file is a part of the Nova Physics Engine
  project and distributed under the MIT license.

  Copyright © Kadir Aksoy
  https://github.com/kadir014/nova-physics

*/

#include <stdbool.h>
#include "novaphysics/broadphase.h"
#include "novaphysics/array.h"
#include "novaphysics/aabb.h"
#include "novaphysics/body.h"
#include "novaphysics/resolution.h"
#include "novaphysics/space.h"


/**
 * @file broadphase.c
 * 
 * Broad-phase
 */


void nv_BroadPhase_brute_force(nv_Space *space) {
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

            if (space->sleeping) {
                // Both bodies are asleep
                if (a->is_sleeping && b->is_sleeping)
                    continue;

                // One body is asleep and other is static
                if ((a->is_sleeping && b->type == nv_BodyType_STATIC) ||
                    (b->is_sleeping && a->type == nv_BodyType_STATIC))
                    continue;
            }

            // Check if a resolution between bodies already exists
            size_t res_index;
            size_t res_exists = nv_check_resolution_duplicate(space->res, a, b, &res_index);
            
            // Check for existing pair
            nv_BodyPair pair = (nv_BodyPair){a, b};
            bool skip_pair = false;

            // Don't create a pair if it already exists
            for (size_t k = 0; k < pairs->size; k++) {
                if ((pair.a == ((nv_BodyPair *)pairs->data[k])->a  &&
                     pair.b == ((nv_BodyPair *)pairs->data[k])->b) ||
                    (pair.a == ((nv_BodyPair *)pairs->data[k])->b  &&
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

                // Do narrow-phase and update collision resolutions
                nv_narrow_phase(space, a, b, res_exists, res_index);
            }

            else {
                // Remove seperated collision resolution from array and awake bodies
                if (res_exists) {
                    free(nv_Array_pop(space->res, res_index));
                }
            }
        }
    }

    nv_Array_free(pairs);
}

void nv_BroadPhase_spatial_hash_grid(nv_Space *space) {

}


void nv_narrow_phase(
    nv_Space *space,
    nv_Body *a,
    nv_Body *b,
    bool res_exists,
    size_t res_index
) {
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

        /*
            If one body is asleep and other is not, wake up the asleep body
            depending on the awake body's motion.
        */
        if (space->sleeping) {
            if (a->is_sleeping && (!b->is_sleeping && b->type != nv_BodyType_STATIC)) {
                nv_float total_energy =
                nv_Vector2_len(b->linear_velocity) + b->angular_velocity;

                if (total_energy > space->wake_energy_threshold)
                    nv_Body_awake(a);
            }

            if (b->is_sleeping && (!a->is_sleeping && a->type != nv_BodyType_STATIC)) {
                nv_float total_energy =
                nv_Vector2_len(a->linear_velocity) + a->angular_velocity;

                if (total_energy > space->wake_energy_threshold)
                    nv_Body_awake(b);
            }
        }

        /*
            If the resolution between bodies already exists then
            just update it. Else, create a new resolution.
        */
        if (res_exists) {
            nv_Resolution *resp = (nv_Resolution *)space->res->data[res_index];
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
        if (res_exists) {
            free(nv_Array_pop(space->res, res_index));
        }
    }
}


bool nv_check_resolution_duplicate(
    nv_Array *res,
    nv_Body *a,
    nv_Body *b,
    size_t *index
) {
    for (size_t j = 0; j < res->size; j++) {
        nv_Resolution *res2 = (nv_Resolution *)res->data[j];

        if ((a == res2->a && b == res2->b) ||
            (b == res2->a && a == res2->b)
        ) {
            *index = j;
            return true;
        }
    }

    return false;
}