/*

  This file is a part of the Nova Physics Engine
  project and distributed under the MIT license.

  Copyright © Kadir Aksoy
  https://github.com/kadir014/nova-physics

*/

#ifndef NOVAPHYSICS_MATH_H
#define NOVAPHYSICS_MATH_H

#include <stdbool.h>
#include <stdint.h>
#include "novaphysics/internal.h"
#include "novaphysics/array.h"
#include "novaphysics/vector.h"
#include "novaphysics/constants.h"


/**
 * @file math.h
 * 
 * @brief Nova physics math utilities.
 */


/**
 * @brief Hash unsigned 32-bit integer
 * 
 * @param key Integer key to hash
 * @return nv_uint32
 */
static inline nv_uint32 nv_hash(nv_uint32 key) {
    // https://stackoverflow.com/a/12996028
    key = ((key >> 16) ^ key) * 0x45d9f3b;
    key = ((key >> 16) ^ key) * 0x45d9f3b;
    key = (key >> 16) ^ key;
    return key;
}

/**
 * @brief Combine two 16-bit integers into unsigned 32-bit one
 * 
 * @param x First integer
 * @param y Second ineger
 * @return nv_uint32
 */
static inline nv_uint32 nv_pair(nv_int16 x, nv_int16 y) {
    // https://stackoverflow.com/a/919631
    return ((unsigned)x << 16) | (unsigned)y;
}


/**
 * @brief Clamp a value between a range
 * 
 * @param value Value
 * @param min_value Minimum value of the range
 * @param max_value Maximum value of the range
 * @return nv_float 
 */
static inline nv_float nv_fclamp(nv_float value, nv_float min_value, nv_float max_value) {
    return nv_fmin(nv_fmax(value, min_value), max_value);
}


/**
 * @brief Check if two values are close enough to be considered as equal.
 * 
 * @param a Left-hand value
 * @param b Right-hand value
 * @return bool
 */
static inline bool nv_nearly_eq(nv_float a, nv_float b) {
    return nv_fabs(a - b) < NV_NEARLY_EQUAL_THRESHOLD;
}

/**
 * @overload Check if two vectors are close enough to be considered as equal.
 * 
 * @param a Left-hand vector
 * @param b Right-hand vector
 * @return bool
 */
static inline bool nv_nearly_eqv(nv_Vector2 a, nv_Vector2 b) {
    return nv_nearly_eq(a.x, b.x) && nv_nearly_eq(a.y, b.y);
}


static inline bool nv_bias_greater_than(nv_float a, nv_float b) {
    // TODO: Look into Box2D's bias function
    nv_float k_biasRelative = 0.95;
    nv_float k_biasAbsolute = 0.01;
    return a >= b * k_biasRelative + a * k_biasAbsolute;
}


/**
 * @brief Calculate relative velocity
 * 
 * @param linear_velocity_a Linear velocity of body A
 * @param anuglar_velocity_a Angular velocity of body A
 * @param ra Vector from body A position to its local anchor point 
 * @param linear_velocity_b Linear velocity of body B
 * @param anuglar_velocity_b Angular velocity of body B
 * @param rb Vector from body B position to its local anchor point 
 * @return nv_Vector2 
 */
static inline nv_Vector2 nv_calc_relative_velocity(
    nv_Vector2 linear_velocity_a,
    nv_float angular_velocity_a,
    nv_Vector2 ra,
    nv_Vector2 linear_velocity_b,
    nv_float angular_velocity_b,
    nv_Vector2 rb
) {
    /*
        Relative velocity

        vᴬᴮ = (vᴮ + wᴮ * r⊥ᴮᴾ) - (vᴬ + wᴬ * r⊥ᴬᴾ)
    */

    nv_Vector2 ra_perp = nv_Vector2_perp(ra);
    nv_Vector2 rb_perp = nv_Vector2_perp(rb);

    return nv_Vector2_sub(
        nv_Vector2_add(linear_velocity_b, nv_Vector2_mul(rb_perp, angular_velocity_b)),
        nv_Vector2_add(linear_velocity_a, nv_Vector2_mul(ra_perp, angular_velocity_a))
    );
}

