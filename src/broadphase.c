/*

  This file is a part of the Nova Physics Engine
  project and distributed under the MIT license.

  Copyright © Kadir Aksoy
  https://github.com/kadir014/nova-physics

*/

#include <stdbool.h>
#include "novaphysics/internal.h"
#include "novaphysics/broadphase.h"
#include "novaphysics/array.h"
#include "novaphysics/aabb.h"
#include "novaphysics/body.h"
#include "novaphysics/math.h"
#include "novaphysics/resolution.h"
#include "novaphysics/space.h"
#include "novaphysics/threading.h"


/**
 * @file broadphase.c
 * 
 * @brief Broad-phase algorithms.
 */


/**
 * @brief Early-out from checking collisions.
 */
static inline bool nvBroadPhase_early_out(nvSpace *space, nvBody *a, nvBody *b) {
    // Same body
    if (a == b)
        return true;

    // One of the bodies have collision detection disabled
    if (!a->enable_collision || !b->enable_collision)
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

    // Bodies share the same non-zero group
    if (a->collision_group == b->collision_group && a->collision_group != 0)
        return true;

    // One of the collision mask doesn't fit the category
    if ((a->collision_mask & b->collision_category) == 0 ||
        (b->collision_mask & a->collision_category) == 0)
        return true;

    return false;
}


/**
 * @brief Update the lifetime and state of the resolution.
 */
static inline void nv_update_resolution(
    nvSpace *space,
    nvBody *a,
    nvBody *b,
    nvResolution *res
) {
    if (space->multithreading) {
        if (!nvMutex_lock(space->res_mutex)) {
            NV_ERROR("Error occured while locking res mutex. (nv_update_resolution)\n");
        }
    }

    if (
        res->state == nvResolutionState_FIRST ||
        res->state == nvResolutionState_NORMAL
    ) {
        res->state = nvResolutionState_CACHED;
    }

    else if (res->state == nvResolutionState_CACHED) {
        if (res->lifetime <= 0) {
            nvHashMap_remove(space->res, &(nvResolution){.a=a, .b=b});
        }

        else {
            res->lifetime--;
        }
    }

    if (space->multithreading) {
        if (!nvMutex_unlock(space->res_mutex)) {
           NV_ERROR("Error occured while unlocking res mutex. (nv_update_resolution)\n");
        }
    }
}

/**
 * @brief Check AABBs of bodies, update resolutions if needed.
 */
static inline void nv_midphase(
        nvSpace *space, 
        nvBody *a,
        nvBody *b,
        nvAABB abox,
        nvAABB bbox,
        bool res_exists,
        nvResolution *found_res
) {
    if (nv_collide_aabb_x_aabb(abox, bbox)) {
        nv_narrow_phase(space, a, b, res_exists, found_res);
    }
    
    else if (res_exists) {
        nv_update_resolution(space, a, b, found_res);
    }
}


void nvBroadPhase_brute_force(nvSpace *space) {
    nvHashMap_clear(space->pairs);

    for (size_t i = 0; i < space->bodies->size; i++) {
        nvBody *a = (nvBody *)space->bodies->data[i];
        nvAABB abox = nvBody_get_aabb(a);

        for (size_t j = 0; j < space->bodies->size; j++) {
            nvBody *b = (nvBody *)space->bodies->data[j];

            // Early out
            if (nvBroadPhase_early_out(space, a, b)) continue;

            // Generate ID pair
            nv_uint32 id_pair;
            if (a->id < b->id) id_pair = nv_pair(a->id, b->id);
            else id_pair = nv_pair(b->id, a->id);

            // Check for existing pair
            if (nvHashMap_get(space->pairs, &(nvBroadPhasePair){.id_pair=id_pair}) != NULL) continue;

            // Add pair to the pairs map
            nvHashMap_set(space->pairs, &(nvBroadPhasePair){.a=a,.b=b,.id_pair=id_pair});

            // Check if resolution already exists
            nvResolution *res_value;
            res_value = nvHashMap_get(space->res, &(nvResolution){.a=a, .b=b});
            bool res_exists = (res_value == NULL) ? false : true;

            nvAABB bbox = nvBody_get_aabb(b);

            nv_midphase(space, a, b, abox, bbox, res_exists, res_value);
        }
    }
}


