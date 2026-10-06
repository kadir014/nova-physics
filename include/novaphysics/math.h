/*

  This file is a part of the Nova Physics Engine
  project and distributed under the MIT license.

  Copyright © Kadir Aksoy
  https://github.com/kadir014/nova-physics

*/

#ifndef NOVAPHYSICS_MATH_H
#define NOVAPHYSICS_MATH_H

#include "novaphysics/internal.h"
#include "novaphysics/vector.h"
#include "novaphysics/constants.h"


/**
 * @file math.h
 * 
 * @brief Nova Physics math utilities.
 */


/**
 * @brief Combine two 32-bit unsigned integers into unsigned 64-bit one.
 * 
 * This function is injective (every distinct ordered pair of integers
 * produces a unique new integer).
 * 
 * @param x First integer.
 * @param y Second ineger.
 * @return Combined 64-bit unsigned integer.
 */
static inline nv_uint64 nv_pair_u32_to_u64(nv_uint32 x, nv_uint32 y) {
    // https://stackoverflow.com/a/2769598
    return ((nv_uint64)x << 32) | (nv_uint64)y;
}

/**
 * @brief Hash 64-bit unsigned integer into 64-bit unsigned integer.
 * 
 * @param x Integer to hash.
 * @return Hashed integer.
 */
static inline nv_uint64 nv_hash_u64_to_u64(nv_uint64 x) {
    /*
        Thomas Wang's 64-bit mix function, licensed under Public Domain.
        https://web.archive.org/web/20071123051617/http://www.concentric.net/~Ttwang/tech/inthash.htm
    */

    x = (~x) + (x << 21); // key = (key << 21) - key - 1;
    x = x ^ (x >> 24);
    x = (x + (x << 3)) + (x << 8); // key * 265
    x = x ^ (x >> 14);
    x = (x + (x << 2)) + (x << 4); // key * 21
    x = x ^ (x >> 28);
    x = x + (x << 31);
    return x;
}


/**
 * @brief Clamp a value between a range.
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
 * @brief Calculate relative velocity.
 * 
 * @param linear_velocity_a Linear velocity of body A
 * @param anuglar_velocity_a Angular velocity of body A
 * @param ra Vector from body A position to its local anchor point 
 * @param linear_velocity_b Linear velocity of body B
 * @param anuglar_velocity_b Angular velocity of body B
 * @param rb Vector from body B position to its local anchor point 
 * @return nvVector2 
 */
static inline nvVector2 nv_calc_relative_velocity(
    nvVector2 linear_velocity_a,
    nv_float angular_velocity_a,
    nvVector2 ra,
    nvVector2 linear_velocity_b,
    nv_float angular_velocity_b,
    nvVector2 rb
) {
    /*
        Relative velocity

        vᴬᴮ = (vᴮ + wᴮ * r⊥ᴮᴾ) - (vᴬ + wᴬ * r⊥ᴬᴾ)
    */

    nvVector2 ra_perp = nvVector2_perp(ra);
    nvVector2 rb_perp = nvVector2_perp(rb);

    return nvVector2_sub(
        nvVector2_add(linear_velocity_b, nvVector2_mul(rb_perp, angular_velocity_b)),
        nvVector2_add(linear_velocity_a, nvVector2_mul(ra_perp, angular_velocity_a))
    );
}

/**
 * @brief Calculate effective mass.
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
    nvVector2 normal,
    nvVector2 ra,
    nvVector2 rb,
    nv_float invmass_a,
    nv_float invmass_b,
    nv_float invinertia_a,
    nv_float invinertia_b
) {
    /*
        Effective mass

        1   1   (r⊥ᴬᴾ · n)^2  (r⊥ᴮᴾ · n)^2
        ─ + ─ + ─────────── + ───────────
        Mᴬ  Mᴮ      Iᴬ            Iᴮ
    */

    nvVector2 ra_perp = nvVector2_perp(ra);
    nvVector2 rb_perp = nvVector2_perp(rb);

    nv_float ran = nvVector2_dot(ra_perp, normal);
    nv_float rbn = nvVector2_dot(rb_perp, normal);
    ran *= ran;
    rbn *= rbn;

    return (invmass_a + invmass_b) + ((ran * invinertia_a) + (rbn * invinertia_b));
}


/**
 * @brief Calculate area of a circle.
 * 
 * @param radius Radius of the circle
 * @return nv_float 
 */
static inline nv_float nv_circle_area(nv_float radius) {
    // πr^2
    return (nv_float)NV_PI * (radius * radius);
}

/**
 * @brief Calculate moment of inertia of a circle.
 * 
 * @param mass Mass of the circles
 * @param radius Radius of the circle
 * @param offset Center offset 
 * @return nv_float 
 */
static inline nv_float nv_circle_inertia(
    nv_float mass,
    nv_float radius,
    nvVector2 offset
) {
    // Circle inertia from center: 1/2 mr^2
    // The Parallel Axis Theorem: I = Ic + mh^2
    // 1/2 mr^2 + mh^2
    return 0.5f * mass * (radius * radius) + mass * nvVector2_len2(offset);
}

