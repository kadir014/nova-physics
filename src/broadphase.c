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
#include "novaphysics/math.h"
#include "novaphysics/resolution.h"
#include "novaphysics/space.h"


/**
 * @file broadphase.c
 * 
 * @details Broad-phase algorithms

 *          Function documentations are in novaphysics/broadphase.h
 */


bool nv_BroadPhase_early_out(nv_Space *space, nv_Body *a, nv_Body *b) {
    // Same body
    if (a == b)
        return true;

    if (!a->collision || !b->collision)
        return true;

    // Two static bodies do not need to interact
    if (a->type == nv_BodyType_STATIC && b->type == nv_BodyType_STATIC)
        return true;

    if (space->sleeping) {
        // Both bodies are asleep
        if (a->is_sleeping && b->is_sleeping)
            return true;

        // One body is asleep and other is static
        if ((a->is_sleeping && b->type == nv_BodyType_STATIC) ||
            (b->is_sleeping && a->type == nv_BodyType_STATIC))
            return true;
    }

    return false;
}


void nv_BroadPhase_brute_force(nv_Space *space) {
    nv_HashMap *pairs = nv_HashMap_new();

    for (size_t i = 0; i < space->bodies->size; i++) {
        nv_Body *a = (nv_Body *)space->bodies->data[i];
        nv_AABB abox = nv_Body_get_aabb(a);

        for (size_t j = 0; j < space->bodies->size; j++) {
            nv_Body *b = (nv_Body *)space->bodies->data[j];

            // Early out
            if (nv_BroadPhase_early_out(space, a, b)) continue;

            // Generate ID pair
            nv_uint32 id_pair;
            if (a->id < b->id) id_pair = nv_pair(a->id, b->id);
            else id_pair = nv_pair(b->id, a->id);
            
            // Check for existing pair
            if (nv_HashMap_get(pairs, id_pair) != NULL) continue;

            // Check if resolution already exists
            void *res_value = nv_HashMap_get(space->res, id_pair);
            bool res_exists = (res_value == NULL) ? false : true;

            nv_AABB bbox = nv_Body_get_aabb(b);

            if (nv_collide_aabb_x_aabb(abox, bbox)) {
                nv_HashMap_set(pairs, id_pair, &id_pair);

                // Do narrow-phase and update collision resolutions
                nv_narrow_phase(space, a, b, res_exists, id_pair, res_value);
            }

            else {
                // Remove seperated collision resolution
                if (res_exists) {
                    nv_HashMap_remove(space->res, id_pair, free);
                }
            }
        }
    }
    
    nv_HashMap_free(pairs, NULL);
}

void nv_BroadPhase_spatial_hash_grid(nv_Space *space) {
    nv_SHG_place(space->shg, space->bodies);

    nv_HashMap *pairs = nv_HashMap_new();

    // TODO: AABB neighbors

    for (size_t i = 0; i < space->bodies->size; i++) {
        nv_Body *a = (nv_Body *)space->bodies->data[i];
        nv_AABB abox = nv_Body_get_aabb(a);

        int16_t cell_x = (int16_t)(a->position.x / space->shg->cell_width);
        int16_t cell_y = (int16_t)(a->position.y / space->shg->cell_height);

        uint32_t neighbors[8];
        bool neighbor_flags[8];
        nv_SHG_get_neighbors(space->shg, cell_x, cell_y, neighbors, neighbor_flags);

        for (size_t j = 0; j < 9; j++) {
            nv_Array *cell;

            // Own cell
            if (j == 8) {
                cell = nv_SHG_get(space->shg, nv_pair(cell_x, cell_y));
                if (cell == NULL) continue;
            }
            // Neighbor cells
            else {
                if (!neighbor_flags[j]) continue;

                cell = nv_SHG_get(space->shg, neighbors[j]);
                if (cell == NULL) continue;
            }

            for (size_t k = 0; k < cell->size; k++) {
                nv_Body *b = (nv_Body *)cell->data[k];

                // Early out
                if (nv_BroadPhase_early_out(space, a, b)) continue;

                // Generate ID pair
                uint32_t id_pair;
                if (a < b) id_pair = nv_pair(a->id, b->id);
                else id_pair = nv_pair(b->id, a->id);

                if (nv_HashMap_get(pairs, id_pair) != NULL) continue;

                void *res_value = nv_HashMap_get(space->res, id_pair);
                bool res_exists = (res_value == NULL) ? false : true;

                nv_AABB bbox = nv_Body_get_aabb(b);

                if (nv_collide_aabb_x_aabb(abox, bbox)) {
                    nv_HashMap_set(pairs, id_pair, &id_pair);

                    // Do narrow-phase and update collision resolutions
                    nv_narrow_phase(space, a, b, res_exists, id_pair, res_value);
                }

                else {
                    // Remove seperated collision resolution
                    if (res_exists) {
                        nv_HashMap_remove(space->res, id_pair, free);
                    }
                }
            }
        }
    }

    nv_HashMap_free(pairs, NULL);
}


void nv_narrow_phase(
    nv_Space *space,
    nv_Body *a,
    nv_Body *b,
    bool res_exists,
    uint32_t res_key,
    void *res_value
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
            nv_Resolution *resp = (nv_Resolution *)res_value;
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
            nv_HashMap_set(space->res, res_key, res_heap);
        }
    }

    else {
        // Remove seperated collision resolution from array
        if (res_exists) {
            nv_HashMap_remove(space->res, res_key, free);
        }
    }
}