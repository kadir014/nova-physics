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
#include "novaphysics/bvh.h"
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
    if (a->type == nvBodyType_STATIC && b->type == nvBodyType_STATIC)
        return true;

    if (space->sleeping) {
        // Both bodies are asleep
        if (a->is_sleeping && b->is_sleeping)
            return true;

        // One body is asleep and other is static
        if ((a->is_sleeping && b->type == nvBodyType_STATIC) ||
            (b->is_sleeping && a->type == nvBodyType_STATIC))
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
 * @brief Check AABBs of bodies, update resolutions if needed.
 */
static inline void nv_midphase(
        nvSpace *space, 
        nvBody *a,
        nvBody *b,
        nvAABB abox,
        nvAABB bbox,
        bool res_exists,
        nvResolution *found_res,
        nv_uint32 id_pair
) {
    if (nv_collide_aabb_x_aabb(abox, bbox)) {
        nvHashMap_set(space->broadphase_pairs, &(nvBroadPhasePair){.a=a, .b=b, .id_pair=id_pair});
    }
    
    else if (res_exists) {
        nvResolution_update(space, a, b, found_res);
    }
}


void nvBroadPhase_brute_force(nvSpace *space) {
    nvHashMap_clear(space->broadphase_pairs);
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

            nv_midphase(space, a, b, abox, bbox, res_exists, res_value, id_pair);
        }
    }
}


void nvBroadPhase_SHG(nvSpace *space) {
    NV_TRACY_ZONE_START;

    nvHashMap_clear(space->broadphase_pairs);
    
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

                        nv_midphase(space, a, b, abox, bbox, res_exists, res_value, id_pair);
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

            if (res_exists) {
                nv_uint32 id_pair;
                if (a->id < b->id) id_pair = nv_pair(a->id, b->id);
                else id_pair = nv_pair(b->id, a->id);

                nv_midphase(space, a, b, abox, bbox, res_exists, res_value, id_pair);
            }
        }
    }

    NV_TRACY_ZONE_END;
}


typedef struct {
    struct nvSpace *space;
    nvArray *bodies;
    nvHashMap *pairs;
    nv_uint8 task_id;
} SHGWorkerData;


static int nvBroadPhase_SHG_task(void *data) {
    NV_TRACY_ZONE_START;

    nvSpace *space = (((SHGWorkerData *)data))->space;
    nvArray *bodies = ((SHGWorkerData *)data)->bodies;
    nvHashMap *pairs = ((SHGWorkerData *)data)->pairs;
    nv_uint8 task_id = ((SHGWorkerData *)data)->task_id;

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
                        if (nvHashMap_get(pairs, &(nvBroadPhasePair){.id_pair=id_pair}) != NULL) continue;

                        // Add pair to pairs map
                        nvHashMap_set(pairs, &(nvBroadPhasePair){.a=a,.b=b,.id_pair=id_pair});

                        nvResolution *res_value;
                        res_value = nvHashMap_get(space->res, &(nvResolution){.a=a, .b=b});
                        bool res_exists = (res_value == NULL) ? false : true;

                        nvAABB bbox = nvBody_get_aabb(b);

                        // Midphase
                        if (nv_collide_aabb_x_aabb(abox, bbox)) {
                            nvBroadPhasePair *pair = NV_NEW(nvBroadPhasePair);
                            pair->a = a;
                            pair->b = b;
                            pair->id_pair = id_pair;

                            switch (task_id) {
                                case 0:
                                    nvArray_add(space->broadphase_pairs0, pair);
                                    break;

                                case 1:
                                    nvArray_add(space->broadphase_pairs1, pair);
                                    break;

                                case 2:
                                    nvArray_add(space->broadphase_pairs2, pair);
                                    break;

                                case 3:
                                    nvArray_add(space->broadphase_pairs3, pair);
                                    break;
                            }
                        }
                        else if (res_exists) {
                            nvResolution_update(space, a, b, res_value);
                        }
                    }
                }
            }
        }
    }

    NV_TRACY_ZONE_END;
    return 0;
}


