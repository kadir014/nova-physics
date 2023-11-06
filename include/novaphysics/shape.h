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
    nv_ShapeType_CIRCLE, /**< Circle shape. It's the simplest collision shape. */
    nv_ShapeType_POLYGON /**< Convex polygon shape. It's more complex than
                              circle shape and the calculations gets more expensive 
                              as the vertex count goes higher. */
} nv_ShapeType;


/**
 * @brief Collision shape.
 */
typedef struct {
    nv_ShapeType type; /**< Type of the shape */
    
    union {
        nv_float radius; /**< Circle radius. */

        struct {
            nv_Array *vertices; /**< Polygon local vertices. */
            nv_Array *trans_vertices; /**< Polygon transformed vertices. */
            nv_Array *normals; /**< Polygon face normals. */
        };
        
    };
} nv_Shape;

/**
 * @brief Create a new circle shape.
 * 
 * @param radius Radius of the circle
 * @return nv_Shape *
 */
nv_Shape *nv_CircleShape_new(nv_float radius);

/**
 * @brief Create a new convex polygon shape.
 * 
 * @param vertices Array of vertices
 * @return nv_Shape *
 */
nv_Shape *nv_PolygonShape_new(nv_Array *vertices);

/**
 * @brief Create a new polygon shape that is a rectangle.
 * 
 * @param width Width
 * @param height Height
 * @return nv_Shape *
 */
nv_Shape *nv_ShapeFactory_Rect(nv_float width, nv_float height);

/**
 * @brief Create a new polygon shape that is a rectangle. Alias for @ref nv_ShapeFactory_Rect
 * 
 * @param width Width
 * @param height Height
 * @return nv_Shape *
 */
#define nv_ShapeFactory_Box(width, height) (nv_ShapeFactory_Rect(width, height))

/**
 * @brief Create a new polygon shape that is a regular n-gon.
 * 
 * @param n Number of vertices or edges
 * @param radius Length of a vertex from the centroid
 * @return nv_Shape *
 */
nv_Shape *nv_ShapeFactory_NGon(int n, nv_float radius);

/**
 * @brief Create a new polygon shape from a convex hull of an array of points.
 * 
 * @param points Points to generate a convex hull from
 * @return nv_Shape * 
 */
nv_Shape *nv_ShapeFactory_ConvexHull(nv_Array *points);

/**
 * @brief Free shape.
 * 
 * @param shape Shape
 */
void nv_Shape_free(nv_Shape *shape);


#endif