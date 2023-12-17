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
    // Same body or B's ID is higher
    // Latter is done in order to avoid checking same pairs twice
    if (a->id >= b->id)
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


void nvBroadPhase_brute_force(nvSpace *space) {
    nvHashMap_clear(space->broadphase_pairs);

    for (size_t i = 0; i < space->bodies->size; i++) {
        nvBody *a = (nvBody *)space->bodies->data[i];
        nvAABB abox = nvBody_get_aabb(a);

        for (size_t j = 0; j < space->bodies->size; j++) {
            nvBody *b = (nvBody *)space->bodies->data[j];

            if (nvBroadPhase_early_out(space, a, b)) continue;

            nv_uint32 id_pair = nv_pair(a->id, b->id);

            nvAABB bbox = nvBody_get_aabb(b);

            if (nv_collide_aabb_x_aabb(abox, bbox)) {
                nvHashMap_set(space->broadphase_pairs, &(nvBroadPhasePair){.a=a, .b=b, .id_pair=id_pair});
            }
        }
    }
}


void nvBroadPhase_SHG(nvSpace *space) {
    NV_TRACY_ZONE_START;
    
    nvHashMap_clear(space->broadphase_pairs);

    nvSHG_place(space->shg, space->bodies);

    for (size_t i = 0; i < space->bodies->size; i++) {
        nvBody *a = (nvBody *)space->bodies->data[i];
        nvAABB abox = nvBody_get_aabb(a);

        nv_int16 min_x = (nv_int16)(abox.min_x / space->shg->cell_width);
        nv_int16 min_y = (nv_int16)(abox.min_y / space->shg->cell_height);
        nv_int16 max_x = (nv_int16)(abox.max_x / space->shg->cell_width);
        nv_int16 max_y = (nv_int16)(abox.max_y / space->shg->cell_height);

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

                        if (nvBroadPhase_early_out(space, a, b)) continue;

                        nv_uint32 id_pair = nv_pair(a->id, b->id);

                        nvAABB bbox = nvBody_get_aabb(b);

                        if (nv_collide_aabb_x_aabb(abox, bbox)) {
                            nvHashMap_set(space->broadphase_pairs, &(nvBroadPhasePair){.a=a, .b=b, .id_pair=id_pair});
                        }
                    }
                }
            }
        }
    }

    NV_TRACY_ZONE_END;
}


typedef struct {
    struct nvSpace *space;
    nvArray *bodies;
    nv_uint8 task_id;
} SHGWorkerData;


static int nvBroadPhase_SHG_task(void *data) {
    NV_TRACY_ZONE_START;

    nvSpace *space = (((SHGWorkerData *)data))->space;
    nvArray *bodies = ((SHGWorkerData *)data)->bodies;
    nv_uint8 task_id = ((SHGWorkerData *)data)->task_id;

    for (size_t i = 0; i < bodies->size; i++) {
        nvBody *a = (nvBody *)bodies->data[i];
        nvAABB abox = nvBody_get_aabb(a);

        nv_int16 min_x = (nv_int16)(abox.min_x / space->shg->cell_width);
        nv_int16 min_y = (nv_int16)(abox.min_y / space->shg->cell_height);
        nv_int16 max_x = (nv_int16)(abox.max_x / space->shg->cell_width);
        nv_int16 max_y = (nv_int16)(abox.max_y / space->shg->cell_height);

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

                        if (nvBroadPhase_early_out(space, a, b)) continue;

                        nv_uint32 id_pair = nv_pair(a->id, b->id);

                        nvAABB bbox = nvBody_get_aabb(b);

                        if (nv_collide_aabb_x_aabb(abox, bbox)) {
                            nvBroadPhasePair pair = {
                                .a = a,
                                .b = b,
                                .id_pair = id_pair
                            };

                            nvHashMap_set(space->mt_shg_pairs->data[task_id], &pair);
                        }
                    }
                }
            }
        }
    }

    NV_TRACY_ZONE_END;
    return 0;
}