void nvBroadPhase_SHG_multithreaded(nvSpace *space) {
    NV_TRACY_ZONE_START;
    
    nvSHG_place(space->shg, space->bodies);

    nvHashMap_clear(space->broadphase_pairs);
    nvHashMap_clear(space->pairs0);
    nvHashMap_clear(space->pairs1);
    nvHashMap_clear(space->pairs2);
    nvHashMap_clear(space->pairs3);
    nvArray_clear(space->broadphase_pairs0, free);
    nvArray_clear(space->broadphase_pairs1, free);
    nvArray_clear(space->broadphase_pairs2, free);
    nvArray_clear(space->broadphase_pairs3, free);
    nvArray_clear(space->split0, NULL);
    nvArray_clear(space->split1, NULL);
    nvArray_clear(space->split2, NULL);
    nvArray_clear(space->split3, NULL);

    nv_float q = space->shg->bounds.max_x / 4.0;

    for (size_t i = 0; i < space->bodies->size; i++) {
        nvBody *body = space->bodies->data[i];

        if (
            body->position.x > space->shg->bounds.min_x &&
            body->position.x <= q * 1.0
        ) {
            nvArray_add(space->split0, body);
        }
        else if (
            body->position.x > q * 1.0 &&
            body->position.x <= q * 2.0
        ) {
            nvArray_add(space->split1, body);
        }
        else if (
            body->position.x > q * 2.0 &&
            body->position.x <= q * 3.0
        ) {
            nvArray_add(space->split2, body);
        }
        else if (
            body->position.x > q * 3.0 &&
            body->position.x <= space->shg->bounds.max_x
        ) {
            nvArray_add(space->split3, body);
        }
    }

    SHGWorkerData data[4] = {
        {.space = space, .bodies = space->split0, .pairs = space->pairs0, .task_id=0},
        {.space = space, .bodies = space->split1, .pairs = space->pairs1, .task_id=1},
        {.space = space, .bodies = space->split2, .pairs = space->pairs2, .task_id=2},
        {.space = space, .bodies = space->split3, .pairs = space->pairs3, .task_id=3}
    };

    nvTaskExecutor_add_task_to(space->task_executor, nvBroadPhase_SHG_task, &data[0], 0);
    nvTaskExecutor_add_task_to(space->task_executor, nvBroadPhase_SHG_task, &data[1], 1);
    nvTaskExecutor_add_task_to(space->task_executor, nvBroadPhase_SHG_task, &data[2], 2);
    nvTaskExecutor_add_task_to(space->task_executor, nvBroadPhase_SHG_task, &data[3], 3);

    nvCondition *events[4] = {
        ((nvTaskExecutorData *)(space->task_executor->data->data[0]))->done_event,
        ((nvTaskExecutorData *)(space->task_executor->data->data[1]))->done_event,
        ((nvTaskExecutorData *)(space->task_executor->data->data[2]))->done_event,
        ((nvTaskExecutorData *)(space->task_executor->data->data[3]))->done_event
    };

    nvCondition_wait(events[0]);
    nvCondition_wait(events[1]);
    nvCondition_wait(events[2]);
    nvCondition_wait(events[3]);

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

            if (res_exists) {
                nv_uint32 id_pair;
                if (a->id < b->id) id_pair = nv_pair(a->id, b->id);
                else id_pair = nv_pair(b->id, a->id);

                // Midphase
                if (nv_collide_aabb_x_aabb(abox, bbox)) {
                    nvBroadPhasePair *pair = NV_NEW(nvBroadPhasePair);
                    pair->a = a;
                    pair->b = b;
                    pair->id_pair = id_pair;

                    nvArray_add(space->broadphase_pairs0, pair);
                }
                else if (res_exists) {
                    nvResolution_update(space, a, b, res_value);
                }
            }
        }
    }

    NV_TRACY_ZONE_END;
}


void nvBroadPhase_BVH(nvSpace *space) {
    NV_TRACY_ZONE_START;

    nvHashMap_clear(space->broadphase_pairs);
    nvHashMap_clear(space->pairs);

    nvBVHNode *bvh_tree = nvBVHTree_new(space->bodies);

    for (size_t i = 0; i < space->bodies->size; i++) {
        nvBody *a = space->bodies->data[i];
        nvAABB aabb = nvBody_get_aabb(a);

        nvArray *collided = nvBVHNode_collide(bvh_tree, aabb);
        if (!collided) continue;

        for (size_t j = 0; j < collided->size; j++) {
            nvBody *b = collided->data[j];

            if (nvBroadPhase_early_out(space, a, b)) continue;

            // Generate ID pair
            nv_uint32 id_pair;
            if (a->id < b->id) id_pair = nv_pair(a->id, b->id);
            else id_pair = nv_pair(b->id, a->id);

            // Skip if the pair is already checked
            if (nvHashMap_get(space->pairs, &(nvBroadPhasePair){.id_pair=id_pair}) != NULL) continue;

            // Add pair to pairs map
            nvHashMap_set(space->pairs, &(nvBroadPhasePair){.a=a,.b=b,.id_pair=id_pair});

            nvHashMap_set(space->broadphase_pairs, &(nvBroadPhasePair){.a=a, .b=b, .id_pair=id_pair});
        }
    }

    nvBVHTree_free(bvh_tree);

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

            if (res_exists) {
                nv_uint32 id_pair;
                if (a->id < b->id) id_pair = nv_pair(a->id, b->id);
                else id_pair = nv_pair(b->id, a->id);

                nv_midphase(space, a, b, abox, bbox, res_exists, res_value, id_pair);
            }
        }
    }

    NV_TRACY_ZONE_END;
}