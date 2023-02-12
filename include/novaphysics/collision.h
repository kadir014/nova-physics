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
 * @brief Calculate the collision between two circle bodies
 * 
 * @param a First circle body
 * @param b Second circle body
 * @return nv_Resolution
 */
nv_Resolution nv_collide_circle_x_circle(nv_Body *a, nv_Body *b);

/**
 * @brief Calculate the collision between polygon and circle bodies
 * 
 * @param polygon Polygon body
 * @param circle Circle body
 * @return nv_Resolution 
 */
nv_Resolution nv_collide_polygon_x_circle(nv_Body *polygon, nv_Body *circle);

/**
 * @brief Calculate the collision between two polygon bodies
 * 
 * @param a First polygon body
 * @param b Second polygon body
 * @return nv_Resolution 
 */
nv_Resolution nv_collide_polygon_x_polygon(nv_Body *a, nv_Body *b);


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