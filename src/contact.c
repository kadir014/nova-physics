/*

  This file is a part of the Nova Physics Engine
  project and distributed under the MIT license.

  Copyright © Kadir Aksoy
  https://github.com/kadir014/nova-physics

*/

#include "novaphysics/internal.h"
#include "novaphysics/contact.h"
#include "novaphysics/collision.h"
#include "novaphysics/array.h"
#include "novaphysics/constants.h"
#include "novaphysics/math.h"


/**
 * @file contact.c
 * 
 * @details Contact point calculation functions
 * 
 *          Function documentations are in novaphysics/contact.h
 */


void nv_contact_circle_x_circle(nv_Resolution *res) {
    nv_Vector2 dir = nv_Vector2_sub(res->b->position, res->a->position);
    // If the bodies are in the exact same position, direct the normal upwards
    if (nv_Vector2_len2(dir) == 0.0) dir = NV_VEC2(0.0, 1.0);
    dir = nv_Vector2_normalize(dir);

    nv_Vector2 cp = nv_Vector2_add(res->a->position, nv_Vector2_muls(dir, res->a->shape->radius));

    res->contact_count = 1;
    res->contacts[0] = cp;
}

void nv_contact_polygon_x_circle(nv_Resolution *res) {
    nv_Body *polygon;
    nv_Body *circle;

    if (res->a->shape->type == nv_ShapeType_POLYGON) {
        polygon = res->a;
        circle = res->b;
    } else {
        polygon = res->b;
        circle = res->a;
    }

    nv_Vector2 cp;
    nv_float min_dist = NV_INF;

    nv_Polygon_model_to_world(polygon);
    nv_Array *vertices = polygon->shape->trans_vertices;
    size_t n = vertices->size;

    nv_float dist;
    nv_Vector2 contact;

    for (size_t i = 0; i < n; i++) {
        nv_Vector2 va = NV_TO_VEC2(vertices->data[i]);
        nv_Vector2 vb = NV_TO_VEC2(vertices->data[(i + 1) % n]);

        nv_point_segment_dist(circle->position, va, vb, &dist, &contact);

        if (dist < min_dist) {
            min_dist = dist;
            cp = contact;
        }
    }

    res->contact_count = 1;
    res->contacts[0] = cp;
}


nv_float find_axis_least_penetration(
    size_t *face,
    nv_Body *a,
    nv_Body *b,
    nv_Mat22 au,
    nv_Mat22 bu
) {
    nv_float best_depth = -NV_INF;
    size_t best_i;

    for (size_t i = 0; i < a->shape->vertices->size; i++) {
        
        // Get face normal from body A
        nv_Vector2 n = NV_TO_VEC2(a->shape->normals->data[i]);
        nv_Vector2 nw = nv_Mat22_mulv(au, n);

        // Transform face normal into body B's model space
        nv_Mat22 b_ut = nv_Mat22_transpose(bu);
        n = nv_Mat22_mulv(b_ut, nw);

        // Get support point from body B along -n
        nv_Vector2 s = nv_polygon_support(b->shape->vertices, nv_Vector2_neg(n));

        // Get vertex on face from body A, transformed into body B's model space
        nv_Vector2 v = NV_TO_VEC2(a->shape->vertices->data[i]);
        v = nv_Vector2_add(nv_Mat22_mulv(au, v), a->position);
        v = nv_Vector2_sub(v, b->position);
        v = nv_Mat22_mulv(b_ut, v);

        // Compute penetration depth (in body B's model space)
        nv_float depth = nv_Vector2_dot(n, nv_Vector2_sub(s, v));

        if (depth > best_depth) {
            best_depth = depth;
            best_i = i;
        }
    }

    *face = best_i;
    return best_depth;
}

void find_incident_face(
    nv_Vector2 *face,
    nv_Body *ref,
    nv_Body *inc,
    nv_Mat22 refu,
    nv_Mat22 incu,
    size_t ref_i
) {
    nv_Vector2 ref_normal = NV_TO_VEC2(ref->shape->normals->data[ref_i]);

    // Calculate nmormal in incident's frame of reference
    ref_normal = nv_Mat22_mulv(refu, ref_normal);
    nv_Mat22 b_ut = nv_Mat22_transpose(incu);
    ref_normal = nv_Mat22_mulv(b_ut, ref_normal);

    // Find the "most anti-normal" face on incident shape
    size_t inc_face = 0;
    nv_float min_dot = NV_INF;

    for (size_t i = 0; i < inc->shape->vertices->size; i++) {
        nv_float dot = nv_Vector2_dot(ref_normal, NV_TO_VEC2(inc->shape->normals->data[i]));

        if (dot < min_dot) {
            min_dot = dot;
            inc_face = i;
        }
    }

    // Assign face vertices for inc_face
    face[0] = nv_Vector2_add(nv_Mat22_mulv(incu, NV_TO_VEC2(inc->shape->vertices->data[inc_face])), inc->position);
    inc_face = inc_face + 1 >= inc->shape->vertices->size ? 0 : inc_face + 1;
    face[1] = nv_Vector2_add(nv_Mat22_mulv(incu, NV_TO_VEC2(inc->shape->vertices->data[inc_face])), inc->position);
}

