/*

  This file is a part of the Nova Physics Engine
  project and distributed under the MIT license.

  Copyright © Kadir Aksoy
  https://github.com/kadir014/nova-physics

*/

#ifndef NOVAPHYSICS_COLLISION_H
#define NOVAPHYSICS_COLLISION_H

#include "novaphysics/contact.h"


/**
 * @file collision.h
 * 
 * @brief Collision detection and contact point generation functions.
 */


// Vector used when the normal can't be calculated (usually when shapes overlap perfectly)
#define NV_DEGENERATE_NORMAL NV_VECTOR2(0.0, 1.0)


/**
 * @brief Check circles collision and generate contact point information.
 * 
 * @param circle_a First circle shape
 * @param xform_a First transform
 * @param circle_b Second circle shape
 * @param xform_b Second transform
 * @return nvPersistentContactPair
 */
nvPersistentContactPair nv_collide_circle_x_circle(
    nvShape *circle_a,
    nvTransform xform_a,
    nvShape *circle_b,
    nvTransform xform_b
);

/**
 * @brief Check if point is inside circle.
 * 
 * @param circle Circle shape
 * @param xform Transform for the shape
 * @param point Point
 * @return nv_bool
 */
nv_bool nv_collide_circle_x_point(
    nvShape *circle,
    nvTransform xform,
    nvVector2 point
);

/**
 * @brief Check polygon x circle collision and generate contact point information.
 * 
 * @param polygon Polygon shape
 * @param xform_poly Transform for the polygon shape
 * @param circle Circle shape
 * @param xform_circle Transform for the circle shape
 * @param flip_anchors Whether to flip anchors of contact or not
 * @return nvPersistentContactPair 
 */
nvPersistentContactPair nv_collide_polygon_x_circle(
    nvShape *polygon,
    nvTransform xform_poly,
    nvShape *circle,
    nvTransform xform_circle,
    nv_bool flip_anchors
);

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
 * @param xform Transform for the shape
 * @param point Point
 * @return nv_bool
 */
nv_bool nv_collide_polygon_x_point(
    nvShape *polygon,
    nvTransform xform,
    nvVector2 point
);

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