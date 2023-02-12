/*

  This file is a part of the Nova Physics Engine
  project and distributed under the MIT license.

  Copyright © Kadir Aksoy
  https://github.com/kadir014/nova-physics

*/

#ifndef NOVAPHYSICS_AABB_H
#define NOVAPHYSICS_AABB_H


/**
 * aabb.h
 * 
 * Axis-aligned bounding box
 */


/**
 * @brief Axis-aligned bounding box
 * 
 * @param min_x Minimum X
 * @param min_y Minimum Y
 * @param max_x Maximum X
 * @param max_y Maximum Y
 */
typedef struct {
    double min_x;
    double min_y;
    double max_x;
    double max_y;
} nv_AABB;


#endif