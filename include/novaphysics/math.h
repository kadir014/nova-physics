/*

  This file is a part of the Nova Physics Engine
  project and distributed under the MIT license.

  Copyright © Kadir Aksoy
  https://github.com/kadir014/nova-physics

*/

#ifndef NOVAPHYSICS_MATH_H
#define NOVAPHYSICS_MATH_H

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
 * @brief Combine two 32-bit unsigned integers into unsigned 64-bit one.
 * 
 * @param x First integer
 * @param y Second ineger
 * @return nv_uint64
 */
static inline nv_uint64 nv_u32pair(nv_uint32 x, nv_uint32 y) {
    // https://stackoverflow.com/a/2769598
    return (nv_uint64)x << 32 | y;
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

        1   1   (r⊥ᴬᴾ · n)²   (r⊥ᴮᴾ · n)²
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
static inline nv_float nv_polygon_area(nvArray *vertices) {
    nv_float area = 0.0;
    size_t n = vertices->size;

    size_t j = n - 1;
    for (size_t i = 0; i < n; i++) {
        nvVector2 va = NV_TO_VEC2(vertices->data[i]);
        nvVector2 vb = NV_TO_VEC2(vertices->data[j]);

        area += (vb.x + va.x) * (vb.y - va.y);
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
static inline nv_float nv_polygon_inertia(nv_float mass, nvArray *vertices) {
    nv_float sum1 = 0.0;
    nv_float sum2 = 0.0;
    size_t n = vertices->size;

    for (size_t i = 0; i < n; i++) {
        nvVector2 v1 = NV_TO_VEC2(vertices->data[i]);
        nvVector2 v2 = NV_TO_VEC2(vertices->data[(i + 1) % n]);

        nv_float a = nvVector2_cross(v2, v1);
        nv_float b = nvVector2_dot(v1, v1) +
                   nvVector2_dot(v1, v2) +
                   nvVector2_dot(v2, v2);
        
        sum1 += a * b;
        sum2 += a;
    }

    return (mass * sum1) / (6.0 * sum2);
}

/**
 * @brief Calculate centroid of a polygon.
 * 
 * @param vertices Array of vertices of polygon
 * @return nvVector2
 */
static inline nvVector2 nv_polygon_centroid(nvArray *vertices) {
    nvVector2 sum = nvVector2_zero;
    size_t n = vertices->size;

    for (size_t i = 0; i < n; i++) {
        sum = nvVector2_add(sum, NV_TO_VEC2(vertices->data[i]));
    }

    return nvVector2_div(sum, (nv_float)n);
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
    nvVector2 center,
    nv_float radius,
    nvVector2 axis,
    nv_float *min_out,
    nv_float *max_out
) {
    nvVector2 a = nvVector2_mul(nvVector2_normalize(axis), radius);

    nvVector2 p1 = nvVector2_add(center, a);
    nvVector2 p2 = nvVector2_sub(center, a);

    nv_float min = nvVector2_dot(p1, axis);
    nv_float max = nvVector2_dot(p2, axis);

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
    nvArray *vertices,
    nvVector2 axis,
    nv_float *min_out,
    nv_float *max_out
) {
    nv_float min = NV_INF;
    nv_float max = -NV_INF;

    for (size_t i = 0; i < vertices->size; i++) {
        nv_float projection = nvVector2_dot(NV_TO_VEC2(vertices->data[i]), axis);
        
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
 * @return nvVector2
 */
static inline nvVector2 nv_polygon_support(nvArray *vertices, nvVector2 axis) {
    nv_float best_proj = -NV_INF;
    nvVector2 best_vertex;

    for (size_t i = 0; i < vertices->size; i++) {
        nvVector2 v = NV_TO_VEC2(vertices->data[i]);
        nv_float proj = nvVector2_dot(v, axis);

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
    nvVector2 center,
    nvVector2 a,
    nvVector2 b,
    nv_float *dist_out,
    nvVector2 *contact_out
) {
    nvVector2 ab = nvVector2_sub(b, a);
    nvVector2 ap = nvVector2_sub(center, a);

    nv_float projection = nvVector2_dot(ap, ab);
    nv_float ab_len = nvVector2_len2(ab);
    nv_float dist = projection / ab_len;
    nvVector2 contact;

    if (dist <= 0.0) contact = a;

    else if (dist >= 1.0) contact = b;

    else contact = nvVector2_add(a, nvVector2_mul(ab, dist));

    *dist_out = nvVector2_dist2(center, contact);
    *contact_out = contact;
}


/**
 * @brief Find closest vertex of the polygon to the circle.

 * @param center Center of the circle
 * @param vertices Vertices of the polygon
 * @return nvVector2 
 */
static inline nvVector2 nv_polygon_closest_vertex_to_circle(
    nvVector2 center,
    nvArray *vertices
) {
    size_t closest = 0;
    nv_float min_dist = NV_INF;
    nv_bool found = false;
    
    for (size_t i = 0; i < vertices->size; i++) {
        nv_float dist = nvVector2_dist2(NV_TO_VEC2(vertices->data[i]), center);

        if (dist < min_dist) {
            min_dist = dist;
            closest = i;
            found = true;
        }
    }

    NV_ASSERT(found, "");

    return NV_TO_VEC2(vertices->data[closest]);
}


/**
 * @brief Calculate convex polygon's winding order in the array.
 * 
 * Returns 0 if CW, 1 if CCW and -1 if collinear.
 * 
 * @param vertices Array of polygon vertices
 * @return int 
 */
static inline int nv_polygon_winding_order(nvArray *vertices) {
    size_t n = vertices->size;
    nv_float sum = 0.0;

    for (size_t i = 0; i < n; i++) {
        nvVector2 current = NV_TO_VEC2(vertices->data[i]);
        nvVector2 next = NV_TO_VEC2(vertices->data[(i + 1) % n]);

        sum += (next.x - current.x) * (next.y + current.y);
    }

    if (sum > 0.0) return 0;
    else if (sum < 0.0) return 1;
    else return -1;
}


static nvVector2 _convex_hull_pivot;

static int _convex_hull_orientation(nvVector2 p, nvVector2 q, nvVector2 r) {
    nv_float d = (q.y - p.y) * (r.x - q.x) - (q.x - p.x) * (r.y - q.y);

    if (d == 0.0) return 0.0; // Collinear
    return (d > 0.0) ? 1 : 2; // CW or CCW
}

static int _convex_hull_cmp(const void *el0, const void *el1) {
    nvVector2 v0 = NV_TO_VEC2(el0);
    nvVector2 v1 = NV_TO_VEC2(el1);

    int o = _convex_hull_orientation(_convex_hull_pivot, v0, v1);

    if (o == 0) {
        if (nvVector2_dist2(_convex_hull_pivot, v1) >= nvVector2_dist2(_convex_hull_pivot, v0))
            return -1;

        else return 1;
    }

    else {
        if (o == 2) return -1;

        else return 1;
    }
}

/**
 * @brief Generate a convex hull around the given points.
 * 
 * This function returns a new allocated array. Passed in array can be freed.
 * 
 * @param points Array of vectors
 * @return nvArray *
 */
static inline nvArray *nv_generate_convex_hull(nvArray *points) {
    // This function implements the Graham Scan algorithm
    // https://en.wikipedia.org/wiki/Graham_scan

    size_t n = points->size;

    size_t current_min_i = 0;
    nv_float min_y = NV_TO_VEC2(points->data[current_min_i]).x;
    nvVector2 pivot;

    // Find the lowest y-coordinate and leftmost point
    for (size_t i = 0; i < n; i++) {
        nvVector2 v = NV_TO_VEC2(points->data[i]);

        if (
            v.y < min_y ||
            (v.y == min_y && v.x < NV_TO_VEC2(points->data[current_min_i]).x)
        ) {
            current_min_i = i;
            min_y = v.y;
        }
    }

    // Swap the pivot with the first point
    nvVector2 *temp = NV_TO_VEC2P(points->data[0]);
    points->data[0] = points->data[current_min_i];
    points->data[current_min_i] = temp;

    pivot = NV_TO_VEC2(points->data[0]);
    _convex_hull_pivot = pivot;

    #ifdef NV_COMPILER_MSVC

        nvVector2 *tmp_points = (nvVector2 *)malloc(sizeof(nvVector2) * points->size);

    #else

        nvVector2 tmp_points[points->size];

    #endif
    
    for (size_t i = 0; i < points->size; i++) {
        nvVector2 v = NV_TO_VEC2(points->data[i]);
        tmp_points[i] = v;
    }

    qsort(&tmp_points[1], points->size - 1, sizeof(nvVector2), _convex_hull_cmp);

    for (size_t i = 0; i < points->size; i++) {
        nvVector2 *v = NV_TO_VEC2P(points->data[i]);
        v->x = tmp_points[i].x;
        v->y = tmp_points[i].y;
    }

    #ifdef NV_COMPILER_MSVC

        free(tmp_points);

    #endif

    nvVector2 *hull = (nvVector2 *)malloc(sizeof(nvVector2) * n);
    size_t hull_size = 3;
    hull[0] = NV_TO_VEC2(points->data[0]);
    hull[1] = NV_TO_VEC2(points->data[1]);
    hull[2] = NV_TO_VEC2(points->data[2]);

    for (size_t i = 3; i < points->size; i++) {
        while (
            hull_size > 1 &&
            _convex_hull_orientation(
                hull[hull_size - 2],
                hull[hull_size - 1],
                NV_TO_VEC2(points->data[i])
            ) != 2
        ) {
            hull_size--;
        }

        hull[hull_size++] = NV_TO_VEC2(points->data[i]);
    }

    nvArray *ret_hull = nvArray_new();
    for (size_t i = 0; i < hull_size; i++) {
        nvArray_add(ret_hull, NV_VEC2_NEW(hull[i].x, hull[i].y));
    }

    free(hull);

    return ret_hull;
}


/**
 * @brief Transform info struct that is used to pass body transform to collision functions.
 */
typedef struct {
    nvVector2 position;
    nv_float angle;
} nvTransform;


#endif