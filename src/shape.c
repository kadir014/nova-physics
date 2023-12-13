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


nvShape *nv_CircleShape_new(nv_float radius) {
    nvShape *shape = NV_NEW(nvShape);
    if (!shape) return NULL;

    shape->type = nvShapeType_CIRCLE;

    shape->radius = radius;

    return shape;
}

nvShape *nv_PolygonShape_new(nvArray *vertices) {
    nvShape *shape = NV_NEW(nvShape);
    if (!shape) return NULL;

    shape->type = nvShapeType_POLYGON;

    shape->vertices = vertices;

    shape->trans_vertices = nvArray_new();
    for (size_t i = 0; i < shape->vertices->size; i++)
        nvArray_add(shape->trans_vertices, NV_VEC2_NEW(0.0, 0.0));

    shape->normals = nvArray_new();
    for (size_t i = 0; i < shape->vertices->size; i++) {
        nvVector2 va = NV_TO_VEC2(shape->vertices->data[i]);
        nvVector2 vb = NV_TO_VEC2(shape->vertices->data[(i + 1) % shape->vertices->size]);
    
        nvVector2 face = nvVector2_sub(vb, va);
        nvVector2 normal = nvVector2_normalize(nvVector2_perpr(face));

        nvArray_add(shape->normals, NV_VEC2_NEW(normal.x, normal.y));
    }

    return shape;
}

nvShape *nvShapeFactory_Rect(nv_float width, nv_float height) {
    nv_float w = width / 2.0;
    nv_float h = height / 2.0;

    nvArray *vertices = nvArray_new();    
    nvArray_add(vertices, NV_VEC2_NEW(-w, -h));
    nvArray_add(vertices, NV_VEC2_NEW( w, -h));
    nvArray_add(vertices, NV_VEC2_NEW( w,  h));
    nvArray_add(vertices, NV_VEC2_NEW(-w,  h));

    return nv_PolygonShape_new(vertices);
}

nvShape *nvShapeFactory_NGon(size_t n, nv_float radius) {
    NV_ASSERT(n >= 3, "Cannot create a polygon with vertices lesser than 3.\n");

    nvArray *vertices = nvArray_new();
    nvVector2 arm = NV_VEC2(radius / 2.0, 0.0);

    for (size_t i = 0; i < n; i++) {
        nvArray_add(vertices, NV_VEC2_NEW(arm.x, arm.y));
        arm = nvVector2_rotate(arm, 2.0 * NV_PI / (nv_float)n);
    }

    return nv_PolygonShape_new(vertices);
}

nvShape *nvShapeFactory_ConvexHull(nvArray *points) {
    nvArray *vertices = nv_generate_convex_hull(points);

    // Transform hull vertices so the center of gravity is at center
    nvVector2 hull_centroid = nv_polygon_centroid(vertices);

    for (size_t i = 0; i < vertices->size; i++) {
        nvVector2 new_vert = nvVector2_sub(NV_TO_VEC2(vertices->data[i]), hull_centroid);
        nvVector2 *current_vert = NV_TO_VEC2P(vertices->data[i]);
        current_vert->x = new_vert.x;
        current_vert->y = new_vert.y;
    }

    return nv_PolygonShape_new(vertices);
}

void nvShape_free(nvShape *shape) {
    if (shape->type == nvShapeType_POLYGON) {
        nvArray_free_each(shape->vertices, free);
        nvArray_free(shape->vertices);
        nvArray_free_each(shape->trans_vertices, free);
        nvArray_free(shape->trans_vertices);
        nvArray_free_each(shape->normals, free);
        nvArray_free(shape->normals);
    }

    free(shape);
}