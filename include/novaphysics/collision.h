/*

  This file is a part of the Nova Physics Engine
  project and distributed under the MIT license.

  Copyright © Kadir Aksoy
  https://github.com/kadir014/nova-physics

*/

#ifndef NOVAPHYSICS_COLLISION_H
#define NOVAPHYSICS_COLLISION_H

#include <stdio.h>
#include "novaphysics/contact.h"


/**
 * @file collision.h
 * 
 * @brief Collision detection and contact point generation functions.
 */


// /**
//  * @brief Calculate the collision between two circles
//  * 
//  * @param a First circle body
//  * @param b Second circle body
//  * @return nvResolution
//  */
// nvResolution nv_collide_circle_x_circle(nvRigidBody *a, nvRigidBody *b);

// /**
//  * @brief Check if point is inside circle
//  * 
//  * @param circle Circle body
//  * @param point Point
//  * @return nv_bool
//  */
// nv_bool nv_collide_circle_x_point(nvRigidBody *circle, nvVector2 point);


// /**
//  * @brief Calculate the collision between polygon and circle
//  * 
//  * @param polygon Polygon body
//  * @param circle Circle body
//  * @return nvResolution 
//  */
// nvResolution nv_collide_polygon_x_circle(nvRigidBody *polygon, nvRigidBody *circle);

/**
 * @brief Check polygons collision and generate contact point information.
 * 
 * @param polygon_a First polygon shape
 * @param xform_a First transform
 * @param polygon_b Second polygon shape
 * @param xform_b Second transform
 * @return nvPersistentContactPair 
 */
nvPersistentContactPair nv_collide_polygon_x_polygon(
    nvShape *polygon_a,
    nvTransform xform_a,
    nvShape *polygon_b,
    nvTransform xform_b
);

/**
 * @brief Check if point is inside polygon.
 * 
 * @param polygon Polygon shape
 * @param point Point
 * @return nv_bool
 */
nv_bool nv_collide_polygon_x_point(nvShape *polygon, nvTransform xform, nvVector2 point);


/**
 * @brief Check if two AABBs collide.
 * 
 * @param a First AABB
 * @param b Second AABB
 * @return nv_bool
 */
nv_bool nv_collide_aabb_x_aabb(nvAABB a, nvAABB b);

/**
 * @brief Check if point is inside AABB.
 * 
 * @param aabb AABB
 * @param point Point
 * @return nv_bool
 */
nv_bool nv_collide_aabb_x_point(nvAABB aabb, nvVector2 point);


#endif