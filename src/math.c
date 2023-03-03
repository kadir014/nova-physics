/*

  This file is a part of the Nova Physics Engine
  project and distributed under the MIT license.

  Copyright © Kadir Aksoy
  https://github.com/kadir014/nova-physics

*/

#include <stdbool.h>
#include <math.h>
#include "novaphysics/internal.h"
#include "novaphysics/math.h"
#include "novaphysics/constants.h"
#include "novaphysics/array.h"
#include "novaphysics/vector.h"


/**
 * @file math.c
 * 
 * @details Nova physics math utilities
 * 
 *          Function documentations are in novaphysics/math.h
 */


bool nv_nearly_eq(nv_float a, nv_float b) {
    return nv_fabs(a - b) < NV_NEARLY_EQUAL_THRESHOLD;
}

bool nv_nearly_eqv(nv_Vector2 a, nv_Vector2 b) {
    return nv_nearly_eq(a.x, b.x) && nv_nearly_eq(a.y, b.y);
}


nv_Vector2 nv_calc_relative_velocity(
    nv_Vector2 linear_velocity_a,
    nv_float angular_velocity_a,
    nv_Vector2 ra,
    nv_Vector2 linear_velocity_b,
    nv_float angular_velocity_b,
    nv_Vector2 rb
) {
    /*
        Relative velocity

        vᴬᴮ = (vᴮ + wv * r⊥ᴮᴾ) - (vᴬ + wᴬ * r⊥ᴬᴾ)
    */

    nv_Vector2 ra_perp = nv_Vector2_perp(ra);
    nv_Vector2 rb_perp = nv_Vector2_perp(rb);

    return nv_Vector2_sub(
        nv_Vector2_add(linear_velocity_b, nv_Vector2_muls(rb_perp, angular_velocity_b)),
        nv_Vector2_add(linear_velocity_a, nv_Vector2_muls(ra_perp, angular_velocity_a))
    );
}

nv_float nv_calc_mass_k(
    nv_Vector2 normal,
    nv_Vector2 ra,
    nv_Vector2 rb,
    nv_float invmass_a,
    nv_float invmass_b,
    nv_float invinertia_a,
    nv_float invinertia_b
) {
    /*
        Effective mass

        1   1   (r⊥ᴬᴾ · n)²   (r⊥ᴮᴾ · n)²
        ─ + ─ + ─────────── + ───────────
        Mᴬ  Mᴮ      Iᴬ            Iᴮ
    */

    nv_Vector2 ra_perp = nv_Vector2_perp(ra);
    nv_Vector2 rb_perp = nv_Vector2_perp(rb);

    nv_float ran = nv_Vector2_dot(ra_perp, normal);
    nv_float rbn = nv_Vector2_dot(rb_perp, normal);
    ran *= ran;
    rbn *= rbn;

    return (invmass_a + invmass_b) + ((ran * invinertia_a) + (rbn * invinertia_b));
}


nv_float nv_circle_area(nv_float radius) {
    return NV_PI * (radius * radius);
}

nv_float nv_circle_inertia(nv_float mass, nv_float radius) {
    return 0.5 * mass * (radius * radius);
}

nv_float nv_polygon_area(nv_Array *vertices) {
    nv_float area = 0.0;
    size_t n = vertices->size;

    size_t j = n - 1;
    for (size_t i = 0; i < n; i++) {
        nv_Vector2 *va = vertices->data[i];
        nv_Vector2 *vb = vertices->data[j];

        area += (vb->x + va->x) *
                (vb->y - va->y);
        j = i;
    }

    return fabs(area / 2.0);
}

nv_float nv_polygon_inertia(nv_float mass, nv_Array *vertices) {
    nv_float sum1 = 0.0;
    nv_float sum2 = 0.0;
    size_t n = vertices->size;

    for (size_t i = 0; i < n; i++) {
        nv_Vector2 v1 = NV_TO_VEC2(vertices->data[i]);
        nv_Vector2 v2 = NV_TO_VEC2(vertices->data[(i + 1) % n]);

        nv_float a = nv_Vector2_cross(v2, v1);
        nv_float b = nv_Vector2_dot(v1, v1) +
                   nv_Vector2_dot(v1, v2) +
                   nv_Vector2_dot(v2, v2);
        
        sum1 += a * b;
        sum2 += a;
    }

    return (mass * sum1) / (6.0 * sum2);
}

nv_Vector2 nv_polygon_centroid(nv_Array *vertices) {
    nv_Vector2 sum = nv_Vector2_zero;
    size_t n = vertices->size;

    for (size_t i = 0; i < n; i++) {
        sum = nv_Vector2_add(sum, NV_TO_VEC2(vertices->data[i]));
    }

    return nv_Vector2_divs(sum, (nv_float)n);
}

void nv_project_circle(
    nv_Vector2 center,
    nv_float radius,
    nv_Vector2 axis,
    nv_float *min_out,
    nv_float *max_out
) {
    nv_Vector2 a = nv_Vector2_muls(nv_Vector2_normalize(axis), radius);

    nv_Vector2 p1 = nv_Vector2_add(center, a);
    nv_Vector2 p2 = nv_Vector2_sub(center, a);

    nv_float min = nv_Vector2_dot(p1, axis);
    nv_float max = nv_Vector2_dot(p2, axis);

    if (min > max) {
        nv_float temp = max;
        max = min;
        min = temp;
    }

    *min_out = min;
    *max_out = max;
}

void nv_project_polyon(
    nv_Array *vertices,
    nv_Vector2 axis,
    nv_float *min_out,
    nv_float *max_out
) {
    nv_float min = NV_INF;
    nv_float max = -NV_INF;

    for (size_t i = 0; i < vertices->size; i++) {
        nv_float projection = nv_Vector2_dot(NV_TO_VEC2(vertices->data[i]), axis);
        
        if (projection < min) min = projection;

        if (projection > max) max = projection;
    }

    *min_out = min;
    *max_out = max;
}


void nv_point_segment_dist(
    nv_Vector2 center,
    nv_Vector2 a,
    nv_Vector2 b,
    nv_float *dist_out,
    nv_Vector2 *contact_out
) {
    nv_Vector2 ab = nv_Vector2_sub(b, a);
    nv_Vector2 ap = nv_Vector2_sub(center, a);

    nv_float projection = nv_Vector2_dot(ap, ab);
    nv_float ab_len = nv_Vector2_len2(ab);
    nv_float dist = projection / ab_len;
    nv_Vector2 contact;

    if (dist <= 0.0) contact = a;

    else if (dist >= 1.0) contact = b;

    else contact = nv_Vector2_add(a, nv_Vector2_muls(ab, dist));

    *dist_out = nv_Vector2_dist2(center, contact);
    *contact_out = contact;
}


nv_Vector2 nv_polygon_closest_vertex_to_circle(
    nv_Vector2 center,
    nv_Array *vertices
) {
    bool found = false;
    size_t closest = 0;
    nv_float min_dist = NV_INF;
    
    for (size_t i = 0; i < vertices->size; i++) {
        nv_float dist = nv_Vector2_dist2(NV_TO_VEC2(vertices->data[i]), center);

        if (dist < min_dist) {
            min_dist = dist;
            closest = i;
            found = true;
        }
    }

    NV_ASSERT(found, NULL);

    return NV_TO_VEC2(vertices->data[closest]);
}