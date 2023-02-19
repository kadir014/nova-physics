/*

  This file is a part of the Nova Physics Engine
  project and distributed under the MIT license.

  Copyright © Kadir Aksoy
  https://github.com/kadir014/nova-physics

*/

#include "novaphysics/contact.h"
#include "novaphysics/array.h"
#include "novaphysics/constants.h"
#include "novaphysics/math.h"


/**
 * contact.c
 * 
 * Contact point calculation functions
 */


void nv_contact_circle_x_circle(nv_Resolution *res) {
    nv_Vector2 dir = nv_Vector2_sub(res->b->position, res->a->position);
    // If the bodies are in the exact same position, direct the normal upwards
    if (nv_Vector2_len2(dir) == 0.0) dir = NV_VEC2(0.0, 1.0);
    dir = nv_Vector2_normalize(dir);

    nv_Vector2 cp = nv_Vector2_add(res->a->position, nv_Vector2_muls(dir, res->a->radius));

    res->contact_count = 1;
    res->contacts[0] = cp;
}

void nv_contact_polygon_x_circle(nv_Resolution *res) {
    nv_Body *polygon;
    nv_Body *circle;

    if (res->a->shape == nv_BodyShape_POLYGON) {
        polygon = res->a;
        circle = res->b;
    } else {
        polygon = res->b;
        circle = res->a;
    }

    nv_Vector2 cp;
    double min_dist = NV_INF;

    nv_Polygon_model_to_world(polygon);
    nv_Array *vertices = polygon->trans_vertices;
    size_t n = vertices->size;

    double dist;
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

void nv_contact_polygon_x_polygon(nv_Resolution *res) {
    nv_Body *a = res->a;
    nv_Body *b = res->b;

    nv_Vector2 contact1 = nv_Vector2_zero;
    nv_Vector2 contact2 = nv_Vector2_zero;
    int contact_count = 0;

    double min_dist = NV_INF;

    nv_Polygon_model_to_world(a);
    nv_Polygon_model_to_world(b);
    nv_Array *vertices_a = a->trans_vertices;
    nv_Array *vertices_b = b->trans_vertices;
    size_t na = vertices_a->size;
    size_t nb = vertices_b->size;

    double dist;
    nv_Vector2 contact;

    // Check vertices on body A to edges on body B
    for (size_t i = 0; i < na; i++) {
        nv_Vector2 p = NV_TO_VEC2(vertices_a->data[i]);

        for (size_t j = 0; j < nb; j++) {
            nv_Vector2 va = NV_TO_VEC2(vertices_b->data[j]);
            nv_Vector2 vb = NV_TO_VEC2(vertices_b->data[(j + 1) % nb]);

            nv_point_segment_dist(p, va, vb, &dist, &contact);

            if (nv_nearly_eq(dist, min_dist)) {
                if (!nv_nearly_eqv(contact, contact1)) {
                    contact2 = contact;
                    contact_count = 2;
                }
            }

            else if (dist < min_dist) {
                min_dist = dist;
                contact1 = contact;
                contact_count = 1;
            }
        }
    }

    // Check vertices on body B to edges on body A
    for (size_t i = 0; i < nb; i++) {
        nv_Vector2 p = NV_TO_VEC2(vertices_b->data[nb-i-1]);

        for (size_t j = 0; j < na; j++) {
            nv_Vector2 va = NV_TO_VEC2(vertices_a->data[j]);
            nv_Vector2 vb = NV_TO_VEC2(vertices_a->data[(j + 1) % na]);

            nv_point_segment_dist(p, va, vb, &dist, &contact);

            if (nv_nearly_eq(dist, min_dist)) {
                if (!nv_nearly_eqv(contact, contact1)) {
                    contact2 = contact;
                    contact_count = 2;
                }
            }

            else if (dist < min_dist) {
                min_dist = dist;
                contact1 = contact;
                contact_count = 1;
            }
        }
    }

    res->contact_count = contact_count;
    res->contacts[0] = contact1;
    res->contacts[1] = contact2;
}