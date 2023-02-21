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
 * collision.h
 * 
 * Collision detection functions
 */


/**
 * @brief Calculate the collision between two circles
 * 
 * @param a First circle body
 * @param b Second circle body
 * @return nv_Resolution
 */
nv_Resolution nv_collide_circle_x_circle(nv_Body *a, nv_Body *b);

/**
 * @brief Check if point is inside circle
 * 
 * @param circle Circle body
 * @param point Point
 * @return bool
 */
bool nv_collide_circle_x_point(nv_Body *circle, nv_Vector2 point);


/**
 * @brief Calculate the collision between polygon and circle
 * 
 * @param polygon Polygon body
 * @param circle Circle body
 * @return nv_Resolution 
 */
nv_Resolution nv_collide_polygon_x_circle(nv_Body *polygon, nv_Body *circle);

/**
 * @brief Calculate the collision between two polygons
 * 
 * @param a First polygon body
 * @param b Second polygon body
 * @return nv_Resolution 
 */
nv_Resolution nv_collide_polygon_x_polygon(nv_Body *a, nv_Body *b);

/**
 * @brief Check if point is inside polygon
 * 
 * @param polygon Polygon body
 * @param point Point
 * @return bool
 */
bool nv_collide_polygon_x_point(nv_Body *polygon, nv_Vector2 point);


/**
 * @brief Check if two AABBs collide
 * 
 * @param a First AABB
 * @param b Second AABB
 * @return bool
 */
bool nv_collide_aabb_x_aabb(nv_AABB a, nv_AABB b);

/**
 * @brief Check if point is inside AABB
 * 
 * @param aabb AABB
 * @param point Point
 * @return bool
 */
bool nv_collide_aabb_x_point(nv_AABB aabb, nv_Vector2 point);


#endif