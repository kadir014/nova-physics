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


/**
 * @file shape.h
 * 
 * @brief Shape struct and methods.
 * 
 * This module defines ShapeType enum, Shape struct and its methods.
 */


/**
 * @brief Shape type enumerator.
 */
typedef enum {
    nvShapeType_CIRCLE, /**< Circle shape. It's the simplest collision shape. */
    nvShapeType_POLYGON /**< Convex polygon shape. It's more complex than
                              circle shape and the calculations gets more expensive 
                              as the vertex count goes higher. */
} nvShapeType;


/**
 * @brief Collision shape.
 */
typedef struct {
    nvShapeType type; /**< Type of the shape */
    
    union {
        nv_float radius; /**< Circle radius. */

        struct {
            nvArray *vertices; /**< Polygon local vertices. */
            nvArray *trans_vertices; /**< Polygon transformed vertices. */
            nvArray *normals; /**< Polygon edge normals. */
        };
        
    };
} nvShape;

/**
 * @brief Create a new circle shape.
 * 
 * @param radius Radius of the circle
 * @return nvShape *
 */
nvShape *nvCircleShape_new(nv_float radius);

/**
 * @brief Create a new convex polygon shape.
 * 
 * @param vertices Array of vertices
 * @return nvShape *
 */
nvShape *nvPolygonShape_new(nvArray *vertices);

/**
 * @brief Create a new polygon shape that is a rectangle.
 * 
 * @param width Width
 * @param height Height
 * @return nvShape *
 */
nvShape *nvRectShape_new(nv_float width, nv_float height);

/**
 * @brief Create a new polygon shape that is a rectangle. Alias for @ref nvRectShape_new
 * 
 * @param width Width
 * @param height Height
 * @return nvShape *
 */
#define nvBoxShape_new(width, height) (nvRectShape_new(width, height))

/**
 * @brief Create a new polygon shape that is a regular n-gon.
 * 
 * @param n Number of vertices or edges
 * @param radius Length of a vertex from the centroid
 * @return nvShape *
 */
nvShape *nvNGonShape_new(size_t n, nv_float radius);

/**
 * @brief Create a new polygon shape from a convex hull of an array of points.
 * 
 * @param points Points to generate a convex hull from
 * @return nvShape * 
 */
nvShape *nvConvexHullShape_new(nvArray *points);

/**
 * @brief Free shape.
 * 
 * @param shape Shape
 */
void nvShape_free(nvShape *shape);


#endif