void nvBroadPhase_SHG_parallel(nvSpace *space) {
    NV_TRACY_ZONE_START;
    
    nvSHG_place(space->shg, space->bodies);

    for (size_t i = 0; i < space->thread_count; i++) {
        nvHashMap_clear(space->mt_shg_pairs->data[i]);
        nvArray_clear(space->mt_shg_bins->data[i], NULL);
    }

    // Add bodies to bins for individual threads
    nv_float q = space->shg->bounds.max_x / (nv_float)space->thread_count;
    for (size_t i = 0; i < space->bodies->size; i++) {
        nvBody *body = space->bodies->data[i];
        nvAABB aabb = nvBody_get_aabb(body);

        for (size_t j = 0; j < space->thread_count; j++) {
            if (j == 0) {
                if (
                    aabb.max_x >= space->shg->bounds.min_x &&
                    body->position.x <= q
                ) {
                    nvArray_add(space->mt_shg_bins->data[j], body);
                    break;
                }
            }

            else if (j == (space->thread_count - 1)) {
                if (
                    aabb.min_x <= space->shg->bounds.max_x &&
                    body->position.x > q * (nv_float)(space->thread_count - 1)
                ) {
                    nvArray_add(space->mt_shg_bins->data[j], body);
                    break;
                }
            }

            else {
                if (
                    body->position.x > q * (nv_float)(j) &&
                    body->position.x <= q * (nv_float)(j + 1)
                ) {
                    nvArray_add(space->mt_shg_bins->data[j], body);
                    break;
                }
            }
        }
    }

    SHGWorkerData data[space->thread_count];

    for (size_t i = 0; i < space->thread_count; i++) {
        data[i] = (SHGWorkerData){
            .space=space,
            .bodies=space->mt_shg_bins->data[i],
            .task_id=i
        };
    }

    // Add and run SHG tasks to task executor
    for (size_t i = 0; i < space->thread_count; i++)
        nvTaskExecutor_add_task_to(
            space->task_executor,
            nvBroadPhase_SHG_task,
            &data[i],
            i
        );

    // Wait for SHG tasks to finish their works
    for (size_t i = 0; i < space->thread_count; i++) {
        nvCondition *done_event = ((nvTaskExecutorData *)(space->task_executor->data->data[i]))->done_event;
        nvCondition_wait(done_event);
    }

    NV_TRACY_ZONE_END;
}


void nvBroadPhase_BVH(nvSpace *space) {
    NV_TRACY_ZONE_START;

    nvHashMap_clear(space->broadphase_pairs);

    nvPrecisionTimer timer;
    
    NV_PROFILER_START(timer);
    nvBVHNode *bvh_tree = nvBVHTree_new(space->bodies);
    NV_PROFILER_STOP(timer, space->profiler.bvh_build);
    
    NV_PROFILER_START(timer);
    for (size_t i = 0; i < space->bodies->size; i++) {
        nvBody *a = space->bodies->data[i];
        nvAABB aabb = a->_cached_aabb;

        bool is_combined;
        nvArray *collided = nvBVHNode_collide(bvh_tree, aabb, &is_combined);
        if (!collided) {
            if (is_combined) nvArray_free(collided);
            continue;
        }

        for (size_t j = 0; j < collided->size; j++) {
            nvBody *b = collided->data[j];

            if (nvBroadPhase_early_out(space, a, b)) continue;

            // nvBody *pair_a, *pair_b;
            // if (a->id < b->id) {
            //     pair_a = a;
            //     pair_b = b;
            // }
            // else {
            //     pair_a = b;
            //     pair_b = a;
            // }
            //nv_uint32 id_pair = nv_pair(pair_a->id, pair_b->id);

            // Skip if the pair is already checked
            //if (nvHashMap_get(space->broadphase_pairs, &(nvBroadPhasePair){.a=pair_a, .b=pair_b, .id_pair=id_pair}))
            //    continue;

            nvHashMap_set(space->broadphase_pairs, &(nvBroadPhasePair){.a=a, .b=b});
        }

        if (is_combined) nvArray_free(collided);
    }
    NV_PROFILER_STOP(timer, space->profiler.bvh_traverse);

    NV_PROFILER_START(timer);
    nvBVHTree_free(bvh_tree);
    NV_PROFILER_STOP(timer, space->profiler.bvh_destroy);

    NV_TRACY_ZONE_END;
}