/**
 * @brief Calculate effective mass
 * 
 * @param normal Constraint axis
 * @param ra Vector from body A position to contact point
 * @param rb vector from body B position to contact point
 * @param invmass_a Inverse mass (1/M) of body A
 * @param invmass_b Inverse mass (1/M) of body B
 * @param invinertia_a Inverse moment of inertia (1/I) of body A
 * @param invinertia_b Inverse moment of inertia (1/I) of body B
 * @return nv_float 
 */
static inline nv_float nv_calc_mass_k(
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


/**
 * @brief Calculate area of a circle (πr²).
 * 
 * @param radius Radius of the circle
 * @return nv_float 
 */
static inline nv_float nv_circle_area(nv_float radius) {
    return NV_PI * (radius * radius);
}

/**
 * @brief Calculate moment of inertia of a circle (1/2 mr²).
 * 
 * @param mass Mass of the circles
 * @param radius Radius of the circle
 * @return nv_float 
 */
static inline nv_float nv_circle_inertia(nv_float mass, nv_float radius) {
    return 0.5 * mass * (radius * radius);
}

/**
 * @brief Calculate area of a polygon (Shoelace formula).
 * 
 * @param vertices Array of vertices of polygon
 * @return nv_float 
 */
static inline nv_float nv_polygon_area(nv_Array *vertices) {
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

/**
 * @brief Calculate moment of inertia of a polygon.
 * 
 * @param mass Mass of the polygon
 * @param vertices Array of vertices of polygon
 * @return nv_float 
 */
static inline nv_float nv_polygon_inertia(nv_float mass, nv_Array *vertices) {
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

/**
 * @brief Calculate centroid of a polygon.
 * 
 * @param vertices Array of vertices of polygon
 * @return nv_Vector2
 */
static inline nv_Vector2 nv_polygon_centroid(nv_Array *vertices) {
    nv_Vector2 sum = nv_Vector2_zero;
    size_t n = vertices->size;

    for (size_t i = 0; i < n; i++) {
        sum = nv_Vector2_add(sum, NV_TO_VEC2(vertices->data[i]));
    }

    return nv_Vector2_div(sum, (nv_float)n);
}


/**
 * @brief Project circle onto axis and return extreme points.
 * 
 * @param center Center of circle
 * @param radius Radius of circle
 * @param axis Axis vector to project on
 * @param min_out Pointer for out min value
 * @param max_out Pointer for out max value
 */
static inline void nv_project_circle(
    nv_Vector2 center,
    nv_float radius,
    nv_Vector2 axis,
    nv_float *min_out,
    nv_float *max_out
) {
    nv_Vector2 a = nv_Vector2_mul(nv_Vector2_normalize(axis), radius);

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

/**
 * @brief Project polygon onto axis and return extreme points.
 * 
 * @param vertices Vertices of the polygon
 * @param axis Axis vector to project on
 * @param min_out Pointer for out min value
 * @param max_out Pointer for out max value
 */
static inline void nv_project_polyon(
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


/**
 * @brief Get support vertex of a polygon along the axis.
 * 
 * @param vertices Vertices of the polygon
 * @param axis Axis
 * @return nv_Vector2
 */
static inline nv_Vector2 nv_polygon_support(nv_Array *vertices, nv_Vector2 axis) {
    nv_float best_proj = -NV_INF;
    nv_Vector2 best_vertex;

    for (size_t i = 0; i < vertices->size; i++) {
        nv_Vector2 v = NV_TO_VEC2(vertices->data[i]);
        nv_float proj = nv_Vector2_dot(v, axis);

        if (proj > best_proj) {
            best_proj = proj;
            best_vertex = v;
        }
    }

    return best_vertex;
}


/**
 * @brief Perpendicular distance between point and line segment.
 * 
 * @param center Point
 * @param a Line segment start
 * @param b Line segment end
 * @param dist_out Distance
 * @param contact_out Contact point
 */
static inline void nv_point_segment_dist(
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

    else contact = nv_Vector2_add(a, nv_Vector2_mul(ab, dist));

    *dist_out = nv_Vector2_dist2(center, contact);
    *contact_out = contact;
}


/**
 * @brief Find closest vertex of the polygon to the circle.

 * @param center Center of the circle
 * @param vertices Vertices of the polygon
 * @return nv_Vector2 
 */
static inline nv_Vector2 nv_polygon_closest_vertex_to_circle(
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


#endif