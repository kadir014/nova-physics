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


/**
 * @file math.h
 * 
 * @details Nova physics math utilities
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
 * @brief Check if two values are close enough to count as equal
 * 
 * @param a Left-hand value
 * @param b Right-hand value
 * @return bool
 */
bool nv_nearly_eq(nv_float a, nv_float b);

/**
 * @overload Check if two vectors are close enough to count as equal
 * 
 * @param a Left-hand vector
 * @param b Right-hand vector
 * @return bool
 */
bool nv_nearly_eqv(nv_Vector2 a, nv_Vector2 b);


static inline bool nv_bias_greater_than(nv_float a, nv_float b) {
    // TODO Change this function (Box2D's bias function or look at issues)
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
nv_Vector2 nv_calc_relative_velocity(
    nv_Vector2 linear_velocity_a,
    nv_float angular_velocity_a,
    nv_Vector2 ra,
    nv_Vector2 linear_velocity_b,
    nv_float angular_velocity_b,
    nv_Vector2 rb
);

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
nv_float nv_calc_mass_k(
    nv_Vector2 normal,
    nv_Vector2 ra,
    nv_Vector2 rb,
    nv_float invmass_a,
    nv_float invmass_b,
    nv_float invinertia_a,
    nv_float invinertia_b
);


/**
 * @brief Calculate area of a circle (πr²)
 * 
 * @param radius Radius of the circle
 * @return nv_float 
 */
nv_float nv_circle_area(nv_float radius);

/**
 * @brief Calculate moment of inertia of a circle (1/2 mr²)
 * 
 * @param mass Mass of the circles
 * @param radius Radius of the circle
 * @return nv_float 
 */
nv_float nv_circle_inertia(nv_float mass, nv_float radius);

/**
 * @brief Calculate area of a polygon (Shoelace formula)
 * 
 * @param vertices Array of vertices of polygon
 * @return nv_float 
 */
nv_float nv_polygon_area(nv_Array *vertices);

/**
 * @brief Calculate moment of inertia of a polygon
 * 
 * @param mass Mass of the polygon
 * @param vertices Array of vertices of polygon
 * @return nv_float 
 */
nv_float nv_polygon_inertia(nv_float mass, nv_Array *vertices);

/**
 * @brief Calculate centroid of a polygon
 * 
 * @param vertices Array of vertices of polygon
 * @return nv_Vector2
 */
nv_Vector2 nv_polygon_centroid(nv_Array *vertices);


/**
 * @brief Project circle onto axis and return min & max points
 * 
 * @param center Center of circle
 * @param radius Radius of circle
 * @param axis Axis vector to project on
 * @param min_out Pointer for out min value
 * @param max_out Pointer for out max value
 */
void nv_project_circle(
    nv_Vector2 center,
    nv_float radius,
    nv_Vector2 axis,
    nv_float *min_out,
    nv_float *max_out
);

/**
 * @brief Project polygon onto axis and return min & max points
 * 
 * @param vertices Vertices of the polygon
 * @param axis Axis vector to project on
 * @param min_out Pointer for out min value
 * @param max_out Pointer for out max value
 */
void nv_project_polyon(
    nv_Array *vertices,
    nv_Vector2 axis,
    nv_float *min_out,
    nv_float *max_out
);


/**
 * @brief Perp distance between point and line segment
 * 
 * @param center
 * @param a 
 * @param b 
 * @param dist_out 
 * @param edge_out 
 */
void nv_point_segment_dist(
    nv_Vector2 center,
    nv_Vector2 a,
    nv_Vector2 b,
    nv_float *dist_out,
    nv_Vector2 *edge_out
);


/**
 * @brief Find closest vertex of the polygon to the circle

 * @param center Center of the circle
 * @param vertices Vertices of the polygon
 * @return nv_Vector2 
 */
nv_Vector2 nv_polygon_closest_vertex_to_circle(
    nv_Vector2 center,
    nv_Array *vertices
);


typedef struct {
    nv_Vector2 col1;
    nv_Vector2 col2;
} nv_Mat22;

static inline nv_Mat22 nv_Mat22_from_angle(nv_float angle) {
    nv_float c = nv_cos(angle);
    nv_float s = nv_sin(angle);

    return (nv_Mat22){
        NV_VEC2(c,  s),
        NV_VEC2(-s, c)
    };
}

static inline nv_Vector2 nv_Mat22_mulv(nv_Mat22 mat, nv_Vector2 vector) {
    return NV_VEC2(
        mat.col1.x * vector.x + mat.col2.x * vector.y,
        mat.col1.y * vector.x + mat.col2.y * vector.y
    );
}

static inline nv_Mat22 nv_Mat22_mul(nv_Mat22 a, nv_Mat22 b) {
    return (nv_Mat22){
        nv_Mat22_mulv(a, b.col1),
        nv_Mat22_mulv(a, b.col2)
    };
}

static inline nv_Mat22 nv_Mat22_transpose(nv_Mat22 mat) {
    return (nv_Mat22){
        NV_VEC2(mat.col1.x, mat.col2.x),
        NV_VEC2(mat.col1.y, mat.col2.y)
    };
}


#endif