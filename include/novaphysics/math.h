/*

  This file is a part of the Nova Physics Engine
  project and distributed under the MIT license.

  Copyright © Kadir Aksoy
  https://github.com/kadir014/nova-physics

*/

#ifndef NOVAPHYSICS_MATH_H
#define NOVAPHYSICS_MATH_H

#include "novaphysics/internal.h"
#include "novaphysics/core/array.h"
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
 * @param x First integer
 * @param y Second ineger
 * @return nv_uint64
 */
static inline nv_uint64 nv_u32pair(nv_uint32 x, nv_uint32 y) {
    // https://stackoverflow.com/a/2769598
    return (nv_uint64)x << 32 | y;
}

static inline nv_uint32 nv_u32hash(nv_uint32 x) {
    x = ((x >> 16) ^ x) * 0x45d9f3b;
    x = ((x >> 16) ^ x) * 0x45d9f3b;
    x = (x >> 16) ^ x;
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
    return 0.5 * mass * (radius * radius) + mass * nvVector2_len2(offset);
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

    nv_float area = 0.0;

    size_t j = num_vertices - 1;
    for (size_t i = 0; i < num_vertices; i++) {
        nvVector2 va = vertices[i];
        nvVector2 vb = vertices[j];

        area += (vb.x + va.x) * (vb.y - va.y);
        j = i;
    }

    return nv_fabs(area / 2.0);
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
    nv_float sum1 = 0.0;
    nv_float sum2 = 0.0;

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

    return (mass * sum1) / (6.0 * sum2);
}

/**
 * @brief Calculate centroid of a polygon.
 * 
 * @param vertices Array of vertices of polygon
 * @param num_vertices Number of vertices
 * @return nvVector2
 */
static inline nvVector2 nv_polygon_centroid(
    nvVector2 *vertices,
    size_t num_vertices
) {
    nvVector2 sum = nvVector2_zero;

    for (size_t i = 0; i < num_vertices; i++) {
        sum = nvVector2_add(sum, vertices[i]);
    }

    return nvVector2_div(sum, (nv_float)num_vertices);
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

    if (z < 0.0) return -1;
    else if (z > 0.0) return 1;
    else return 0;
}


// /**
//  * @brief Calculate convex polygon's winding order in the array.
//  * 
//  * Returns 0 if CW, 1 if CCW and -1 if collinear.
//  * 
//  * @param vertices Array of polygon vertices
//  * @return int 
//  */
// static inline int nv_polygon_winding_order(nvArray *vertices) {
//     size_t n = vertices->size;
//     nv_float sum = 0.0;

//     for (size_t i = 0; i < n; i++) {
//         nvVector2 current = NV_TO_VEC2(vertices->data[i]);
//         nvVector2 next = NV_TO_VEC2(vertices->data[(i + 1) % n]);

//         sum += (next.x - current.x) * (next.y + current.y);
//     }

//     if (sum > 0.0) return 0;
//     else if (sum < 0.0) return 1;
//     else return -1;
// }


// static nvVector2 _convex_hull_pivot;

// static int _convex_hull_orientation(nvVector2 p, nvVector2 q, nvVector2 r) {
//     nv_float d = (q.y - p.y) * (r.x - q.x) - (q.x - p.x) * (r.y - q.y);

//     if (d == 0.0) return 0.0; // Collinear
//     return (d > 0.0) ? 1 : 2; // CW or CCW
// }

// static int _convex_hull_cmp(const void *el0, const void *el1) {
//     nvVector2 v0 = NV_TO_VEC2(el0);
//     nvVector2 v1 = NV_TO_VEC2(el1);

//     int o = _convex_hull_orientation(_convex_hull_pivot, v0, v1);

//     if (o == 0) {
//         if (nvVector2_dist2(_convex_hull_pivot, v1) >= nvVector2_dist2(_convex_hull_pivot, v0))
//             return -1;

//         else return 1;
//     }

//     else {
//         if (o == 2) return -1;

//         else return 1;
//     }
// }

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

    // size_t n = points->size;

    // size_t current_min_i = 0;
    // nv_float min_y = NV_TO_VEC2(points->data[current_min_i]).x;
    // nvVector2 pivot;

    // // Find the lowest y-coordinate and leftmost point
    // for (size_t i = 0; i < n; i++) {
    //     nvVector2 v = NV_TO_VEC2(points->data[i]);

    //     if (
    //         v.y < min_y ||
    //         (v.y == min_y && v.x < NV_TO_VEC2(points->data[current_min_i]).x)
    //     ) {
    //         current_min_i = i;
    //         min_y = v.y;
    //     }
    // }

    // // Swap the pivot with the first point
    // nvVector2 *temp = NV_TO_VEC2P(points->data[0]);
    // points->data[0] = points->data[current_min_i];
    // points->data[current_min_i] = temp;

    // pivot = NV_TO_VEC2(points->data[0]);
    // _convex_hull_pivot = pivot;

    // #ifdef NV_COMPILER_MSVC

    //     nvVector2 *tmp_points = (nvVector2 *)NV_MALLOC(sizeof(nvVector2) * points->size);

    // #else

    //     nvVector2 tmp_points[points->size];

    // #endif
    
    // for (size_t i = 0; i < points->size; i++) {
    //     nvVector2 v = NV_TO_VEC2(points->data[i]);
    //     tmp_points[i] = v;
    // }

    // qsort(&tmp_points[1], points->size - 1, sizeof(nvVector2), _convex_hull_cmp);

    // for (size_t i = 0; i < points->size; i++) {
    //     nvVector2 *v = NV_TO_VEC2P(points->data[i]);
    //     v->x = tmp_points[i].x;
    //     v->y = tmp_points[i].y;
    // }

    // #ifdef NV_COMPILER_MSVC

    //     free(tmp_points);

    // #endif

    // nvVector2 *hull = (nvVector2 *)NV_MALLOC(sizeof(nvVector2) * n);
    // size_t hull_size = 3;
    // hull[0] = NV_TO_VEC2(points->data[0]);
    // hull[1] = NV_TO_VEC2(points->data[1]);
    // hull[2] = NV_TO_VEC2(points->data[2]);

    // for (size_t i = 3; i < points->size; i++) {
    //     while (
    //         hull_size > 1 &&
    //         _convex_hull_orientation(
    //             hull[hull_size - 2],
    //             hull[hull_size - 1],
    //             NV_TO_VEC2(points->data[i])
    //         ) != 2
    //     ) {
    //         hull_size--;
    //     }

    //     hull[hull_size++] = NV_TO_VEC2(points->data[i]);
    // }

    // nvArray *ret_hull = nvArray_new();
    // for (size_t i = 0; i < hull_size; i++) {
    //     nvArray_add(ret_hull, NV_VECTOR2_NEW(hull[i].x, hull[i].y));
    // }

    // free(hull);

    // return ret_hull;
    return NULL;
}


/**
 * @brief Transform info struct that is used to pass body transform to collision functions.
 */
typedef struct {
    nvVector2 position;
    nv_float angle;
} nvTransform;


#endif