/*

  This file is a part of the Nova Physics Engine
  project and distributed under the MIT license.

  Copyright © Kadir Aksoy
  https://github.com/kadir014/nova-physics

*/

#include <math.h>
#include "novaphysics/internal.h"
#include "novaphysics/shg.h"
#include "novaphysics/body.h"
#include "novaphysics/debug.h"


/**
 * @file shg.c
 * 
 * @brief Spatial Hash Grid implementation.
 */


// Hashing function for SHG cells
static nv_uint64 nvSHG_hash(void *item) {
    nvSHGEntry *entry = (nvSHGEntry *)item;
    return (nv_uint64)nv_hash(entry->xy_pair);
}


nvSHG *nvSHG_new(
    nvAABB bounds,
    nv_float cell_width,
    nv_float cell_height
) {
    nvSHG *shg = NV_NEW(nvSHG);
    if (!shg) return NULL;

    shg->bounds = bounds;
    shg->cols = (nv_uint32)((bounds.max_x - bounds.min_x) / cell_width);
    shg->rows = (nv_uint32)((bounds.max_y - bounds.min_y) / cell_height);
    shg->cell_width = cell_width;
    shg->cell_height = cell_height;

    shg->map = nvHashMap_new(sizeof(nvSHGEntry), 0, nvSHG_hash);
    if (!shg->map) return NULL;

    return shg;
}

void nvSHG_free(nvSHG *shg) {
    if (!shg) return;
    
    size_t iter = 0;
    void *item;
    while (nvHashMap_iter(shg->map, &iter, &item)) {
        nvSHGEntry *entry = (nvSHGEntry *)item;
        if (entry->cell != NULL) free((entry)->cell);
    }
    nvHashMap_free(shg->map);
    free(shg);
}

nvArray *nvSHG_get(nvSHG *shg, nv_uint32 key) {
    nvSHGEntry *entry = (nvSHGEntry *)nvHashMap_get(shg->map, &(nvSHGEntry){.xy_pair=key});
    if (entry == NULL) return NULL;
    else return entry->cell;
}

void nvSHG_place(nvSHG *shg, nvArray *bodies) {
    NV_TRACY_ZONE_START;

    size_t iter = 0;
    void *item;

    // Free each array from previous frame
    while (nvHashMap_iter(shg->map, &iter, &item)) {
        nvSHGEntry *entry = (nvSHGEntry *)item;
        nvArray_free((entry)->cell);
    }

    nvHashMap_clear(shg->map);

    for (nv_uint32 i = 0; i < bodies->size; i++) {
        nvRigidBody *body = (nvRigidBody *)bodies->data[i];
        nvAABB aabb = nvRigidBody_get_aabb(body);

        /*
            Spread AABB to exceeding cells

            min
            [ ] ---- [ ]
             |        |
             |        |
            [ ] ---- [ ]
                     max
        */

        nv_int16 min_x = (nv_int16)(aabb.min_x / shg->cell_width);
        nv_int16 min_y = (nv_int16)(aabb.min_y / shg->cell_height);
        nv_int16 max_x = (nv_int16)(aabb.max_x / shg->cell_width);
        nv_int16 max_y = (nv_int16)(aabb.max_y / shg->cell_height);

        for (nv_int16 y = min_y; y < max_y + 1; y++) {
            for (nv_int16 x = min_x; x < max_x + 1; x++) {

                // Don't insert outside of the borders
                if (0 <= x && x < (signed)shg->cols && 0 <= y && y < (signed)shg->rows) {
                    nv_uint32 pair = nv_pair(x, y);

                    nvSHGEntry *entry = (nvSHGEntry *)nvHashMap_get(shg->map, &(nvSHGEntry){.xy_pair=pair});

                    // If grid doesn't exist, create it
                    if (entry == NULL) {
                        nvArray *new_cell = nvArray_new();
                        nvArray_add(new_cell, body);
                        nvHashMap_set(shg->map, &(nvSHGEntry){.xy_pair=pair, .cell=new_cell});
                    }

                    // If grid exists, add body to it
                    else {
                        nvArray_add(entry->cell, body);
                    }
                }

            }
        }
    }

    NV_TRACY_ZONE_END;
}

void nvSHG_get_neighbors(
    nvSHG *shg,
    nv_int16 x0,
    nv_int16 y0,
    nv_uint32 neighbors[],
    nv_bool neighbor_flags[]
) {
    /*
        One cell has 8 neighbor cells

        [ ][ ][ ]
        [ ][X][ ]
        [ ][ ][ ]

        Depending on the position, neighbors change
        to keep them inside the boundaries

        Corner case:          Border case:
        [X][ ]                [ ][ ]
        [ ][ ]                [X][ ]
                              [ ][ ]
    */

    NV_TRACY_ZONE_START;

    // Initialize flag array
    for (size_t j = 0; j < 8; j++)
        neighbor_flags[j] = false;

    size_t i = 0;

    // TODO: This might be optimized
    for (nv_int16 y1 = -1; y1 < 2; y1++) {
        for (nv_int16 x1 = -1; x1 < 2; x1++) {

            nv_int16 x = (signed)x0 + x1;
            nv_int16 y = (signed)y0 + y1;

            // Skip current cell
            if (x == x0 && y == y0) continue;

            // Skip cells outside the boundaries
            if (0 <= x && x < (signed)shg->cols && 0 <= y && y < (signed)shg->rows) {
                neighbors[i] = nv_pair(x, y);
                neighbor_flags[i] = true;
                i++;
            }
        }
    }

    NV_TRACY_ZONE_END;
}