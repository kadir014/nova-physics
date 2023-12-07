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


#endif