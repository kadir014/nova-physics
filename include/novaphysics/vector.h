/*

  This file is a part of the Nova Physics Engine
  project and distributed under the MIT license.

  Copyright © Kadir Aksoy
  https://github.com/kadir014/nova-physics

*/

#ifndef NOVAPHYSICS_VECTOR_H
#define NOVAPHYSICS_VECTOR_H

#include <math.h>
#include <stdlib.h>
#include <stdbool.h>
#include "novaphysics/internal.h"


/**
 * @file vector.h
 * 
 * @details 2D vector type and math
 */


/**
 * @brief 2D vector type
 * 
 * @param x X component of the vector
 * @param y Y component of the vector
 */
typedef struct {
    nv_float x;
    nv_float y;
} nv_Vector2;


/*
    Utility macro to initialize vectors
*/
#define NV_VEC2(x, y) ((nv_Vector2){(x), (y)})

/*
    Utility (not really) macro to initialize and store vectors on HEAP
*/
static inline nv_Vector2 *NV_VEC2_NEW(nv_float x, nv_float y) {
    nv_Vector2 *vector_heap = NV_NEW(nv_Vector2);
    vector_heap->x = x;
    vector_heap->y = y;
    return vector_heap;
}

/*
    Utility macro to cast void * to nv_Vector2
    This is useful for directly passing indexed data from nv_Array
*/
#define NV_TO_VEC2(x) (*(nv_Vector2 *)(x))

/*
    Utility macro to cast void * to nv_Vector2 *
    This is useful for modifying vector element of nv_Array
*/
#define NV_TO_VEC2P(x) ((nv_Vector2 *)(x))


/**
 * Constant zero vector
 */
static const nv_Vector2 nv_Vector2_zero = {0.0, 0.0};


/**
 * @brief Check if two vectors are equal
 * 
 * @param a Left-hand vector
 * @param b Right-hand vector
 * @return bool
 */
static inline bool nv_Vector2_eq(nv_Vector2 a, nv_Vector2 b) {
    return (a.x == b.x && a.y == b.y);
}

/**
 * @brief Add two vectors
 * 
 * @param a Left-hand vector
 * @param b Right-hand vector
 * @return nv_Vector2 
 */
static inline nv_Vector2 nv_Vector2_add(nv_Vector2 a, nv_Vector2 b) {
    return (nv_Vector2){a.x + b.x, a.y + b.y};
}

/**
 * @brief Subtract two vectors
 * 
 * @param a Left-hand vector
 * @param b Right-hand vector
 * @return nv_Vector2 
 */
static inline nv_Vector2 nv_Vector2_sub(nv_Vector2 a, nv_Vector2 b) {
    return (nv_Vector2){a.x - b.x, a.y - b.y};
}

/**
 * @brief Multiply two vectors
 * 
 * @param a Left-hand vector
 * @param b Right-hand vector
 * @return nv_Vector2 
 */
static inline nv_Vector2 nv_Vector2_mulv(nv_Vector2 a, nv_Vector2 b) {
    return (nv_Vector2){a.x * b.x, a.y * b.y};
}

/**
 * @brief Multiply vector by scalar
 * 
 * @param v Vector
 * @param s Scalar
 * @return nv_Vector2 
 */
static inline nv_Vector2 nv_Vector2_muls(nv_Vector2 v, nv_float s) {
    return (nv_Vector2){v.x * s, v.y * s};
}

/**
 * @brief Divide two vectors
 * 
 * @param a Left-hand vector
 * @param b Right-hand vector
 * @return nv_Vector2 
 */
static inline nv_Vector2 nv_Vector2_divv(nv_Vector2 a, nv_Vector2 b) {
    return (nv_Vector2){a.x / b.x, a.y / b.y};
}

/**
 * @brief Divide vector by scalar
 * 
 * @param v Vector
 * @param s Scalar
 * @return nv_Vector2 
 */
static inline nv_Vector2 nv_Vector2_divs(nv_Vector2 v, nv_float s) {
    return (nv_Vector2){v.x / s, v.y / s};
}

/**
 * @brief Negate a vector
 * 
 * @param v Vector to negate
 * @return nv_Vector2 
 */
static inline nv_Vector2 nv_Vector2_neg(nv_Vector2 v) {
    return (nv_Vector2){-v.x, -v.y};
}

/**
 * @brief Rotate vector around the origin
 * 
 * @param v Vector to rotate 
 * @param a Angle in radians
 * @return nv_Vector2 
 */
static inline nv_Vector2 nv_Vector2_rotate(nv_Vector2 v, nv_float a) {
    nv_float c = nv_cos(a);
    nv_float s = nv_sin(a);
    return (nv_Vector2){c * v.x - s * v.y, s * v.x + c * v.y};
}

/**
 * @brief Perpendicular vector (+90 degrees)
 * 
 * @param v Vector
 * @param a Angle in radians
 * @return nv_Vector2 
 */
static inline nv_Vector2 nv_Vector2_perp(nv_Vector2 v) {
    return (nv_Vector2){-v.y, v.x};
}

/**
 * @brief Perpendicular vector (-90 degrees)
 * 
 * @param v Vector
 * @return nv_Vector2 
 */
static inline nv_Vector2 nv_Vector2_perpr(nv_Vector2 v) {
    return (nv_Vector2){v.y, -v.x};
}

/**
 * @brief Calculate squared length (magnitude) of a vector
 * 
 * @param v Vector
 * @return nv_float 
 */
static inline nv_float nv_Vector2_len2(nv_Vector2 v) {
    return v.x * v.x + v.y * v.y;
}

/**
 * @brief Calculate length (magnitude) of a vector
 * 
 * @param v Vector
 * @return nv_float 
 */
static inline nv_float nv_Vector2_len(nv_Vector2 v) {
    return nv_sqrt(nv_Vector2_len2(v));
}

/**
 * @brief Dot product of two vectors
 * 
 * @param a Left-hand vector
 * @param b Right-hand vector 
 * @return nv_float
 */
static inline nv_float nv_Vector2_dot(nv_Vector2 a, nv_Vector2 b) {
    return a.x * b.x + a.y * b.y;
}

/**
 * @brief Z component of cross product of two vectors
 * 
 * @param a Left-hand vector
 * @param b Right-hand vector
 * @return nv_float 
 */
static inline nv_float nv_Vector2_cross(nv_Vector2 a, nv_Vector2 b) {
    return a.x * b.y - a.y * b.x;
}

/**
 * @brief Squared distance from one vector to another
 * 
 * @param a Left-hand vector
 * @param b Right-hand vector
 * @return nv_float 
 */
static inline nv_float nv_Vector2_dist2(nv_Vector2 a, nv_Vector2 b) {
    return (b.x - a.x) * (b.x - a.x) + (b.y - a.y) * (b.y - a.y);
}

/**
 * @brief Distance from one vector to another
 * 
 * @param a Left-hand vector
 * @param b Right-hand vector
 * @return nv_float 
 */
static inline nv_float nv_Vector2_dist(nv_Vector2 a, nv_Vector2 b) {
    return sqrt(nv_Vector2_dist2(a, b));
}

/**
 * @brief Normalize a vector
 * 
 * @param v Vector to normalize
 * @return nv_Vector2 
 */
static inline nv_Vector2 nv_Vector2_normalize(nv_Vector2 v) {
    return nv_Vector2_divs(v, nv_Vector2_len(v));
}


#endif