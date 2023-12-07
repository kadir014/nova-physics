/*

  This file is a part of the Nova Physics Engine
  project and distributed under the MIT license.

  Copyright © Kadir Aksoy
  https://github.com/kadir014/nova-physics

*/

#ifndef NOVAPHYSICS_COLLISION_H
#define NOVAPHYSICS_COLLISION_H

#include <stdio.h>
#include "novaphysics/resolution.h"


/**
 * @file collision.h
 * 
 * @brief Collision detection functions.
 */


/**
 * @brief Calculate the collision between two circles
 * 
 * @param a First circle body
 * @param b Second circle body
 * @return nvResolution
 */
nvResolution nv_collide_circle_x_circle(nvBody *a, nvBody *b);

/**
 * @brief Check if point is inside circle
 * 
 * @param circle Circle body
 * @param point Point
 * @return bool
 */
bool nv_collide_circle_x_point(nvBody *circle, nvVector2 point);


/**
 * @brief Calculate the collision between polygon and circle
 * 
 * @param polygon Polygon body
 * @param circle Circle body
 * @return nvResolution 
 */
nvResolution nv_collide_polygon_x_circle(nvBody *polygon, nvBody *circle);

/**
 * @brief Calculate the collision between two polygons
 * 
 * @param a First polygon body
 * @param b Second polygon body
 * @return nvResolution 
 */
nvResolution nv_collide_polygon_x_polygon(nvBody *a, nvBody *b);

/**
 * @brief Check if point is inside polygon
 * 
 * @param polygon Polygon body
 * @param point Point
 * @return bool
 */
bool nv_collide_polygon_x_point(nvBody *polygon, nvVector2 point);


/**
 * @brief Check if two AABBs collide
 * 
 * @param a First AABB
 * @param b Second AABB
 * @return bool
 */
bool nv_collide_aabb_x_aabb(nvAABB a, nvAABB b);

/**
 * @brief Check if point is inside AABB
 * 
 * @param aabb AABB
 * @param point Point
 * @return bool
 */
bool nv_collide_aabb_x_point(nvAABB aabb, nvVector2 point);


#endif