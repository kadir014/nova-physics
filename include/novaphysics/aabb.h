/*

  This file is a part of the Nova Physics Engine
  project and distributed under the MIT license.

  Copyright © Kadir Aksoy
  https://github.com/kadir014/nova-physics

*/

#ifndef NOVAPHYSICS_AABB_H
#define NOVAPHYSICS_AABB_H

#include "novaphysics/internal.h"


/**
 * @file aabb.h
 * 
 * @brief AABB struct.
 */


/**
 * @brief Axis-aligned bounding box.
 */
typedef struct {
    nv_float min_x; /**< Minimum X */
    nv_float min_y; /**< Minimum Y */
    nv_float max_x; /**< Maximum X */
    nv_float max_y; /**< Maximum Y */
} nvAABB;

/**
 * @brief Merge two AABBs.
 * 
 * @param a First AABB
 * @param b Second AABB
 * @return nvAABB 
 */
static inline nvAABB nvAABB_merge(nvAABB a, nvAABB b) {
    return (nvAABB){
        nv_fmin(a.min_x, b.min_x),
        nv_fmin(a.min_y, b.min_y),
        nv_fmax(a.max_x, b.max_x),
        nv_fmax(a.max_y, b.max_y)
    };
}

/**
 * @brief Inflate an AABB in all directions.
 * 
 * @param aabb AABB
 * @param amount Amount to inflate
 * @return nvAABB 
 */
static inline nvAABB nvAABB_inflate(nvAABB aabb, nv_float amount) {
    return (nvAABB){
        aabb.min_x - amount,
        aabb.min_y - amount,
        aabb.max_x + amount,
        aabb.max_y + amount
    };
}


#endif