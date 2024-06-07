/*

  This file is a part of the Nova Physics Engine
  project and distributed under the MIT license.

  Copyright © Kadir Aksoy
  https://github.com/kadir014/nova-physics

*/

#ifndef NOVAPHYSICS_VECTOR_H
#define NOVAPHYSICS_VECTOR_H

#include "novaphysics/internal.h"


/**
 * @file vector.h
 * 
 * @brief 2D vector type and math.
 */


/**
 * @brief 2D vector type.
 */
typedef struct {
    nv_float x; /**< X component of the vector. */
    nv_float y; /**< Y component of the vector. */
} nvVector2;


/**
 * @brief Initialize nvVector2 literal. 
 * 
 * @param x X component
 * @param y Y component
 * @return nvVector2
*/
#define NV_VECTOR2(x, y) ((nvVector2){(x), (y)})


/**
 * @brief Constant zero vector.
 */
static const nvVector2 nvVector2_zero = {0.0, 0.0};


/**
 * @brief Check if two vectors are equal.
 * 
 * @param a Left-hand vector
 * @param b Right-hand vector
 * @return nv_bool
 */
static inline nv_bool nvVector2_eq(nvVector2 a, nvVector2 b) {
    return (a.x == b.x && a.y == b.y);
}

/**
 * @brief Add two vectors.
 * 
 * @param a Left-hand vector
 * @param b Right-hand vector
 * @return nvVector2 
 */
static inline nvVector2 nvVector2_add(nvVector2 a, nvVector2 b) {
    return NV_VECTOR2(a.x + b.x, a.y + b.y);
}

/**
 * @brief Subtract two vectors.
 * 
 * @param a Left-hand vector
 * @param b Right-hand vector
 * @return nvVector2 
 */
static inline nvVector2 nvVector2_sub(nvVector2 a, nvVector2 b) {
    return NV_VECTOR2(a.x - b.x, a.y - b.y);
}

/**
 * @brief Multiply vector by scalar.
 * 
 * @param v Vector
 * @param s Scalar
 * @return nvVector2 
 */
static inline nvVector2 nvVector2_mul(nvVector2 v, nv_float s) {
    return NV_VECTOR2(v.x * s, v.y * s);
}

/**
 * @brief Divide vector by scalar.
 * 
 * @param v Vector
 * @param s Scalar
 * @return nvVector2 
 */
static inline nvVector2 nvVector2_div(nvVector2 v, nv_float s) {
    return NV_VECTOR2(v.x / s, v.y / s);
}

/**
 * @brief Negate a vector.
 * 
 * @param v Vector to negate
 * @return nvVector2 
 */
static inline nvVector2 nvVector2_neg(nvVector2 v) {
    return NV_VECTOR2(-v.x, -v.y);
}

/**
 * @brief Rotate vector around the origin.
 * 
 * @param v Vector to rotate 
 * @param a Angle in radians
 * @return nvVector2 
 */
static inline nvVector2 nvVector2_rotate(nvVector2 v, nv_float a) {
    nv_float c = nv_cos(a);
    nv_float s = nv_sin(a);
    return NV_VECTOR2(c * v.x - s * v.y, s * v.x + c * v.y);
}

/**
 * @brief Perpendicular vector (+90 degrees).
 * 
 * @param v Vector
 * @param a Angle in radians
 * @return nvVector2 
 */
static inline nvVector2 nvVector2_perp(nvVector2 v) {
    return NV_VECTOR2(-v.y, v.x);
}

/**
 * @brief Perpendicular vector (-90 degrees).
 * 
 * @param v Vector
 * @return nvVector2 
 */
static inline nvVector2 nvVector2_perpr(nvVector2 v) {
    return NV_VECTOR2(v.y, -v.x);
}

/**
 * @brief Calculate squared length (magnitude) of a vector.
 * 
 * @param v Vector
 * @return nv_float 
 */
static inline nv_float nvVector2_len2(nvVector2 v) {
    return v.x * v.x + v.y * v.y;
}

/**
 * @brief Calculate length (magnitude) of a vector.
 * 
 * @param v Vector
 * @return nv_float 
 */
static inline nv_float nvVector2_len(nvVector2 v) {
    return nv_sqrt(nvVector2_len2(v));
}

/**
 * @brief Dot product of two vectors.
 * 
 * @param a Left-hand vector
 * @param b Right-hand vector 
 * @return nv_float
 */
static inline nv_float nvVector2_dot(nvVector2 a, nvVector2 b) {
    return a.x * b.x + a.y * b.y;
}

/**
 * @brief Z component of cross product of two vectors.
 * 
 * @param a Left-hand vector
 * @param b Right-hand vector
 * @return nv_float 
 */
static inline nv_float nvVector2_cross(nvVector2 a, nvVector2 b) {
    return a.x * b.y - a.y * b.x;
}

/**
 * @brief Squared distance from one vector to another.
 * 
 * @param a Left-hand vector
 * @param b Right-hand vector
 * @return nv_float 
 */
static inline nv_float nvVector2_dist2(nvVector2 a, nvVector2 b) {
    return (b.x - a.x) * (b.x - a.x) + (b.y - a.y) * (b.y - a.y);
}

/**
 * @brief Distance from one vector to another.
 * 
 * @param a Left-hand vector
 * @param b Right-hand vector
 * @return nv_float 
 */
static inline nv_float nvVector2_dist(nvVector2 a, nvVector2 b) {
    return nv_sqrt(nvVector2_dist2(a, b));
}

/**
 * @brief Normalize a vector.
 * 
 * @param v Vector to normalize
 * @return nvVector2 
 */
static inline nvVector2 nvVector2_normalize(nvVector2 v) {
    return nvVector2_div(v, nvVector2_len(v));
}

/**
 * @brief Lerp between two vectors.
 * 
 * @param a First vector
 * @param b Second vector
 * @param t Interpolation amount [0, 1]
 * @return nvVector2 
 */
static inline nvVector2 nvVector2_lerp(nvVector2 a, nvVector2 b, nv_float t) {
    return NV_VECTOR2((1.0 - t) * a.x + t * b.x, (1.0 - t) * a.y + t * b.y);
}


#endif