void nvBroadPhase_SHG(nvSpace *space) {
    NV_TRACY_ZONE_START;
    
    nvSHG_place(space->shg, space->bodies);

    nvHashMap_clear(space->pairs);

    for (size_t i = 0; i < space->bodies->size; i++) {
        nvBody *a = (nvBody *)space->bodies->data[i];
        nvAABB abox = nvBody_get_aabb(a);

        nv_float min_x = (nv_int16)(abox.min_x / space->shg->cell_width);
        nv_float min_y = (nv_int16)(abox.min_y / space->shg->cell_height);
        nv_float max_x = (nv_int16)(abox.max_x / space->shg->cell_width);
        nv_float max_y = (nv_int16)(abox.max_y / space->shg->cell_height);

        for (nv_int16 y = min_y; y < max_y + 1; y++) {
            for (nv_int16 x = min_x; x < max_x + 1; x++) {

                nv_uint32 neighbors[8];
                bool neighbor_flags[8];
                nvSHG_get_neighbors(space->shg, x, y, neighbors, neighbor_flags);

                for (size_t j = 0; j < 9; j++) {
                    nvArray *cell;

                    // Own cell
                    if (j == 8) {
                        cell = nvSHG_get(space->shg, nv_pair(x, y));
                        if (!cell) continue;
                    }
                    // Neighbor cells
                    else {
                        if (!neighbor_flags[j]) continue;

                        cell = nvSHG_get(space->shg, neighbors[j]);
                        if (!cell) continue;
                    }

                    for (size_t k = 0; k < cell->size; k++) {
                        nvBody *b = (nvBody *)cell->data[k];

                        // Early out
                        if (nvBroadPhase_early_out(space, a, b)) continue;

                        // Generate ID pair
                        nv_uint32 id_pair;
                        if (a->id < b->id) id_pair = nv_pair(a->id, b->id);
                        else id_pair = nv_pair(b->id, a->id);

                        // Skip if the pair is already checked
                        if (nvHashMap_get(space->pairs, &(nvBroadPhasePair){.id_pair=id_pair}) != NULL) continue;

                        // Add pair to pairs map
                        nvHashMap_set(space->pairs, &(nvBroadPhasePair){.a=a,.b=b,.id_pair=id_pair});

                        nvResolution *res_value;
                        res_value = nvHashMap_get(space->res, &(nvResolution){.a=a, .b=b});
                        bool res_exists = (res_value == NULL) ? false : true;

                        nvAABB bbox = nvBody_get_aabb(b);

                        nv_midphase(space, a, b, abox, bbox, res_exists, res_value);
                    }
                }
            }
        }
    }

    nv_float threshold = 3.0;
    nv_float t2 = threshold * threshold;

    size_t iter = 0;
    void *item;
    while (nvHashMap_iter(space->res, &iter, &item)) {
        nvResolution *res = (nvResolution *)item;
        nvBody *a = res->a;
        nvBody *b = res->b;

        if (
            nvVector2_len2(a->linear_velocity) > t2 ||
            nvVector2_len2(b->linear_velocity) > t2
        ) {
            nvAABB abox = nvBody_get_aabb(a);
            nvAABB bbox = nvBody_get_aabb(b);

            nvResolution *res_value;
            res_value = nvHashMap_get(space->res, &(nvResolution){.a=a, .b=b});
            bool res_exists = (res_value == NULL) ? false : true;

            if (res_exists)
                nv_midphase(space, a, b, abox, bbox, res_exists, res_value);
        }
    }

    NV_TRACY_ZONE_END;
}


typedef struct {
    struct nvSpace *space;
    nvArray *bodies;
    nvMutex *pair_mutex;
} SHGWorkerData;

static int nvBroadPhase_SHG_task(nvThreadWorkerData *data) {
    NV_TRACY_ZONE_START;

    nvSpace *space = ((SHGWorkerData *)(data->data))->space;
    nvArray *bodies = ((SHGWorkerData *)(data->data))->bodies;
    nvMutex *pair_mutex = ((SHGWorkerData *)(data->data))->pair_mutex;

    for (size_t i = 0; i < bodies->size; i++) {
        nvBody *a = (nvBody *)bodies->data[i];
        nvAABB abox = nvBody_get_aabb(a);

        nv_float min_x = (nv_int16)(abox.min_x / space->shg->cell_width);
        nv_float min_y = (nv_int16)(abox.min_y / space->shg->cell_height);
        nv_float max_x = (nv_int16)(abox.max_x / space->shg->cell_width);
        nv_float max_y = (nv_int16)(abox.max_y / space->shg->cell_height);

        for (nv_int16 y = min_y; y < max_y + 1; y++) {
            for (nv_int16 x = min_x; x < max_x + 1; x++) {

                nv_uint32 neighbors[8];
                bool neighbor_flags[8];
                nvSHG_get_neighbors(space->shg, x, y, neighbors, neighbor_flags);

                for (size_t j = 0; j < 9; j++) {
                    nvArray *cell;

                    // Own cell
                    if (j == 8) {
                        cell = nvSHG_get(space->shg, nv_pair(x, y));
                        if (!cell) continue;
                    }
                    // Neighbor cells
                    else {
                        if (!neighbor_flags[j]) continue;

                        cell = nvSHG_get(space->shg, neighbors[j]);
                        if (!cell) continue;
                    }

                    for (size_t k = 0; k < cell->size; k++) {
                        nvBody *b = (nvBody *)cell->data[k];

                        // Early out
                        if (nvBroadPhase_early_out(space, a, b)) continue;

                        // Generate ID pair
                        nv_uint32 id_pair;
                        if (a->id < b->id) id_pair = nv_pair(a->id, b->id);
                        else id_pair = nv_pair(b->id, a->id);

                        // Skip if the pair is already checked
                        if (nvHashMap_get(space->pairs, &(nvBroadPhasePair){.id_pair=id_pair}) != NULL) continue;

                        // Add pair to pairs map
                        if (!nvMutex_lock(pair_mutex)) continue;

                        nvHashMap_set(space->pairs, &(nvBroadPhasePair){.a=a,.b=b,.id_pair=id_pair});
                        
                        if (!nvMutex_unlock(pair_mutex))
                            NV_ERROR("Error occured while unlocking pair mutex.\n");

                        nvResolution *res_value;
                        res_value = nvHashMap_get(space->res, &(nvResolution){.a=a, .b=b});
                        bool res_exists = (res_value == NULL) ? false : true;

                        nvAABB bbox = nvBody_get_aabb(b);

                        nv_midphase(space, a, b, abox, bbox, res_exists, res_value);
                    }
                }
            }
        }
    }

    NV_TRACY_ZONE_END;
    return 0;
}


