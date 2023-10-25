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
 * @brief Broad-phase algorithms.
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
    nv_HashMap_clear(space->pairs, false);

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
            if (nv_HashMap_get(space->pairs, &(nv_BroadPhasePair){.id_pair=id_pair}) != NULL) continue;

            // Add pair to the pairs map
            nv_HashMap_set(space->pairs, &(nv_BroadPhasePair){.a=a,.b=b,.id_pair=id_pair});

            // Check if resolution already exists
            nv_Resolution *res_value;
            res_value = nv_HashMap_get(space->res, &(nv_Resolution){.a=a, .b=b});
            bool res_exists = (res_value == NULL) ? false : true;

            nv_AABB bbox = nv_Body_get_aabb(b);

            if (nv_collide_aabb_x_aabb(abox, bbox)) {
                // Do narrow-phase and update collision resolutions
                nv_narrow_phase(space, a, b, res_exists, id_pair, res_value);
            }

            else {
                // Remove seperated collision resolution
                if (res_exists) {
                    if (res_value->state == nv_ResolutionState_FIRST || res_value->state == nv_ResolutionState_NORMAL) {
                        res_value->state = nv_ResolutionState_CACHED; // cached
                    }

                    else if (res_value->state == nv_ResolutionState_CACHED) {
                        if (res_value->lifetime <= 0) {
                            nv_HashMap_remove(space->res, &(nv_Resolution){.a=a, .b=b}); 
                        }

                        else {
                            res_value->lifetime--;
                        }
                    }
                }
            }
        }
    }
}


