/*

  This file is a part of the Nova Physics Engine
  project and distributed under the MIT license.

  Copyright © Kadir Aksoy
  https://github.com/kadir014/nova-physics

*/

#include "novaphysics/internal.h"
#include "novaphysics/shape.h"


/**
 * @file shape.c
 * 
 * @brief Collision shape implementations.
 */


/*
    Cheap solution, but it works..!
    No one would need over 4 billion shapes... right?
*/
static nv_uint32 id_counter;


nvShape *nvCircleShape_new(nvVector2 center, nv_float radius) {
    nvShape *shape = NV_NEW(nvShape);
    NV_MEM_CHECK(shape);

    shape->id = id_counter++;
    shape->xform_cache = false;
    shape->aabb_cache = false;

    shape->type = nvShapeType_CIRCLE;
    nvCircle *circle = &shape->circle;

    circle->center = center;
    circle->radius = radius;
    circle->xcenter = nvVector2_zero;

    return shape;
}

nvShape *nvPolygonShape_new(
    nvVector2 *vertices,
    size_t num_vertices,
    nvVector2 offset
) {
    if (num_vertices > NV_POLYGON_MAX_VERTICES) {
        nv_set_error("Exceeds maximum number of vertices per convex polygon shape.");
        return NULL;
    }

    if (num_vertices < 3) {
        nv_set_error("Cannot create a polygon shape with fewer than 3 vertices.");
        return NULL;
    }

    nvShape *shape = NV_NEW(nvShape);
    NV_MEM_CHECK(shape);

    shape->type = nvShapeType_POLYGON;
    nvPolygon *polygon = &shape->polygon;
    polygon->num_vertices = num_vertices;

    for (size_t i = 0; i < num_vertices; i++)
        polygon->vertices[i] = nvVector2_add(vertices[i], offset);

    for (size_t i = 0; i < num_vertices; i++)
        polygon->xvertices[i] = nvVector2_zero;

    for (size_t i = 0; i < num_vertices; i++) {
        nvVector2 va = polygon->vertices[i];
        nvVector2 vb = polygon->vertices[(i + 1) % num_vertices];
    
        nvVector2 edge = nvVector2_sub(vb, va);
        nvVector2 normal = nvVector2_normalize(nvVector2_perpr(edge));

        polygon->normals[i] = normal;
    }

    shape->id = id_counter++;
    shape->xform_cache = false;
    shape->aabb_cache = false;

    return shape;
}

nvShape *nvRectShape_new(nv_float width, nv_float height, nvVector2 offset) {
    nv_float w = width * 0.5f;
    nv_float h = height * 0.5f;

    nvVector2 vertices[4] = {
        NV_VECTOR2(-w, -h),
        NV_VECTOR2( w, -h),
        NV_VECTOR2( w, h),
        NV_VECTOR2(-w, h)
    };

    return nvPolygonShape_new(vertices, 4, offset);
}

nvShape *nvNGonShape_new(size_t n, nv_float radius, nvVector2 offset) {
    if (n < 3) {
        nv_set_error("Cannot create a polygon shape with fewer than 3 vertices.");
        return NULL;
    }
    if (n > NV_POLYGON_MAX_VERTICES) {
        nv_set_error("Too many polygon vertices (check NV_POLYGON_MAX_VERTICES).");
        return NULL;
    }

    nvVector2 vertices[NV_POLYGON_MAX_VERTICES];
    nvVector2 arm = NV_VECTOR2(radius, 0.0f);

    for (size_t i = 0; i < n; i++) {
        vertices[i] = arm;
        arm = nvVector2_rotate(arm, NV_TAU / (nv_float)n);
    }

    return nvPolygonShape_new(vertices, n, offset);
}

nvShape *nvConvexHullShape_new(
    nvVector2 *points,
    size_t num_points,
    nvVector2 offset,
    nv_bool center
){
    if (num_points < 3) {
        nv_set_error("Cannot create a polygon shape with fewer than 3 vertices.");
        return NULL;
    }

    nvVector2 vertices[NV_POLYGON_MAX_VERTICES];
    size_t num_vertices = nv_quickhull(points, num_points, vertices);

    // Center the hull, then add the offset later
    nvVector2 hull_centroid = nv_polygon_centroid(vertices, num_vertices);
    for (size_t i = 0; i < num_vertices; i++) {
        vertices[i] = nvVector2_sub(vertices[i], hull_centroid);
    }

    nvVector2 initial_offset = nvVector2_zero;
    if (!center) {
        initial_offset = hull_centroid;
    }

    return nvPolygonShape_new(
        vertices, num_vertices, nvVector2_add(offset, initial_offset)
    );
}

void nvShape_free(nvShape *shape) {
    if (!shape) return;

    NV_FREE(shape);
}

nvShapeMassInfo nvShape_calculate_mass(nvShape *shape, nv_float density) {
    nv_float mass, inertia;

    switch (shape->type) {
        case nvShapeType_CIRCLE: {
            nvCircle circle = shape->circle;

            mass = nv_circle_area(circle.radius) * density;
            inertia = nv_circle_inertia(mass, circle.radius, circle.center);

            return (nvShapeMassInfo){mass, inertia, circle.center};
        }
        case nvShapeType_POLYGON: {
            nvPolygon polygon = shape->polygon;

            mass = nv_polygon_area(polygon.vertices, polygon.num_vertices) * density;
            inertia = nv_polygon_inertia(mass, polygon.vertices, polygon.num_vertices);
            nvVector2 centroid = nv_polygon_centroid(polygon.vertices, polygon.num_vertices);

            return (nvShapeMassInfo){mass, inertia, centroid};
        }
        default:
            nv_set_error("Invalid shape.");
            return (nvShapeMassInfo){-1.0f, -1.0f, NV_VECTOR2(-1.0f, -1.0f)};
    }
}

void nvPolygon_transform(nvShape *shape, nvTransform xform) {
    NV_TRACY_ZONE_START;

    if (shape->xform_cache) {
        NV_TRACY_ZONE_END;
        return;
    }

    shape->xform_cache = true;

    for (size_t i = 0; i < shape->polygon.num_vertices; i++) {
        nvVector2 new = nvVector2_add(xform.position,
            nvVector2_rotate(
                shape->polygon.vertices[i],
                xform.angle
                )
            );

        shape->polygon.xvertices[i] = new;
    }

    NV_TRACY_ZONE_END;
}

size_t nvShape_total_memory_used(nvShape *shape) {
    size_t size = 0;
    if (!shape) return size;

    // nvShape
    size += sizeof(nvShape);

    return size;
}