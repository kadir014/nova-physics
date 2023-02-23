/*

  This file is a part of the Nova Physics Engine
  project and distributed under the MIT license.

  Copyright © Kadir Aksoy
  https://github.com/kadir014/nova-physics

*/

#include <stdbool.h>
#include <math.h>
#include "novaphysics/collision.h"
#include "novaphysics/array.h"
#include "novaphysics/math.h"
#include "novaphysics/constants.h"
#include "novaphysics/aabb.h"


/**
 * collision.c
 * 
 * Collision detection functions
 */


nv_Resolution nv_collide_circle_x_circle(nv_Body *a, nv_Body *b) {
    nv_Resolution res = {
        .collision = false,
        .a = a,
        .b = b,
        .normal = nv_Vector2_zero,
        .depth = 0.0
        };

    double dist = nv_Vector2_dist(b->position, a->position);
    double radii = a->radius + b->radius;

    // Circles aren't colliding
    if (dist >= radii) return res;

    res.collision = true;

    nv_Vector2 normal = nv_Vector2_sub(b->position, a->position);
    // If the bodies are in the exact same position, direct the normal upwards
    if (nv_Vector2_len2(normal) == 0.0) normal = (nv_Vector2){0.0, 1.0};
    else normal = nv_Vector2_normalize(normal);

    res.normal = normal;
    res.depth = radii - dist;

    return res;
}

bool nv_collide_circle_x_point(nv_Body *circle, nv_Vector2 point) {
    return nv_Vector2_len2(
        nv_Vector2_sub(circle->position, point)) <= circle->radius * circle->radius;
}


nv_Resolution nv_collide_polygon_x_circle(nv_Body *polygon, nv_Body *circle) {
    nv_Resolution res = {
        .collision = false,
        .a = polygon,
        .b = circle,
        .normal = nv_Vector2_zero,
        .depth = NV_INF
        };

    nv_Polygon_model_to_world(polygon);
    nv_Array *vertices = polygon->trans_vertices;

    size_t n = vertices->size;

    double min_a, min_b, max_a, max_b;

    for (size_t i = 0; i < n; i++) {
        nv_Vector2 va = NV_TO_VEC2(vertices->data[i]);
        nv_Vector2 vb = NV_TO_VEC2(vertices->data[(i + 1) % n]);

        nv_Vector2 edge = nv_Vector2_sub(vb, va);
        nv_Vector2 axis = nv_Vector2_normalize(nv_Vector2_perp(edge));

        nv_project_polyon(vertices, axis, &min_a, &max_a);
        nv_project_circle(circle->position, circle->radius, axis, &min_b, &max_b);

        // Doesn't collide
        if (min_a >= max_b || min_b >= max_a) {
            return res;
        }

        double axis_depth = fmin(max_b - min_a, max_a - min_b);

        if (axis_depth < res.depth) {
            res.depth = axis_depth;
            res.normal = axis;
        }
    }

    nv_Vector2 cp = nv_polygon_closest_vertex_to_circle(circle->position, vertices);

    nv_Vector2 axis = nv_Vector2_normalize(nv_Vector2_sub(cp, circle->position));

    nv_project_polyon(vertices, axis, &min_a, &max_a);
    nv_project_circle(circle->position, circle->radius, axis, &min_b, &max_b);

    // Doesn't collide
    if (min_a >= max_b || min_b >= max_a) {
        return res;
    }

    double axis_depth = fmin(max_b - min_a, max_a - min_b);

    if (axis_depth < res.depth) {
        res.depth = axis_depth;
        res.normal = axis;
    }

    nv_Vector2 direction = nv_Vector2_sub(polygon->position, circle->position);

    if (nv_Vector2_dot(direction, res.normal) > 0.0)
        res.normal = nv_Vector2_neg(res.normal);

    res.collision = true;

    return res;
}

