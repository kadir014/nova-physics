/*

  This file is a part of the Nova Physics Engine
  project and distributed under the MIT license.

  Copyright © Kadir Aksoy
  https://github.com/kadir014/nova-physics

*/

#include "novaphysics/shape.h"
#include "novaphysics/vector.h"
#include "novaphysics/math.h"


/**
 * @file shape.c
 * 
 * @brief Shape struct and methods.
 * 
 * This module implements ShapeType enum, Shape struct and its methods.
 */


nv_Shape *nv_CircleShape_new(nv_float radius) {
    nv_Shape *shape = NV_NEW(nv_Shape);
    if (!shape) return NULL;

    shape->type = nv_ShapeType_CIRCLE;

    shape->radius = radius;

    return shape;
}

nv_Shape *nv_PolygonShape_new(nv_Array *vertices) {
    nv_Shape *shape = NV_NEW(nv_Shape);
    if (!shape) return NULL;

    shape->type = nv_ShapeType_POLYGON;

    shape->vertices = vertices;

    shape->trans_vertices = nv_Array_new();
    for (size_t i = 0; i < shape->vertices->size; i++)
        nv_Array_add(shape->trans_vertices, NV_VEC2_NEW(0.0, 0.0));

    shape->normals = nv_Array_new();
    for (size_t i = 0; i < shape->vertices->size; i++) {
        nv_Vector2 va = NV_TO_VEC2(shape->vertices->data[i]);
        nv_Vector2 vb = NV_TO_VEC2(shape->vertices->data[(i + 1) % shape->vertices->size]);
    
        nv_Vector2 face = nv_Vector2_sub(vb, va);
        nv_Vector2 normal = nv_Vector2_normalize(nv_Vector2_perpr(face));

        nv_Array_add(shape->normals, NV_VEC2_NEW(normal.x, normal.y));
    }

    return shape;
}

nv_Shape *nv_ShapeFactory_Rect(nv_float width, nv_float height) {
    nv_float w = width / 2.0;
    nv_float h = height / 2.0;

    nv_Array *vertices = nv_Array_new();    
    nv_Array_add(vertices, NV_VEC2_NEW(-w, -h));
    nv_Array_add(vertices, NV_VEC2_NEW( w, -h));
    nv_Array_add(vertices, NV_VEC2_NEW( w,  h));
    nv_Array_add(vertices, NV_VEC2_NEW(-w,  h));

    return nv_PolygonShape_new(vertices);
}

nv_Shape *nv_ShapeFactory_NGon(int n, nv_float radius) {
    NV_ASSERT(n >= 3, "Cannot create a polygon with vertices lesser than 3.\n");

    nv_Array *vertices = nv_Array_new();
    nv_Vector2 arm = NV_VEC2(radius / 2.0, 0.0);

    for (size_t i = 0; i < n; i++) {
        nv_Array_add(vertices, NV_VEC2_NEW(arm.x, arm.y));
        arm = nv_Vector2_rotate(arm, 2.0 * NV_PI / (nv_float)n);
    }

    return nv_PolygonShape_new(vertices);
}

nv_Shape *nv_ShapeFactory_ConvexHull(nv_Array *points) {
    nv_Array *vertices = nv_generate_convex_hull(points);

    // Transform hull vertices so the center of gravity is at center
    nv_Vector2 hull_centroid = nv_polygon_centroid(vertices);

    for (size_t i = 0; i < vertices->size; i++) {
        nv_Vector2 new_vert = nv_Vector2_sub(NV_TO_VEC2(vertices->data[i]), hull_centroid);
        nv_Vector2 *current_vert = NV_TO_VEC2P(vertices->data[i]);
        current_vert->x = new_vert.x;
        current_vert->y = new_vert.y;
    }

    return nv_PolygonShape_new(vertices);
}

void nv_Shape_free(nv_Shape *shape) {
    if (shape->type == nv_ShapeType_POLYGON) {
        nv_Array_free_each(shape->vertices, free);
        nv_Array_free(shape->vertices);
        nv_Array_free_each(shape->trans_vertices, free);
        nv_Array_free(shape->trans_vertices);
        nv_Array_free_each(shape->normals, free);
        nv_Array_free(shape->normals);
    }

    free(shape);
}