void nvBroadPhase_SHG_multithreaded(struct nvSpace *space) {
    NV_TRACY_ZONE_START;
    
    nvSHG_place(space->shg, space->bodies);

    nvHashMap_clear(space->pairs);

    nvArray *a0 = nvArray_new();
    nvArray *a1 = nvArray_new();
    nvArray *a2 = nvArray_new();
    nvArray *a3 = nvArray_new();

    nv_float q = space->shg->bounds.max_x / 4.0;

    for (size_t i = 0; i < space->bodies->size; i++) {
        nvBody *body = space->bodies->data[i];

        if (
            body->position.x > space->shg->bounds.min_x &&
            body->position.x <= q * 1.0
        ) {
            nvArray_add(a0, body);
        }
        else if (
            body->position.x > q * 1.0 &&
            body->position.x < q * 2.0
        ) {
            nvArray_add(a1, body);
        }
        else if (
            body->position.x > q * 2.0 &&
            body->position.x < q * 3.0
        ) {
            nvArray_add(a2, body);
        }
        else if (
            body->position.x > q * 3.0 &&
            body->position.x < space->shg->bounds.max_x
        ) {
            nvArray_add(a3, body);
        }
    }

    nvMutex *pair_mutex = nvMutex_new();

    SHGWorkerData data[4] = {
        {.space = space, .bodies = a0, .pair_mutex=pair_mutex},
        {.space = space, .bodies = a1, .pair_mutex=pair_mutex},
        {.space = space, .bodies = a2, .pair_mutex=pair_mutex},
        {.space = space, .bodies = a3, .pair_mutex=pair_mutex},
    };

    nvThread *threads[4];
    for (size_t i = 0; i < 4; i++) {
        threads[i] = nvThread_create(nvBroadPhase_SHG_task, &data[i]);
    }

    nvThread_join_multiple(threads, 4);

    for (size_t i = 0; i < 4; i++) {
        nvThread_free(threads[i]);
    }
    nvMutex_free(pair_mutex);
    nvArray_free(a0);
    nvArray_free(a1);
    nvArray_free(a2);
    nvArray_free(a3);


    nv_float threshold = 3.0;
    nv_float t2 = threshold * threshold;

    size_t iter = 0;
    void *item;
    while (nvHashMap_iter(space->res, &iter, &item)) {
        nvResolution *res = (nvResolution *)item;
        nvBody *a = res->a;
        nvBody *b = res->b;

        if (
            nvVector2_len2(a->linear_velocity) > t2 ||
            nvVector2_len2(b->linear_velocity) > t2
        ) {
            nvAABB abox = nvBody_get_aabb(a);
            nvAABB bbox = nvBody_get_aabb(b);

            nvResolution *res_value;
            res_value = nvHashMap_get(space->res, &(nvResolution){.a=a, .b=b});
            bool res_exists = (res_value == NULL) ? false : true;

            if (res_exists)
                nv_midphase(space, a, b, abox, bbox, res_exists, res_value);
        }
    }

    NV_TRACY_ZONE_END;
}


