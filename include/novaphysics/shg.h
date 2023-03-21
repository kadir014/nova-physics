/*

  This file is a part of the Nova Physics Engine
  project and distributed under the MIT license.

  Copyright © Kadir Aksoy
  https://github.com/kadir014/nova-physics

*/

#ifndef NOVAPHYSICS_SPATIAL_HASH_GRID_H
#define NOVAPHYSICS_SPATIAL_HASH_GRID_H

#include <stdint.h>
#include "novaphysics/internal.h"
#include "novaphysics/aabb.h"
#include "novaphysics/hashmap.h"
#include "novaphysics/array.h"


/**
 * @file shg.h
 * 
 * @details Spatial Hash Grid implementation
 */


/**
 * @brief Spatial Hash Grid struct
 * 
 * @param bounds Boundaries of the grid
 * @param cols Columns of the grid (cells on X axis)
 * @param rows Rows of the grid (cells on Y axis)
 * @param cell_width Width of one cell
 * @param cell_height Height of one cell
 */
typedef struct {
    nv_AABB bounds;
    uint32_t cols;
    uint32_t rows;
    nv_float cell_width;
    nv_float cell_height;
    nv_HashMap *map;
} nv_SHG;

/**
 * @brief Create a new Spatial Hash Grid
 * 
 * @param cols Columns of the grid (cells on X axis)
 * @param rows Rows of the grid (cells on Y axis)
 * @return nv_SHG * 
 */
nv_SHG *nv_SHG_new(
    nv_AABB bounds,
    nv_float cell_width,
    nv_float cell_height
) ;

/**
 * @brief Free the Spatial Hash Grid
 * 
 * @param shg Spatial Hash Grid to free
 */
void nv_SHG_free(nv_SHG *shg);

/**
 * @brief Get content of one cell
 * 
 * @param shg Spatial Hash Grid
 * @param x Cell X
 * @param y Cell Y
 * @return nv_Array * 
 */
nv_Array *nv_SHG_get_cell(nv_SHG *shg, int16_t x, int16_t y);

/**
 * @brief Get content of one cell with key
 * 
 * @param shg Spatial Hash Grid
 * @param key Cell key
 * @return nv_Array * 
 */
nv_Array *nv_SHG_get(nv_SHG *shg, uint32_t key);

/**
 * @brief Place bodies onto Spatial Hash Grid 
 * 
 * @param shg Spatial Hash Grid
 * @param bodies Body array
 */
void nv_SHG_place(nv_SHG *shg, nv_Array *bodies);

/**
 * @brief Get neighbor cell keys
 * 
 * @param shg Spatial Hash Grid
 * @param x0 Cell X
 * @param y0 Cell y
 * @param neighbors 32-bit int array to insert keys at
 * @param neighbor_flags Neighbor flags
 */
void nv_SHG_get_neighbors(
    nv_SHG *shg,
    int16_t x0,
    int16_t y0,
    uint32_t neighbors[],
    bool neighbor_flags[]
);


#endif