size_t clip(nv_Vector2 n, nv_float c, nv_Vector2 *face) {
    size_t sp = 0;
    nv_Vector2 out[2] = {face[0], face[1]};

    // Retrieve distances from each endpoint to the line
    // d = ax + by - c
    nv_float d1 = nv_Vector2_dot(n, face[0]) - c;
    nv_float d2 = nv_Vector2_dot(n, face[1]) - c;

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
        out[sp] = nv_Vector2_add(
            face[0],
            nv_Vector2_muls(
                nv_Vector2_sub(face[1], face[0]),
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


void nv_contact_polygon_x_polygon(nv_Resolution *res) {
    nv_Body *a = res->a;
    nv_Body *b = res->b;

    // Rotation matrices
    nv_Mat22 au = nv_Mat22_from_angle(a->angle);
    nv_Mat22 bu = nv_Mat22_from_angle(b->angle);
    nv_Mat22 refu;
    nv_Mat22 incu;

    // Check for a separating axis with body A's faces
    size_t face_a;
    nv_float depth_a = find_axis_least_penetration(&face_a, a, b, au, bu);
    if (depth_a >= 0.0) {
        res->collision = false;
        return;
    }

    // Check for a separating axis with body B's faces
    size_t face_b;
    nv_float depth_b = find_axis_least_penetration(&face_b, b, a, bu, au);
    if (depth_b >= 0.0) {
        res->collision = false;
        return;
    }

    size_t ref_i;
    bool flip; // Always point from body A to body B

    nv_Body *ref; // Reference body
    nv_Body *inc; // Incident body

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
    nv_Vector2 inc_face[2];
    find_incident_face(inc_face, ref, inc, refu, incu, ref_i);

    // Setup reference face vertices
    nv_Vector2 v1 = NV_TO_VEC2(ref->shape->vertices->data[ref_i]);
    ref_i = ref_i + 1 == ref->shape->vertices->size ? 0 : ref_i + 1;
    nv_Vector2 v2 = NV_TO_VEC2(ref->shape->vertices->data[ref_i]);

    // Transform vertices to world space
    v1 = nv_Vector2_add(nv_Mat22_mulv(refu, v1), ref->position);
    v2 = nv_Vector2_add(nv_Mat22_mulv(refu, v2), ref->position);

    // Calculate reference face side normal in world space
    nv_Vector2 side_normal = nv_Vector2_normalize(nv_Vector2_sub(v2, v1));

    nv_Vector2 ref_normal = nv_Vector2_perpr(side_normal);

    // ax + by = c  (c is distance from origin)
    nv_float c = nv_Vector2_dot(ref_normal, v1);
    nv_float neg_side = -nv_Vector2_dot(side_normal, v1);
    nv_float pos_side = nv_Vector2_dot(side_normal, v2);

    // Clip incident face to reference face side planes
    // Due to floating point errors it's possible to not have required points
    if (clip(nv_Vector2_neg(side_normal), neg_side, inc_face) < 2) {
        res->collision = false;
        return;
    }
    if (clip(side_normal, pos_side, inc_face) < 2) {
        res->collision = false;
        return;
    }

    res->normal = flip ? nv_Vector2_neg(ref_normal) : ref_normal;

    // Keep points behind reference face
    size_t cp = 0;

    nv_float separation = nv_Vector2_dot(ref_normal, inc_face[0]) - c;
    if (separation < 0.0) {
        res->contacts[cp] = inc_face[0];
        res->depth = -separation;
        cp++;
    }
    else
        res->depth = 0.0;

    separation = nv_Vector2_dot(ref_normal, inc_face[1]) - c;
    if (separation <= 0.0) {
        res->contacts[cp] = inc_face[1];
        res->depth += -separation;
        cp++;

        // Average penetration depth
        res->depth /= (nv_float)cp;
    }

    if (cp > 0) res->collision = true;
    res->contact_count = cp;
}