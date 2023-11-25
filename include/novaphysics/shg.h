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
 * @brief Spatial Hash Grid implementation.
 */


typedef struct {
    nv_uint32 xy_pair;
    nv_Array *cell;
} nv_SHGEntry;


/**
 * @brief Spatial Hash Grid struct.
 * 
 * @param bounds Boundaries of the grid
 * @param cols Columns of the grid (cells on X axis)
 * @param rows Rows of the grid (cells on Y axis)
 * @param cell_width Width of one cell
 * @param cell_height Height of one cell
 */
typedef struct {
    nv_AABB bounds; /**< Boundaries of the grid. */
    nv_uint32 cols; /**< Columns of the grid (cells on X axis). */
    nv_uint32 rows; /**< Rows of the grid (cells on Y axis). */
    nv_float cell_width; /**< Width of one cell. */
    nv_float cell_height; /**< Height of one cell. */
    nv_HashMap *map; /**< Hashmap used internally to store cells. */
} nv_SHG;

/**
 * @brief Create a new Spatial Hash Grid.
 * 
 * @param cell_width Width of one cell
 * @param cell_width Height of one cell
 * @return nv_SHG * 
 */
nv_SHG *nv_SHG_new(
    nv_AABB bounds,
    nv_float cell_width,
    nv_float cell_height
) ;

/**
 * @brief Free the Spatial Hash Grid.
 * 
 * @param shg Spatial Hash Grid to free
 */
void nv_SHG_free(nv_SHG *shg);

/**
 * @brief Get content of one cell.
 * 
 * @param shg Spatial Hash Grid
 * @param key Cell key
 * @return nv_Array * 
 */
nv_Array *nv_SHG_get(nv_SHG *shg, nv_uint32 key);

/**
 * @brief Place bodies onto Spatial Hash Grid.
 * 
 * @param shg Spatial Hash Grid
 * @param bodies Body array
 */
void nv_SHG_place(nv_SHG *shg, nv_Array *bodies);

/**
 * @brief Get neighboring cell information.
 * 
 * @param shg Spatial Hash Grid
 * @param x0 Cell X
 * @param y0 Cell y
 * @param neighbors 32-bit int array to insert pair keys at
 * @param neighbor_flags Neighbor flags
 */
void nv_SHG_get_neighbors(
    nv_SHG *shg,
    nv_int16 x0,
    nv_int16 y0,
    nv_uint32 neighbors[],
    bool neighbor_flags[]
);


#endif