nv_Resolution nv_collide_polygon_x_polygon(nv_Body *a, nv_Body *b) {
    nv_Resolution res = {
        .collision = false,
        .a = a,
        .b = b,
        .normal = nv_Vector2_zero,
        .depth = NV_INF
        };

    nv_Polygon_model_to_world(a);
    nv_Polygon_model_to_world(b);
    nv_Array *vertices_a = a->trans_vertices;
    nv_Array *vertices_b = b->trans_vertices;
    size_t na = vertices_a->size;
    size_t nb = vertices_b->size;

    size_t i;

    double min_a, max_a, min_b, max_b;

    for (i = 0; i < na; i++) {
        nv_Vector2 va = NV_TO_VEC2(vertices_a->data[i]);
        nv_Vector2 vb = NV_TO_VEC2(vertices_a->data[(i + 1) % na]);

        nv_Vector2 edge = nv_Vector2_sub(vb, va);
        nv_Vector2 axis = nv_Vector2_normalize(nv_Vector2_perpr(edge));

        nv_project_polyon(vertices_a, axis, &min_a, &max_a);
        nv_project_polyon(vertices_b, axis, &min_b, &max_b);

        // Doesn't collide
        if (min_a >= max_b || min_b >= max_a) {
            return res;
        }

        double axis_depth = fmin(max_b - min_a, max_a - min_b);

        if (axis_depth < res.depth) {
            res.depth = axis_depth;
            res.normal = axis;
        }
    }

    for (i = 0; i < nb; i++) {
        nv_Vector2 va = NV_TO_VEC2(vertices_b->data[i]);
        nv_Vector2 vb = NV_TO_VEC2(vertices_b->data[(i + 1) % na]);

        nv_Vector2 edge = nv_Vector2_sub(vb, va);
        nv_Vector2 axis = nv_Vector2_normalize(nv_Vector2_perpr(edge));

        nv_project_polyon(vertices_a, axis, &min_a, &max_a);
        nv_project_polyon(vertices_b, axis, &min_b, &max_b);

        // Doesn't collide
        if (min_a >= max_b || min_b >= max_a) {
            return res;
        }

        double axis_depth = fmin(max_b - min_a, max_a - min_b);

        if (axis_depth < res.depth) {
            res.depth = axis_depth;
            res.normal = axis;
        }
    }

    nv_Vector2 center_a = nv_Vector2_add(nv_polygon_centroid(a->vertices), a->position);
    nv_Vector2 center_b = nv_Vector2_add(nv_polygon_centroid(b->vertices), b->position);

    if (nv_Vector2_dot(nv_Vector2_sub(center_b, center_a), res.normal) < 0.0)
        res.normal = nv_Vector2_neg(res.normal);

    res.collision = true;

    return res;
}

bool nv_collide_polygon_x_point(nv_Body *polygon, nv_Vector2 point) {
    // https://stackoverflow.com/a/48760556
    nv_Polygon_model_to_world(polygon);
    nv_Array *vertices = polygon->trans_vertices;

    size_t n = vertices->size;
    size_t i = 0;
    size_t j = n - 1;
    bool inside = false;

    while (i < n) {
        nv_Vector2 vi = NV_TO_VEC2(vertices->data[i]);
        nv_Vector2 vj = NV_TO_VEC2(vertices->data[j]);

        nv_Vector2 diri = nv_Vector2_normalize(nv_Vector2_sub(vi, polygon->position));
        nv_Vector2 dirj = nv_Vector2_normalize(nv_Vector2_sub(vj, polygon->position));

        vi = nv_Vector2_add(vi, nv_Vector2_muls(diri, 0.1));
        vj = nv_Vector2_add(vj, nv_Vector2_muls(dirj, 0.1));

        if ((vi.y > point.y) != (vj.y > point.y) && (point.x < (vj.x - vi.x) * 
            (point.y - vi.y) / (vj.y - vi.y) + vi.x )) {
                inside = !inside;
            }

        j = i;
        i += 1;
    }

    return inside;
}


bool nv_collide_aabb_x_aabb(nv_AABB a, nv_AABB b) {
    return (!(a.max_x <= b.min_x || b.max_x <= a.min_x ||
              a.max_y <= b.min_y || b.max_y <= a.min_y));
}

bool nv_collide_aabb_x_point(nv_AABB aabb, nv_Vector2 point) {
    return (aabb.min_x <= point.x && point.x <= aabb.max_x &&
            aabb.min_y <= point.y && point.y <= aabb.max_y);
}