/**
 * @brief Calculate area of a polygon.
 * 
 * @param vertices Array of vertices of polygon
 * @param num_vertices Number of vertices
 * @return nv_float 
 */
static inline nv_float nv_polygon_area(
    nvVector2 *vertices,
    size_t num_vertices
) {
    // https://en.wikipedia.org/wiki/Shoelace_formula

    nv_float area = 0.0f;

    size_t j = num_vertices - 1;
    for (size_t i = 0; i < num_vertices; i++) {
        nvVector2 va = vertices[i];
        nvVector2 vb = vertices[j];

        area += (vb.x + va.x) * (vb.y - va.y);
        j = i;
    }

    return nv_fabs(area * 0.5f);
}

/**
 * @brief Calculate moment of inertia of a polygon.
 * 
 * @param mass Mass of the polygon
 * @param vertices Array of vertices of polygon
 * @param num_vertices Number of vertices
 * @return nv_float 
 */
static inline nv_float nv_polygon_inertia(
    nv_float mass,
    nvVector2 *vertices,
    size_t num_vertices
) {
    nv_float sum1 = 0.0f;
    nv_float sum2 = 0.0f;

    for (size_t i = 0; i < num_vertices; i++) {
        nvVector2 v1 = vertices[i];
        nvVector2 v2 = vertices[(i + 1) % num_vertices];

        nv_float a = nvVector2_cross(v2, v1);
        nv_float b = nvVector2_dot(v1, v1) +
                   nvVector2_dot(v1, v2) +
                   nvVector2_dot(v2, v2);
        
        sum1 += a * b;
        sum2 += a;
    }

    return (mass * sum1) / (6.0f * sum2);
}

/**
 * @brief Calculate centroid of a polygon.
 * 
 * @param vertices Array of vertices of polygon.
 * @param num_vertices Number of vertices.
 * @return Centroid of the polygon.
 */
static inline nvVector2 nv_polygon_centroid(
    nvVector2 *vertices,
    size_t num_vertices
) {
    // https://en.wikipedia.org/wiki/Centroid#Of_a_polygon

    // nv_polygon_area returns the absolute area, not signed, so get the signed
    // area here while accumulating centroid.

    nv_float area2 = 0.0f;
    nvVector2 centroid = nvVector2_zero;

    for (size_t i = 0; i < num_vertices; i++) {
        nvVector2 a = vertices[i];
        nvVector2 b = vertices[(i + 1) % num_vertices];

        nv_float cross = nvVector2_cross(a, b);

        area2 += cross;
        centroid.x += (a.x + b.x) * cross;
        centroid.y += (a.y + b.y) * cross;
    }

    centroid.x /= 3.0f * area2;
    centroid.y /= 3.0f * area2;

    return centroid;
}


/**
 * @brief Check winding order of a triangle.
 * 
 * Returns:
 *   -1 if CW
 *   1 if CCW
 *   0 if colliniear
 * 
 * @param vertices Triangle vertices
 * @return int Winding order
 */
static inline int nv_triangle_winding(nvVector2 vertices[3]) {
    nvVector2 ba = nvVector2_sub(vertices[1], vertices[0]);
    nvVector2 ca = nvVector2_sub(vertices[2], vertices[0]);
    nv_float z = nvVector2_cross(ba, ca);

    if (z < 0.0f) return -1;
    else if (z > 0.0f) return 1;
    else return 0;
}

static inline float nv_signed_triangle_area(nvVector2 a, nvVector2 b, nvVector2 c) {
    return (b.x - a.x) * (c.y - a.y) - (b.y - a.y) * (c.x - a.x);
}

static inline size_t _nv_quickhull_find_hull(
    const nvVector2 *points,
    size_t n_points,
    nvVector2 a,
    nvVector2 b,
    nvVector2 *out
) {
    if (n_points == 0) {
        return 0;
    }

    // Farthest point from line AB
    nvVector2 c = NV_VECTOR2(-NV_INF, -NV_INF);
    nv_float c_k = -NV_INF;
    for (size_t i = 0; i < n_points; i++) {
        nv_float k = nv_fabs(nv_signed_triangle_area(a, b, points[i]));
        if (k > c_k) {
            c = points[i];
            c_k = k;
        }
    }

    // Points outside edge A -> C
    nvVector2 *left_ac = NV_MALLOC(sizeof(nvVector2) * n_points);
    size_t n_left_ac = 0;
    for (size_t i = 0; i < n_points; i++) {
        if (nv_signed_triangle_area(a, c, points[i]) > 0.0f) {
            left_ac[n_left_ac++] = points[i];
        }
    }

    // Points outside edge C -> B
    nvVector2 *left_cb = NV_MALLOC(sizeof(nvVector2) * n_points);
    size_t n_left_cb = 0;
    for (size_t i = 0; i < n_points; i++) {
        if (nv_signed_triangle_area(c, b, points[i]) > 0.0f) {
            left_cb[n_left_cb++] = points[i];
        }
    }

    nvVector2 *ac_hull = NV_MALLOC(sizeof(nvVector2) * n_points);
    size_t n_ac_hull = _nv_quickhull_find_hull(left_ac, n_left_ac, a, c, ac_hull);

    nvVector2 *cb_hull = NV_MALLOC(sizeof(nvVector2) * n_points);
    size_t n_cb_hull = _nv_quickhull_find_hull(left_cb, n_left_cb, c, b, cb_hull);

    // out -> ac_hull + [c] + cb_hull

    size_t n_out = 0;

    for (size_t i = 0; i < n_ac_hull; i++) {
        out[n_out++] = ac_hull[i];
    }

    out[n_out++] = c;

    for (size_t i = 0; i < n_cb_hull; i++) {
        out[n_out++] = cb_hull[i];
    }

    NV_FREE(cb_hull);
    NV_FREE(ac_hull);
    NV_FREE(left_cb);
    NV_FREE(left_ac);

    return n_out;
}

