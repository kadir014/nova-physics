/*

  This file is a part of the Nova Physics Engine
  project and distributed under the MIT license.

  Copyright © Kadir Aksoy
  https://github.com/kadir014/nova-physics

*/

#ifndef NOVAPHYSICS_SHAPE_H
#define NOVAPHYSICS_SHAPE_H

#include "novaphysics/internal.h"
#include "novaphysics/core/refarray.h"
#include "novaphysics/vector.h"
#include "novaphysics/math.h"
#include "novaphysics/aabb.h"


/**
 * @file shape.h
 * 
 * @brief Collision shape implementations.
 */


/**
 * @brief Collision filter information.
 */
typedef struct {
    nv_bool enabled; /**< Whether collision detection is enabled for this filter. */
    nv_uint32 group; /**< Filters that share the same non-zero group do not collide. */
    nv_uint32 category; /**< Bitmask of the categories this filter belongs to. */
    nv_uint32 mask; /**< Bitmask for which categories this filter collides with. */
} nvCollisionFilter;

static const nvCollisionFilter nvCollisionFilter_default = {
    true,
    0,
    0b11111111111111111111111111111111,
    0b11111111111111111111111111111111
};

/**
 * @brief Evaluate two collision filters and check if they collide.
 * 
 * @param a First collision filter.
 * @param b Second collision filter.
 * @return nv_bool 
 */
static inline nv_bool nvCollisionFilter_eval(
    nvCollisionFilter a,
    nvCollisionFilter b
) {
    // One of the filters is not enabled.
    if (!a.enabled || !b.enabled) {
        return false;
    }

    // Filters share the same non-zero group.
    if (
        a.group == b.group &&
        a.group != 0
    ) {
        return false;
    }

    // One of the collision mask doesn't fit the category.
    if (
        (a.mask & b.category) == 0 ||
        (b.mask & a.category) == 0
    ) {
        return false;
    }

    return true;
}


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
 * @brief Mass related information about shape.
 */
typedef struct {
    nv_float mass;
    nv_float inertia;
    nvVector2 center;
} nvShapeMassInfo;


/**
 * @brief Circle shape.
 * 
 * Do not initialize manually. Use shape creation functions.
 */
typedef struct {
    nvVector2 center; /**< Center position in local (body) space. */
    nv_float radius; /**< Radius. */
    nvVector2 xcenter;
} nvCircle;


/**
 * @brief Convex polygon shape.
 * 
 * Do not initialize manually. Use shape creation functions.
 */
typedef struct {
    nvVector2 vertices[NV_POLYGON_MAX_VERTICES]; /**< Vertices in local (body) space. */
    nvVector2 xvertices[NV_POLYGON_MAX_VERTICES]; /**< Vertices transformed into world space. */
    nvVector2 normals[NV_POLYGON_MAX_VERTICES]; /**< Edge normals in local (body) space. */
    size_t num_vertices; /**< Number of vertices. */
} nvPolygon;


/**
 * @brief Collision shape.
 * 
 * Do not initialize manually. Use shape creation functions.
 */
typedef struct {
    nvShapeType type; /**< Type of the shape */
    nv_uint32 id;
    nv_bool xform_cache;
    nv_bool aabb_cache;
    nvAABB cached_aabb;
    
    union {
        nvCircle circle;
        nvPolygon polygon;
    };
} nvShape;

/**
 * @brief Create a new circle shape.
 * 
 * Returns `NULL` on error. Use @ref nv_get_error to get more information.
 * 
 * @param center Center position relative to body position
 * @param radius Radius
 * @return nvShape *
 */
nvShape *nvCircleShape_new(nvVector2 center, nv_float radius);

/**
 * @brief Create a new convex polygon shape.
 * 
 * Returns `NULL` on error. Use @ref nv_get_error to get more information.
 * 
 * @param vertices Array of vertices
 * @param offset Offset to centroid
 * @return nvShape *
 */
