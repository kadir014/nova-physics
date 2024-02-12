/*

  This file is a part of the Nova Physics Engine
  project and distributed under the MIT license.

  Copyright © Kadir Aksoy
  https://github.com/kadir014/nova-physics

*/

#include "novaphysics/internal.h"
#include "novaphysics/matrix.h"
#include "novaphysics/contact.h"
#include "novaphysics/collision.h"
#include "novaphysics/array.h"
#include "novaphysics/constants.h"
#include "novaphysics/math.h"


/**
 * @file contact.c
 * 
 * @brief Contact point calculation functions.
 */


void nv_contact_circle_x_circle(nvResolution *res) {
    nvVector2 dir = nvVector2_sub(res->b->position, res->a->position);
    // If the bodies are in the exact same position, direct the normal upwards
    if (nvVector2_len2(dir) == 0.0) dir = NV_VEC2(0.0, 1.0);
    dir = nvVector2_normalize(dir);

    nvVector2 ap = nvVector2_add(res->a->position, nvVector2_mul(dir, res->a->shape->radius));
    nvVector2 bp = nvVector2_add(res->b->position, nvVector2_mul(dir, -res->b->shape->radius));
    nvVector2 cp = nvVector2_mul(nvVector2_add(ap, bp), 0.5);

    res->contact_count = 1;
    res->contacts[0] = (nvContact){.position = cp};
}

void nv_contact_polygon_x_circle(nvResolution *res) {
    nvBody *polygon;
    nvBody *circle;

    if (res->a->shape->type == nvShapeType_POLYGON) {
        polygon = res->a;
        circle = res->b;
    } else {
        polygon = res->b;
        circle = res->a;
    }

    nvVector2 cp;
    nv_float min_dist = NV_INF;

    nvBody_local_to_world(polygon);
    nvArray *vertices = polygon->shape->trans_vertices;
    size_t n = vertices->size;

    nv_float dist;
    nvVector2 contact;

    for (size_t i = 0; i < n; i++) {
        nvVector2 va = NV_TO_VEC2(vertices->data[i]);
        nvVector2 vb = NV_TO_VEC2(vertices->data[(i + 1) % n]);

        nv_point_segment_dist(circle->position, va, vb, &dist, &contact);

        if (dist < min_dist) {
            min_dist = dist;
            cp = contact;
        }
    }

    res->contact_count = 1;
    res->contacts[0] = (nvContact){.position = cp};
}


nv_float _find_axis_least_penetration(
    size_t *face,
    nvBody *a,
    nvBody *b,
    nvMat2x2 au,
    nvMat2x2 bu
) {
    nv_float best_depth = -NV_INF;
    size_t best_i = -1;

    for (size_t i = 0; i < a->shape->vertices->size; i++) {
        
        // Get face normal from body A
        nvVector2 n = NV_TO_VEC2(a->shape->normals->data[i]);
        nvVector2 nw = nvMat2x2_mulv(au, n);

        // Transform face normal into body B's model space
        nvMat2x2 b_ut = nvMat2x2_transpose(bu);
        n = nvMat2x2_mulv(b_ut, nw);

        // Get support point from body B along -n
        nvVector2 s = nv_polygon_support(b->shape->vertices, nvVector2_neg(n));

        // Get vertex on face from body A, transformed into body B's model space
        nvVector2 v = NV_TO_VEC2(a->shape->vertices->data[i]);
        v = nvVector2_add(nvMat2x2_mulv(au, v), a->position);
        v = nvVector2_sub(v, b->position);
        v = nvMat2x2_mulv(b_ut, v);

        // Compute penetration depth (in body B's model space)
        nv_float depth = nvVector2_dot(n, nvVector2_sub(s, v));

        if (depth > best_depth) {
            best_depth = depth;
            best_i = i;
        }
    }

    *face = best_i;
    return best_depth;
}

static inline void _find_incident_face(
    nvVector2 *face,
    nvBody *ref,
    nvBody *inc,
    nvMat2x2 refu,
    nvMat2x2 incu,
    size_t ref_i
) {
    nvVector2 ref_normal = NV_TO_VEC2(ref->shape->normals->data[ref_i]);

    // Calculate nmormal in incident's frame of reference
    ref_normal = nvMat2x2_mulv(refu, ref_normal);
    nvMat2x2 b_ut = nvMat2x2_transpose(incu);
    ref_normal = nvMat2x2_mulv(b_ut, ref_normal);

    // Find the "most anti-normal" face on incident shape
    size_t inc_face = 0;
    nv_float min_dot = NV_INF;

    for (size_t i = 0; i < inc->shape->vertices->size; i++) {
        nv_float dot = nvVector2_dot(ref_normal, NV_TO_VEC2(inc->shape->normals->data[i]));

        if (dot < min_dot) {
            min_dot = dot;
            inc_face = i;
        }
    }

    // Assign face vertices for inc_face
    face[0] = nvVector2_add(nvMat2x2_mulv(incu, NV_TO_VEC2(inc->shape->vertices->data[inc_face])), inc->position);
    inc_face = inc_face + 1 >= inc->shape->vertices->size ? 0 : inc_face + 1;
    face[1] = nvVector2_add(nvMat2x2_mulv(incu, NV_TO_VEC2(inc->shape->vertices->data[inc_face])), inc->position);
}

