/*

  This file is a part of the Nova Physics Engine
  project and distributed under the MIT license.

  Copyright © Kadir Aksoy
  https://github.com/kadir014/nova-physics

*/

#ifndef NOVAPHYSICS_SHAPE_H
#define NOVAPHYSICS_SHAPE_H

#include "novaphysics/internal.h"
#include "novaphysics/array.h"
#include "novaphysics/vector.h"
#include "novaphysics/aabb.h"


/**
 * @file shape.h
 * 
 * @brief Collision shape implementations.
 */


/**
 * @brief Shape type enumerator.
 */
typedef enum {
    nvShapeType_CIRCLE, /**< Circle is the simplest collision shape. */
    nvShapeType_POLYGON /**< Convex polygon shape. It's more complex than
                              circle shape and the calculations gets more expensive 
                              as the vertex count goes higher. */
} nvShapeType;


/**
 * @brief Circle shape.
 * 
 * Do not initialize manually. Use shape creation functions.
 */
typedef struct {
    nvVector2 center; /**< Center position in local (body) space. */
    nv_float radius; /**< Radius. */
} nvCircle;


/**
 * @brief Convex polygon shape.
 * 
 * Do not initialize manually. Use shape creation functions.
 */
typedef struct {
    nvArray *vertices; /**< Vertices in local (body) space. */
    nvArray *xvertices; /**< Vertices transformed into world space. */
    nvArray *normals; /**< Edge normals in local (body) space. */
} nvPolygon;


/**
 * @brief Collision shape.
 * 
 * Do not initialize manually. Use shape creation functions.
 */
typedef struct {
    nvShapeType type; /**< Type of the shape */
    
    union {
        nvCircle circle;
        nvPolygon polygon;
    };
} nvShape;

/**
 * @brief Create a new circle shape.
 * 
 * Returns NULL on error. Use @ref nv_get_error to get more information.
 * 
 * @param center Center position relative to body position
 * @param radius Radius
 * @return nvShape *
 */
nvShape *nvCircleShape_new(nvVector2 center, nv_float radius);

/**
 * @brief Create a new convex polygon shape.
 * 
 * Returns NULL on error. Use @ref nv_get_error to get more information.
 * 
 * @param vertices Array of vertices
 * @param offset Offset to centroid
 * @return nvShape *
 */
nvShape *nvPolygonShape_new(nvArray *vertices, nvVector2 offset);

/**
 * @brief Create a new polygon shape that is a rectangle.
 * 
 * Returns NULL on error. Use @ref nv_get_error to get more information.
 * 
 * @param width Width
 * @param height Height
 * @param offset Offset to centroid
 * @return nvShape *
 */
nvShape *nvRectShape_new(nv_float width, nv_float height, nvVector2 offset);

/**
 * @brief Create a new polygon shape that is a rectangle.
 * 
 * Returns NULL on error. Use @ref nv_get_error to get more information.
 * 
 * @param width Width
 * @param height Height
 * @param offset Offset to centroid
 * @return nvShape *
 */
#define nvBoxShape_new(width, height, offset) (nvRectShape_new(width, height, offset))

/**
 * @brief Create a new polygon shape that is a regular n-gon.
 * 
 * Returns NULL on error. Use @ref nv_get_error to get more information.
 * 
 * @param n Number of vertices or edges
 * @param radius Length of a vertex from the centroid
 * @param offset Offset to centroid
 * @return nvShape *
 */
nvShape *nvNGonShape_new(size_t n, nv_float radius, nvVector2 offset);

/**
 * @brief Create a new polygon shape from a convex hull of an array of points.
 * 
 * Returns NULL on error. Use @ref nv_get_error to get more information.
 * 
 * @param points Points to generate a convex hull from
 * @param offset Offset to centroid
 * @return nvShape * 
 */
nvShape *nvConvexHullShape_new(nvArray *points, nvVector2 offset);

/**
 * @brief Free shape.
 * 
 * It's safe to pass NULL to this function.
 * 
 * @param shape Shape
 */
void nvShape_free(nvShape *shape);

/**
 * @brief Get AABB of shape.
 * 
 * @param shape Shape
 * @return nvAABB 
 */
nvAABB nvShape_get_aabb(nvShape *shape);

void nvPolygon_transform(nvShape *shape);


#endif