void nv_BroadPhase_spatial_hash_grid(nv_Space *space) {
    nv_SHG_place(space->shg, space->bodies);

    nv_HashMap_clear(space->pairs, false);

    for (size_t i = 0; i < space->bodies->size; i++) {
        nv_Body *a = (nv_Body *)space->bodies->data[i];
        nv_AABB abox = nv_Body_get_aabb(a);

        nv_float min_x = (nv_int16)(abox.min_x / space->shg->cell_width);
        nv_float min_y = (nv_int16)(abox.min_y / space->shg->cell_height);
        nv_float max_x = (nv_int16)(abox.max_x / space->shg->cell_width);
        nv_float max_y = (nv_int16)(abox.max_y / space->shg->cell_height);

        for (nv_int16 y = min_y; y < max_y + 1; y++) {
            for (nv_int16 x = min_x; x < max_x + 1; x++) {

                nv_uint32 neighbors[8];
                bool neighbor_flags[8];
                nv_SHG_get_neighbors(space->shg, x, y, neighbors, neighbor_flags);

                for (size_t j = 0; j < 9; j++) {
                    nv_Array *cell;

                    // Own cell
                    if (j == 8) {
                        cell = nv_SHG_get(space->shg, nv_pair(x, y));
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

                        // Generate ID pair
                        nv_uint32 id_pair;
                        if (a->id < b->id) id_pair = nv_pair(a->id, b->id);
                        else id_pair = nv_pair(b->id, a->id);

                        // Early out
                        if (nv_BroadPhase_early_out(space, a, b)) continue;

                        // Skip if the pair is already checked
                        if (nv_HashMap_get(space->pairs, &(nv_BroadPhasePair){.id_pair=id_pair}) != NULL) continue;

                        // Add pair to pairs map
                        nv_HashMap_set(space->pairs, &(nv_BroadPhasePair){.a=a,.b=b,.id_pair=id_pair});

                        nv_Resolution *res_value;
                        res_value = nv_HashMap_get(space->res, &(nv_Resolution){.a=a, .b=b});
                        bool res_exists = (res_value == NULL) ? false : true;

                        nv_AABB bbox = nv_Body_get_aabb(b);

                        if (nv_collide_aabb_x_aabb(abox, bbox)) {
                            // Do narrow-phase and update collision resolutions
                            nv_narrow_phase(space, a, b, res_exists, id_pair, res_value);
                        }

                        else {
                            // Remove seperated collision resolution
                            if (res_exists) {
                                if (res_value->state == nv_ResolutionState_FIRST || res_value->state == nv_ResolutionState_NORMAL) {
                                    res_value->state = nv_ResolutionState_CACHED; // cached
                                }

                                else if (res_value->state == nv_ResolutionState_CACHED) {
                                    if (res_value->lifetime <= 0) {
                                        nv_HashMap_remove(space->res, &(nv_Resolution){.a=a, .b=b});
                                    }

                                    else {
                                        res_value->lifetime--;
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    nv_float threshold = 3.0;
    nv_float t2 = threshold * threshold;

    size_t iter = 0;
    void *item;
    while (nv_HashMap_iter(space->res, &iter, &item)) {
        nv_Resolution *res = (nv_Resolution *)item;
        nv_Body *a = res->a;
        nv_Body *b = res->b;

        if (
            nv_Vector2_len2(a->linear_velocity) > t2 ||
            nv_Vector2_len2(b->linear_velocity) > t2
        ) {

            nv_uint32 id_pair;
            if (a->id < b->id) id_pair = nv_pair(a->id, b->id);
            else id_pair = nv_pair(b->id, a->id);

            nv_AABB abox = nv_Body_get_aabb(a);
            nv_AABB bbox = nv_Body_get_aabb(b);

            nv_Resolution *res_value;
            res_value = nv_HashMap_get(space->res, &(nv_Resolution){.a=a, .b=b});
            bool res_exists = (res_value == NULL) ? false : true;

            if (res_exists) {
                if (nv_collide_aabb_x_aabb(abox, bbox)) {
                    nv_narrow_phase(space, a, b, res_exists, id_pair, res_value);
                }
                else {
                    //nv_HashMap_remove(space->res, &(nv_Resolution){.a=a, .b=b});
                    //iter = 0;
                    if (res_value->state == nv_ResolutionState_FIRST || res_value->state == nv_ResolutionState_NORMAL) {
                        res_value->state = nv_ResolutionState_CACHED; // cached
                    }

                    else if (res_value->state == nv_ResolutionState_CACHED) {
                        if (res_value->lifetime <= 0) {
                            nv_HashMap_remove(space->res, &(nv_Resolution){.a=a, .b=b});
                            iter = 0;
                        }

                        else {
                            res_value->lifetime--;
                        }
                    }
                }
            }
        }
    }
}


void nv_narrow_phase(
    nv_Space *space,
    nv_Body *a,
    nv_Body *b,
    bool res_exists,
    nv_uint32 res_key,
    nv_Resolution *res_value
) {
    nv_Resolution res;
    res.collision = false;

    if (a->shape->type == nv_ShapeType_CIRCLE && b->shape->type == nv_ShapeType_CIRCLE)
        res = nv_collide_circle_x_circle(a, b);

    else if (a->shape->type == nv_ShapeType_CIRCLE && b->shape->type == nv_ShapeType_POLYGON)
        res = nv_collide_polygon_x_circle(b, a);

    else if (a->shape->type == nv_ShapeType_POLYGON && b->shape->type == nv_ShapeType_CIRCLE)
        res = nv_collide_polygon_x_circle(a, b);

    else if (a->shape->type == nv_ShapeType_POLYGON && b->shape->type == nv_ShapeType_POLYGON) {
        res.a = a;
        res.b = b;
        nv_contact_polygon_x_polygon(&res);
    }

    if (res.collision) {
        if (a->shape->type == nv_ShapeType_CIRCLE && b->shape->type == nv_ShapeType_CIRCLE)
            nv_contact_circle_x_circle(&res);

        else if (a->shape->type == nv_ShapeType_CIRCLE && b->shape->type == nv_ShapeType_POLYGON)
            nv_contact_polygon_x_circle(&res);

        else if (a->shape->type == nv_ShapeType_POLYGON && b->shape->type == nv_ShapeType_CIRCLE)
            nv_contact_polygon_x_circle(&res);

        /*
            If one body is asleep and other is not, wake up the asleep body
            depending on the awake body's motion.
        */
        if (space->sleeping) {
            if (a->is_sleeping && (!b->is_sleeping && b->type != nv_BodyType_STATIC)) {
                nv_float linear = nv_Vector2_len2(b->linear_velocity) * (1.0 / 60.0);
                nv_float angular = b->angular_velocity * (1.0 / 60.0);
                nv_float total_energy = linear + angular;

                if (total_energy > space->wake_energy_threshold)
                    nv_Body_awake(a);
            }

            if (b->is_sleeping && (!a->is_sleeping && a->type != nv_BodyType_STATIC)) {
                nv_float linear = nv_Vector2_len2(a->linear_velocity) * (1.0 / 60.0);
                nv_float angular = a->angular_velocity * (1.0 / 60.0);
                nv_float total_energy = linear + angular;

                if (total_energy > space->wake_energy_threshold)
                    nv_Body_awake(b);
            }
        }

        /*
            If the resolution between bodies already exists then
            just update it. Else, create a new resolution.
        */
        if (res_exists) {
            res_value->normal = res.normal;
            res_value->depth = res.depth;
            res_value->collision = res.collision;
            res_value->contact_count = res.contact_count;
            res_value->contacts[0] = res.contacts[0];
            res_value->contacts[1] = res.contacts[1];

            if (res_value->state == nv_ResolutionState_CACHED) {
                res_value->lifetime = space->collision_persistence;
                res_value->state = nv_ResolutionState_FIRST;
            }
            else if (res_value->state == nv_ResolutionState_FIRST) {
                res_value->state = nv_ResolutionState_NORMAL;
            }
        }
        else {
            nv_Resolution res_new;
            res_new.a = res.a;
            res_new.b = res.b;
            res_new.normal = res.normal;
            res_new.depth = res.depth;
            res_new.collision = res.collision;
            res_new.contact_count = res.contact_count;
            res_new.contacts[0] = res.contacts[0];
            res_new.contacts[1] = res.contacts[1];
            res_new.jn[0] = 0.0;
            res_new.jn[1] = 0.0; 
            res_new.jt[0] = 0.0;
            res_new.jt[1] = 0.0; 
            res_new.state = nv_ResolutionState_FIRST;
            res_new.lifetime = space->collision_persistence;
            nv_HashMap_set(space->res, &res_new);
        }
    }

    else {
        // Remove separated collision resolution from array
        if (res_exists) {
            if (res_value->state == nv_ResolutionState_FIRST || res_value->state == nv_ResolutionState_NORMAL) {
                res_value->state = 2; // cached
            }

            else if (res_value->state == nv_ResolutionState_CACHED) {
                if (res_value->lifetime <= 0) {
                    nv_HashMap_remove(space->res, &(nv_Resolution){.a=a, .b=b});
                }

                else {
                    res_value->lifetime--;
                }
            }
        }
    }
}