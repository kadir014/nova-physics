/*

  This file is a part of the Nova Physics Engine
  project and distributed under the MIT license.

  Copyright © Kadir Aksoy
  https://github.com/kadir014/Nova-Physics

*/

#ifndef NOVAPHYSICS_VECTOR_H
#define NOVAPHYSICS_VECTOR_H

#include <math.h>
#include <stdlib.h>
#include <stdbool.h>


/**
 * vector.h
 * 
 * 2D vector type and vector math
 */


/**
 * @brief 2D vector type
 * 
 * @param x X component of the vector
 * @param y Y component of the vector
 */
typedef struct {
    double x;
    double y;
} nv_Vector2;


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
static inline nv_Vector2 nv_Vector2_muls(nv_Vector2 v, double s) {
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
static inline nv_Vector2 nv_Vector2_divs(nv_Vector2 v, double s) {
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
 * @param v Vector to rottate 
 * @param a Angle in radians
 * @return nv_Vector2 
 */
static inline nv_Vector2 nv_Vector2_rotate(nv_Vector2 v, double a) {
    double c = cos(a);
    double s = sin(a);
    return (nv_Vector2){c * v.x - s * v.y, s * v.x + c * v.y};
}

/**
 * @brief Rotate vector around the origin
 * 
 * @param v Vector to rottate 
 * @param a Angle in radians
 * @return nv_Vector2 
 */
static inline nv_Vector2 nv_Vector2_perp(nv_Vector2 v) {
    return (nv_Vector2){-v.y, v.x};
}

/**
 * @brief Rotate vector to perpendeciular eaxisdgsh
 * 
 * @param v Vector to rotate
 * @return nv_Vector2 
 */
static inline nv_Vector2 nv_Vector2_perpr(nv_Vector2 v) {
    return (nv_Vector2){v.y, -v.x};
}

/**
 * @brief Calculate squared length (magnitude) of a vector
 * 
 * @param v 
 * @return double 
 */
static inline double nv_Vector2_len2(nv_Vector2 v) {
    return v.x * v.x + v.y * v.y;
}

/**
 * @brief Calculate length (magnitude) of a vector
 * 
 * @param v 
 * @return double 
 */
static inline double nv_Vector2_len(nv_Vector2 v) {
    return sqrt(nv_Vector2_len2(v));
}

/**
 * @brief Dot product of two vectors
 * 
 * @param a Left-hand vector
 * @param b Right-hand vector 
 * @return double
 */
static inline double nv_Vector2_dot(nv_Vector2 a, nv_Vector2 b) {
    return a.x * b.x + a.y * b.y;
}

/**
 * @brief Z component of cross product of two vectors
 * 
 * @param a Left-hand vector
 * @param b Right-hand vector
 * @return double 
 */
static inline double nv_Vector2_cross(nv_Vector2 a, nv_Vector2 b) {
    return a.x * b.y - a.y * b.x;
}

/**
 * @brief Squared distance from one vector to another
 * 
 * @param a Left-hand vector
 * @param b Right-hand vector
 * @return double 
 */
static inline double nv_Vector2_dist2(nv_Vector2 a, nv_Vector2 b) {
    return (b.x - a.x) * (b.x - a.x) + (b.y - a.y) * (b.y - a.y);
}

/**
 * @brief Distance from one vector to another
 * 
 * @param a Left-hand vector
 * @param b Right-hand vector
 * @return double 
 */
static inline double nv_Vector2_dist(nv_Vector2 a, nv_Vector2 b) {
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


/**
 * @brief Array of nv_Vector2 objects
 * 
 * @param size Size of the array
 * @param data Pointer to vectors
 */
typedef struct {
    size_t size;
    nv_Vector2 *data;
} nv_Vector2Array;

/**
 * @brief Create a new nv_Vector2 array
 * 
 * @return nv_Vector2Array * 
 */
static nv_Vector2Array *nv_Vector2Array_new() {
    nv_Vector2Array *array = (nv_Vector2Array *)malloc(sizeof(nv_Vector2Array));

    array->size = 0;

    array->data = (nv_Vector2 *)malloc(sizeof(nv_Vector2));

    return array;
}

/**
 * @brief Free nv_Vector2 array
 * 
 * @param array Array to free
 */
static void nv_Vector2Array_free(nv_Vector2Array *array) {
    free(array->data);
    array->data = NULL;
    array->size = 0;
    free(array);
}

/**
 * @brief Add a new nv_Vector2 to the array
 * 
 * @param array Array to append to
 * @param vector Vector to append
 */
static void nv_Vector2Array_add(nv_Vector2Array *array, nv_Vector2 vector) {
    array->size += 1;

    array->data = (nv_Vector2 *)realloc(array->data, array->size * sizeof(nv_Vector2));

    array->data[array->size - 1] = vector;
}


#endif