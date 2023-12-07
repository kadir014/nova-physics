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
 * @file collision.c
 * 
 * @brief Collision detection functions.
 */


nvResolution nv_collide_circle_x_circle(nvBody *a, nvBody *b) {
    nvResolution res = {
        .collision = false,
        .a = a,
        .b = b,
        .normal = nvVector2_zero,
        .depth = 0.0
        };

    nv_float dist2 = nvVector2_dist2(b->position, a->position);
    nv_float dist = nv_sqrt(dist2);
    nv_float radii = a->shape->radius + b->shape->radius;
    nv_float radii2 = radii * radii;

    // Circles aren't colliding
    if (dist2 >= radii2) return res;

    res.collision = true;

    nvVector2 normal = nvVector2_sub(b->position, a->position);
    // If the bodies are in the exact same position, direct the normal upwards
    if (nvVector2_len2(normal) == 0.0) normal = NV_VEC2(0.0, 1.0);
    else normal = nvVector2_normalize(normal);

    res.normal = normal;
    res.depth = radii - dist;

    return res;
}

bool nv_collide_circle_x_point(nvBody *circle, nvVector2 point) {
    return nvVector2_len2(
        nvVector2_sub(circle->position, point)) <= circle->shape->radius * circle->shape->radius;
}


nvResolution nv_collide_polygon_x_circle(nvBody *polygon, nvBody *circle) {
    nvResolution res = {
        .collision = false,
        .a = polygon,
        .b = circle,
        .normal = nvVector2_zero,
        .depth = NV_INF
        };

    nv_Polygon_model_to_world(polygon);
    nvArray *vertices = polygon->shape->trans_vertices;

    size_t n = vertices->size;

    nv_float min_a, min_b, max_a, max_b;

    for (size_t i = 0; i < n; i++) {
        nvVector2 va = NV_TO_VEC2(vertices->data[i]);
        nvVector2 vb = NV_TO_VEC2(vertices->data[(i + 1) % n]);

        nvVector2 edge = nvVector2_sub(vb, va);
        nvVector2 axis = nvVector2_normalize(nvVector2_perp(edge));

        nv_project_polyon(vertices, axis, &min_a, &max_a);
        nv_project_circle(circle->position, circle->shape->radius, axis, &min_b, &max_b);

        // Doesn't collide
        if (min_a >= max_b || min_b >= max_a) {
            return res;
        }

        nv_float axis_depth = nv_fmin(max_b - min_a, max_a - min_b);

        if (axis_depth < res.depth) {
            res.depth = axis_depth;
            res.normal = axis;
        }
    }

    nvVector2 cp = nv_polygon_closest_vertex_to_circle(circle->position, vertices);

    nvVector2 axis = nvVector2_normalize(nvVector2_sub(cp, circle->position));

    nv_project_polyon(vertices, axis, &min_a, &max_a);
    nv_project_circle(circle->position, circle->shape->radius, axis, &min_b, &max_b);

    // Doesn't collide
    if (min_a >= max_b || min_b >= max_a) {
        return res;
    }

    nv_float axis_depth = nv_fmin(max_b - min_a, max_a - min_b);

    if (axis_depth < res.depth) {
        res.depth = axis_depth;
        res.normal = axis;
    }

    nvVector2 direction = nvVector2_sub(polygon->position, circle->position);

    if (nvVector2_dot(direction, res.normal) > 0.0)
        res.normal = nvVector2_neg(res.normal);

    res.collision = true;

    return res;
}