/**
 * @brief Generate a convex hull around the given point cloud using QuickHull algorithm.
 * 
 * @param points Point cloud.
 * @param n_points Number of points.
 * @param hull Generated output hull buffer.
 * @return Number of vertices in generated hull.
 */
static inline size_t nv_quickhull(
    const nvVector2 *points,
    size_t n_points,
    nvVector2 *hull
) {
    // This function implements QuickHull algorithm.
    // https://en.wikipedia.org/wiki/Quickhull

    if (n_points <= 2) {
        for (size_t i = 0; i < n_points; i++) {
            hull[i] = points[i];
        }
        return n_points;
    }

    // Find extreme endpoints
    nvVector2 a = NV_VECTOR2(NV_INF, NV_INF);
    for (size_t i = 0; i < n_points; i++) {
        if (points[i].x < a.x || (points[i].x == a.x && points[i].y < a.y)) {
            a = points[i];
        }
    }
    nvVector2 b = NV_VECTOR2(-NV_INF, -NV_INF);
    for (size_t i = 0; i < n_points; i++) {
        if (points[i].x > b.x || (points[i].x == b.x && points[i].y > b.y)) {
            b = points[i];
        }
    }

    // TODO: Expose NV_QUICKHULL_EPSILON or smth
    if (nvVector2_dist2(a, b) <= 0.00001f) {
        hull[0] = a;
        return 1;
    }

    // Split cloud on either side of AB

    nvVector2 *left = NV_MALLOC(sizeof(nvVector2) * n_points);
    size_t n_left = 0;
    for (size_t i = 0; i < n_points; i++) {
        if (nv_signed_triangle_area(a, b, points[i]) > 0.0f) {
            left[n_left++] = points[i];
        }
    }

    nvVector2 *right = NV_MALLOC(sizeof(nvVector2) * n_points);
    size_t n_right = 0;
    for (size_t i = 0; i < n_points; i++) {
        if (nv_signed_triangle_area(a, b, points[i]) < 0.0f) {
            right[n_right++] = points[i];
        }
    }

    // Recursively gather verts outside candidate edges

    nvVector2 *left_hull = NV_MALLOC(sizeof(nvVector2) * n_points);
    size_t n_left_hull = _nv_quickhull_find_hull(left, n_left, a, b, left_hull);

    nvVector2 *right_hull = NV_MALLOC(sizeof(nvVector2) * n_points);
    size_t n_right_hull = _nv_quickhull_find_hull(right, n_right, b, a, right_hull);

    // out -> [a] + left_hull + [b] + right_hull

    size_t n_out = 0;

    hull[n_out++] = a;

    // TODO: Better max_vertices truncation, [b] should not be left out!

    for (size_t i = 0; i < n_left_hull; i++)  {
        if (n_out >= NV_POLYGON_MAX_VERTICES) {
            break;
        }
        hull[n_out++] = left_hull[i];
    }

    if (n_out < NV_POLYGON_MAX_VERTICES) {
        hull[n_out++] = b;
    }

    for (size_t i = 0; i < n_right_hull; i++)  {
        if (n_out >= NV_POLYGON_MAX_VERTICES) {
            break;
        }
        hull[n_out++] = right_hull[i];
    }

    // Reverse winding order
    // TODO: Maybe do this an argument? Because only Nova expects CCW
    for (size_t i = 0; i < n_out / 2; i++) {
        nvVector2 tmp = hull[i];
        hull[i] = hull[n_out - i - 1];
        hull[n_out - i - 1] = tmp;
    }

    NV_FREE(right_hull);
    NV_FREE(left_hull);
    NV_FREE(right);
    NV_FREE(left);

    return n_out;
}


/**
 * @brief Transform info struct that is used to pass body transform to collision functions.
 */
typedef struct {
    nvVector2 position;
    nv_float angle;
} nvTransform;


#endif