void nv_narrow_phase(
    nvSpace *space,
    nvBody *a,
    nvBody *b,
    bool res_exists,
    nvResolution *found_res
) {
    NV_TRACY_ZONE_START;

    nvResolution res;
    res.collision = false;

    if (a->shape->type == nvShapeType_CIRCLE && b->shape->type == nvShapeType_CIRCLE)
        res = nv_collide_circle_x_circle(a, b);

    else if (a->shape->type == nvShapeType_CIRCLE && b->shape->type == nvShapeType_POLYGON)
        res = nv_collide_polygon_x_circle(b, a);

    else if (a->shape->type == nvShapeType_POLYGON && b->shape->type == nvShapeType_CIRCLE)
        res = nv_collide_polygon_x_circle(a, b);

    else if (a->shape->type == nvShapeType_POLYGON && b->shape->type == nvShapeType_POLYGON) {
        res.a = a;
        res.b = b;
        nv_contact_polygon_x_polygon(&res);
    }

    if (res.collision) {
        if (a->shape->type == nvShapeType_CIRCLE && b->shape->type == nvShapeType_CIRCLE)
            nv_contact_circle_x_circle(&res);

        else if (a->shape->type == nvShapeType_CIRCLE && b->shape->type == nvShapeType_POLYGON)
            nv_contact_polygon_x_circle(&res);

        else if (a->shape->type == nvShapeType_POLYGON && b->shape->type == nvShapeType_CIRCLE)
            nv_contact_polygon_x_circle(&res);

        /*
            If one body is asleep and other is not, wake up the asleep body
            depending on the awake body's motion.
        */
        if (space->sleeping) {
            if (a->is_sleeping && (!b->is_sleeping && b->type != nv_BodyType_STATIC)) {
                nv_float linear = nvVector2_len2(b->linear_velocity) * (1.0 / 60.0);
                nv_float angular = b->angular_velocity * (1.0 / 60.0);
                nv_float total_energy = linear + angular;

                if (total_energy > space->wake_energy_threshold)
                    nvBody_awake(a);
            }

            if (b->is_sleeping && (!a->is_sleeping && a->type != nv_BodyType_STATIC)) {
                nv_float linear = nvVector2_len2(a->linear_velocity) * (1.0 / 60.0);
                nv_float angular = a->angular_velocity * (1.0 / 60.0);
                nv_float total_energy = linear + angular;

                if (total_energy > space->wake_energy_threshold)
                    nvBody_awake(b);
            }
        }

        /*
            If the resolution between bodies already exists then
            just update it. Else, create a new resolution.
        */
        if (res_exists) {
            if (space->multithreading) {
                if (!nvMutex_lock(space->res_mutex)) {
                    NV_ERROR("Error occured while locking res mutex. (nv_narrow_phase)\n");
                }
            }

            found_res->normal = res.normal;
            found_res->depth = res.depth;
            found_res->collision = res.collision;
            found_res->contact_count = res.contact_count;
            found_res->contacts[0].position = res.contacts[0].position;
            found_res->contacts[1].position = res.contacts[1].position;

            if (found_res->state == nvResolutionState_CACHED) {
                found_res->lifetime = space->collision_persistence;
                found_res->state = nvResolutionState_FIRST;
            }
            else if (found_res->state == nvResolutionState_FIRST) {
                found_res->state = nvResolutionState_NORMAL;
            }

            if (space->multithreading) {
                if (!nvMutex_unlock(space->res_mutex)) {
                    NV_ERROR("Error occured while unlocking res mutex. (nv_narrow_phase)\n");
                }
            }
        }
        else {
            nvResolution res_new;
            res_new.a = res.a;
            res_new.b = res.b;
            res_new.normal = res.normal;
            res_new.depth = res.depth;
            res_new.collision = res.collision;
            res_new.contact_count = res.contact_count;
            res_new.contacts[0] = res.contacts[0];
            res_new.contacts[1] = res.contacts[1];
            res_new.contacts[0].jb = 0.0;
            res_new.contacts[1].jb = 0.0;
            res_new.contacts[0].jn = 0.0;
            res_new.contacts[1].jn = 0.0; 
            res_new.contacts[0].jt = 0.0;
            res_new.contacts[1].jt = 0.0; 
            res_new.state = nvResolutionState_FIRST;
            res_new.lifetime = space->collision_persistence;

            if (space->multithreading) {
                if (!nvMutex_lock(space->res_mutex)) {
                    NV_ERROR("Error occured while locking res mutex. (nv_narrow_phase)\n");
                }
            }
            
            nvHashMap_set(space->res, &res_new);
                
            if (space->multithreading) {
                if (!nvMutex_unlock(space->res_mutex)) {
                    NV_ERROR("Error occured while unlocking res mutex. (nv_narrow_phase)\n");
                }
            }
        }
    }

    else if (res_exists) {
        nv_update_resolution(space, a, b, found_res);
    }

    NV_TRACY_ZONE_END;
}