static inline size_t _clip_segment_to_line(
    nvVector2 n,
    nv_float c,
    nvVector2 *face
) {
    size_t sp = 0;
    nvVector2 out[2] = {face[0], face[1]};

    // Retrieve distances from each endpoint to the line
    // d = ax + by - c
    nv_float d1 = nvVector2_dot(n, face[0]) - c;
    nv_float d2 = nvVector2_dot(n, face[1]) - c;

    // If negative (behind plane), clip
    if (d1 <= 0.0) {
        out[sp] = face[0];
        sp++;
    }

    if (d2 <= 0.0) {
        out[sp] = face[1];
        sp++;
    }

    // If the points are on different sides of the plane
    if (d1 * d2 < 0.0) {
        // Push intersection point
        nv_float alpha = d1 / (d1 - d2);

        // f0 + a * (f1 - f0)
        out[sp] = nvVector2_add(
            face[0],
            nvVector2_mul(
                nvVector2_sub(face[1], face[0]),
                alpha
            )
        );
        sp++;
    }

    // Assign our new converted values
    face[0] = out[0];
    face[1] = out[1];

    if (sp == 3) NV_ERROR("there can't be 3 points???");

    return sp;
}

void nv_contact_polygon_x_polygon(nvResolution *res) {
    /*
        Erin Catto's GDC talk about polygon clipping for contact generation:
            https://box2d.org/files/ErinCatto_ContactManifolds_GDC2007.pdf

        Box2D-Lite's implementation:
            https://github.com/erincatto/box2d-lite/blob/master/src/Collide.cpp

        Randy Gaul's implementation:
            https://github.com/RandyGaul/ImpulseEngine/blob/master/Collision.cpp
    */

    nvBody *a = res->a;
    nvBody *b = res->b;

    // Rotation matrices
    nvMat2x2 au = nvMat2x2_from_angle(a->angle);
    nvMat2x2 bu = nvMat2x2_from_angle(b->angle);
    nvMat2x2 refu;
    nvMat2x2 incu;

    // Check for a separating axis with body A's faces
    size_t face_a;
    nv_float depth_a = _find_axis_least_penetration(&face_a, a, b, au, bu);
    if (depth_a >= 0.0) {
        res->collision = false;
        return;
    }

    // Check for a separating axis with body B's faces
    size_t face_b;
    nv_float depth_b = _find_axis_least_penetration(&face_b, b, a, bu, au);
    if (depth_b >= 0.0) {
        res->collision = false;
        return;
    }

    size_t ref_i;
    bool flip; // Always point from body A to body B

    nvBody *ref; // Reference body
    nvBody *inc; // Incident body

    // Determine which shapes contains reference face
    if (nv_bias_greater_than(depth_a, depth_b)) {
        ref = a;
        inc = b;
        refu = au;
        incu = bu;
        ref_i = face_a;
        flip = false;
    }
    else {
        ref = b;
        inc = a;
        refu = bu;
        incu = au;
        ref_i = face_b;
        flip = true;
    }

    // World space incident face
    nvVector2 inc_face[2];
    _find_incident_face(inc_face, ref, inc, refu, incu, ref_i);

    // Setup reference face vertices
    nvVector2 v1 = NV_TO_VEC2(ref->shape->vertices->data[ref_i]);
    ref_i = ref_i + 1 == ref->shape->vertices->size ? 0 : ref_i + 1;
    nvVector2 v2 = NV_TO_VEC2(ref->shape->vertices->data[ref_i]);

    // Transform vertices to world space
    v1 = nvVector2_add(nvMat2x2_mulv(refu, v1), ref->position);
    v2 = nvVector2_add(nvMat2x2_mulv(refu, v2), ref->position);

    // Calculate reference face side normal in world space
    nvVector2 side_normal = nvVector2_normalize(nvVector2_sub(v2, v1));

    nvVector2 ref_normal = nvVector2_perpr(side_normal);

    // ax + by = c  (c is distance from origin)
    nv_float c = nvVector2_dot(ref_normal, v1);
    nv_float neg_side = -nvVector2_dot(side_normal, v1);
    nv_float pos_side = nvVector2_dot(side_normal, v2);

    // Clip incident face to reference face side planes
    // Due to floating point errors it's possible to not have required points
    if (_clip_segment_to_line(nvVector2_neg(side_normal), neg_side, inc_face) < 2) {
        res->collision = false;
        return;
    }
    if (_clip_segment_to_line(side_normal, pos_side, inc_face) < 2) {
        res->collision = false;
        return;
    }

    res->normal = flip ? nvVector2_neg(ref_normal) : ref_normal;

    // Keep points behind reference face
    nv_uint8 cp = 0;

    nv_float separation = nvVector2_dot(ref_normal, inc_face[0]) - c;
    if (separation < 0.0) {
        res->contacts[cp] = (nvContact){.position = inc_face[0]};
        res->depth = -separation;
        cp++;
    }
    else
        res->depth = 0.0;

    separation = nvVector2_dot(ref_normal, inc_face[1]) - c;
    if (separation <= 0.0) {
        res->contacts[cp] = (nvContact){.position = inc_face[1]};
        res->depth += -separation;
        cp++;

        // Average penetration depth
        res->depth /= (nv_float)cp;
    }

    if (cp > 0) res->collision = true;
    res->contact_count = cp;
}