nvResolution nv_collide_polygon_x_polygon(nvBody *a, nvBody *b) {
    nvResolution res = {
        .collision = false,
        .a = a,
        .b = b,
        .normal = nvVector2_zero,
        .depth = NV_INF
        };

    nv_Polygon_model_to_world(a);
    nv_Polygon_model_to_world(b);
    nvArray *vertices_a = a->shape->trans_vertices;
    nvArray *vertices_b = b->shape->trans_vertices;
    size_t na = vertices_a->size;
    size_t nb = vertices_b->size;

    size_t i;

    nv_float min_a, max_a, min_b, max_b;

    for (i = 0; i < na; i++) {
        nvVector2 va = NV_TO_VEC2(vertices_a->data[i]);
        nvVector2 vb = NV_TO_VEC2(vertices_a->data[(i + 1) % na]);

        nvVector2 edge = nvVector2_sub(vb, va);
        nvVector2 axis = nvVector2_normalize(nvVector2_perpr(edge));

        nv_project_polyon(vertices_a, axis, &min_a, &max_a);
        nv_project_polyon(vertices_b, axis, &min_b, &max_b);

        // Doesn't collide
        if (min_a >= max_b || min_b >= max_a) {
            return res;
        }

        nv_float axis_depth = nv_fmin(max_b - min_a, max_a - min_b);

        if (axis_depth < res.depth) {
            res.depth = axis_depth;
            res.normal = axis;
        }
    }

    for (i = 0; i < nb; i++) {
        nvVector2 va = NV_TO_VEC2(vertices_b->data[i]);
        nvVector2 vb = NV_TO_VEC2(vertices_b->data[(i + 1) % nb]);

        nvVector2 edge = nvVector2_sub(vb, va);
        nvVector2 axis = nvVector2_normalize(nvVector2_perpr(edge));

        nv_project_polyon(vertices_a, axis, &min_a, &max_a);
        nv_project_polyon(vertices_b, axis, &min_b, &max_b);

        // Doesn't collide
        if (min_a >= max_b || min_b >= max_a) {
            return res;
        }

        nv_float axis_depth = nv_fmin(max_b - min_a, max_a - min_b);

        if (axis_depth < res.depth) {
            res.depth = axis_depth;
            res.normal = axis;
        }
    }

    nvVector2 center_a = nvVector2_add(nv_polygon_centroid(a->shape->vertices), a->position);
    nvVector2 center_b = nvVector2_add(nv_polygon_centroid(b->shape->vertices), b->position);

    if (nvVector2_dot(nvVector2_sub(center_b, center_a), res.normal) < 0.0)
        res.normal = nvVector2_neg(res.normal);

    res.collision = true;

    return res;
}

bool nv_collide_polygon_x_point(nvBody *polygon, nvVector2 point) {
    // https://stackoverflow.com/a/48760556
    nv_Polygon_model_to_world(polygon);
    nvArray *vertices = polygon->shape->trans_vertices;

    size_t n = vertices->size;
    size_t i = 0;
    size_t j = n - 1;
    bool inside = false;

    while (i < n) {
        nvVector2 vi = NV_TO_VEC2(vertices->data[i]);
        nvVector2 vj = NV_TO_VEC2(vertices->data[j]);

        nvVector2 diri = nvVector2_normalize(nvVector2_sub(vi, polygon->position));
        nvVector2 dirj = nvVector2_normalize(nvVector2_sub(vj, polygon->position));

        vi = nvVector2_add(vi, nvVector2_mul(diri, 0.1));
        vj = nvVector2_add(vj, nvVector2_mul(dirj, 0.1));

        if ((vi.y > point.y) != (vj.y > point.y) && (point.x < (vj.x - vi.x) * 
            (point.y - vi.y) / (vj.y - vi.y) + vi.x )) {
                inside = !inside;
            }

        j = i;
        i += 1;
    }

    return inside;
}


bool nv_collide_aabb_x_aabb(nvAABB a, nvAABB b) {
    return (!(a.max_x <= b.min_x || b.max_x <= a.min_x ||
              a.max_y <= b.min_y || b.max_y <= a.min_y));
}

bool nv_collide_aabb_x_point(nvAABB aabb, nvVector2 point) {
    return (aabb.min_x <= point.x && point.x <= aabb.max_x &&
            aabb.min_y <= point.y && point.y <= aabb.max_y);
}