nvShape *nvPolygonShape_new(
    nvVector2 *vertices,
    size_t num_vertices,
    nvVector2 offset
);

/**
 * @brief Create a new polygon shape that is a rectangle.
 * 
 * Returns `NULL` on error. Use @ref nv_get_error to get more information.
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
 * Returns `NULL` on error. Use @ref nv_get_error to get more information.
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
 * Returns `NULL` on error. Use @ref nv_get_error to get more information.
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
 * Returns `NULL` on error. Use @ref nv_get_error to get more information.
 * 
 * @param points Points to generate a convex hull from
 * @param num_points Number of points
 * @param offset Offset to centroid
 * @param bool Transform hull so the centroid is at origin?
 * @return nvShape * 
 */
nvShape *nvConvexHullShape_new(
    nvVector2 *points,
    size_t num_points,
    nvVector2 offset,
    nv_bool center
);

/**
 * @brief Free shape.
 * 
 * It's safe to pass `NULL` to this function.
 * 
 * @param shape Shape
 */
void nvShape_free(nvShape *shape);

/**
 * @brief Transform the polygon shape vertices.
 * 
 * @param shape Shape
 * @param xform Transform
 */
void nvPolygon_transform(nvShape *shape, nvTransform xform);

/**
 * @brief Get AABB of shape.
 * 
 * @param shape Shape
 * @param xform Shape transform
 * @return nvAABB 
 */
static inline nvAABB nvShape_get_aabb(nvShape *shape, nvTransform xform) {
    NV_TRACY_ZONE_START;

    if (shape->aabb_cache) {
        NV_TRACY_ZONE_END;
        return shape->cached_aabb;
    }

    nv_float min_x;
    nv_float min_y;
    nv_float max_x;
    nv_float max_y;
 
    nvAABB aabb;

    switch (shape->type) {
        case nvShapeType_CIRCLE: {
            
            if (!shape->xform_cache) {
                shape->circle.xcenter = nvVector2_add(nvVector2_rotate(shape->circle.center, xform.angle), xform.position);
                shape->xform_cache = true;
            }
            nvVector2 c = shape->circle.xcenter;

            nv_float r = shape->circle.radius;
            aabb = (nvAABB){
                c.x - r,
                c.y - r,
                c.x + r,
                c.y + r
            };

            shape->aabb_cache = true;
            shape->cached_aabb = aabb;

            NV_TRACY_ZONE_END;
            return aabb;
        }
        case nvShapeType_POLYGON: {
            min_x = NV_INF;
            min_y = NV_INF;
            max_x = -NV_INF;
            max_y = -NV_INF;

            nvPolygon_transform(shape, xform);

            for (size_t i = 0; i < shape->polygon.num_vertices; i++) {
                nvVector2 v = shape->polygon.xvertices[i];
                if (v.x < min_x) min_x = v.x;
                if (v.x > max_x) max_x = v.x;
                if (v.y < min_y) min_y = v.y;
                if (v.y > max_y) max_y = v.y;
            }

            aabb = (nvAABB){min_x, min_y, max_x, max_y};

            shape->aabb_cache = true;
            shape->cached_aabb = aabb;

            NV_TRACY_ZONE_END;
            return aabb;
        }
        default:
            NV_TRACY_ZONE_END;
            return (nvAABB){0.0f, 0.0f, 0.0f, 0.0f};
    }

    NV_TRACY_ZONE_END;
}

/**
 * @brief Calculate mass information of shape.
 * 
 * Returns a struct filled with -1 on error. Use @ref nv_get_error to get more information.
 * 
 * @param shape Shape
 * @return nvShapeMassInfo 
 */
nvShapeMassInfo nvShape_calculate_mass(nvShape *shape, nv_float density);

/**
 * @brief Get the total amount of memory used by this shape.
 * 
 * @param shape Shape.
 * @return Number of bytes allocated.
 */
size_t nvShape_total_memory_used(nvShape *shape);


#endif