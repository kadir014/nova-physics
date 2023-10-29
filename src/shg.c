/*

  This file is a part of the Nova Physics Engine
  project and distributed under the MIT license.

  Copyright © Kadir Aksoy
  https://github.com/kadir014/nova-physics

*/

#include <math.h>
#include "novaphysics/shg.h"
#include "novaphysics/body.h"
#include "novaphysics/debug.h"


/**
 * @file shg.c
 * 
 * @brief Spatial Hash Grid implementation.
 */


// Helper hashing function for SHG map
static uint64_t shghash(void *item) {
    nv_SHGEntry *entry = (nv_SHGEntry *)item;
    return (uint64_t)nv_hash(entry->id_pair);
}


nv_SHG *nv_SHG_new(
    nv_AABB bounds,
    nv_float cell_width,
    nv_float cell_height
) {
    nv_SHG *shg = NV_NEW(nv_SHG);
    if (!shg) return NULL;

    shg->bounds = bounds;
    shg->cols = (nv_uint32)((bounds.max_x - bounds.min_x) / cell_width);
    shg->rows = (nv_uint32)((bounds.max_y - bounds.min_y) / cell_height);
    shg->cell_width = cell_width;
    shg->cell_height = cell_height;

    shg->map = nv_HashMap_new(sizeof(nv_SHGEntry), 0, shghash);

    return shg;
}

void nv_SHG_free(nv_SHG *shg) {
    size_t iter = 0;
    void *item;
    while (nv_HashMap_iter(shg->map, &iter, &item)) {
        nv_SHGEntry *entry = (nv_SHGEntry *)item;
        if (entry->cell != NULL) free((entry)->cell);
    }
    nv_HashMap_free(shg->map);
    free(shg);
}

nv_Array *nv_SHG_get(nv_SHG *shg, nv_uint32 key) {
    nv_SHGEntry *entry = (nv_SHGEntry *)nv_HashMap_get(shg->map, &(nv_SHGEntry){.id_pair=key});
    if (entry == NULL) return NULL;
    else return entry->cell;
}

void nv_SHG_place(nv_SHG *shg, nv_Array *bodies) {
    size_t iter = 0;
    void *item;

    // Free each array from previous frame
    while (nv_HashMap_iter(shg->map, &iter, &item)) {
        nv_SHGEntry *entry = (nv_SHGEntry *)item;
        nv_Array_free((entry)->cell);
    }

    nv_HashMap_clear(shg->map, false);

    for (nv_uint32 i = 0; i < bodies->size; i++) {
        nv_Body *body = (nv_Body *)bodies->data[i];
        nv_AABB aabb = nv_Body_get_aabb(body);

        /*
            Spread AABB to exceeding cells

            min
            [ ] ---- [ ]
             |        |
             |        |
            [ ] ---- [ ]
                     max
        */

        nv_float min_x = (nv_int16)(aabb.min_x / shg->cell_width);
        nv_float min_y = (nv_int16)(aabb.min_y / shg->cell_height);
        nv_float max_x = (nv_int16)(aabb.max_x / shg->cell_width);
        nv_float max_y = (nv_int16)(aabb.max_y / shg->cell_height);

        for (nv_int16 y = min_y; y < max_y + 1; y++) {
            for (nv_int16 x = min_x; x < max_x + 1; x++) {

                // Don't insert outside of the borders
                if (0 <= x && x < shg->cols && 0 <= y && y < shg->rows) {
                    nv_uint32 pair = nv_pair(x, y);

                    nv_SHGEntry *entry = (nv_SHGEntry *)nv_HashMap_get(shg->map, &(nv_SHGEntry){.id_pair=pair});

                    // If grid doesn't exist, create it
                    if (entry == NULL) {
                        nv_Array *new_cell = nv_Array_new();
                        nv_Array_add(new_cell, body);
                        nv_HashMap_set(shg->map, &(nv_SHGEntry){.id_pair=pair, .cell=new_cell});
                    }

                    // If grid exists, add body to it
                    else {
                        nv_Array_add(entry->cell, body);
                    }
                }

            }
        }
    }
}

void nv_SHG_get_neighbors(
    nv_SHG *shg,
    nv_int16 x0,
    nv_int16 y0,
    nv_uint32 neighbors[],
    bool neighbor_flags[]
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
            if (0 <= x && x < shg->cols && 0 <= y && y < shg->rows) {
                neighbors[i] = nv_pair(x, y);
                neighbor_flags[i] = true;
                i++;
            }
        }
    }
}