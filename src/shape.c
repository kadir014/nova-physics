/*

  This file is a part of the Nova Physics Engine
  project and distributed under the MIT license.

  Copyright © Kadir Aksoy
  https://github.com/kadir014/nova-physics

*/

#include "novaphysics/shape.h"
#include "novaphysics/vector.h"


nv_Shape *nv_CircleShape_new(nv_float radius) {
    nv_Shape *shape = NV_NEW(nv_Shape);

    shape->type = nv_ShapeType_CIRCLE;

    shape->radius = radius;

    return shape;
}

nv_Shape *nv_PolygonShape_new(nv_Array *vertices) {
    nv_Shape *shape = NV_NEW(nv_Shape);

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