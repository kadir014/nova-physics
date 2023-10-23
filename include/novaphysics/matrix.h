/*

  This file is a part of the Nova Physics Engine
  project and distributed under the MIT license.

  Copyright © Kadir Aksoy
  https://github.com/kadir014/nova-physics

*/

#ifndef NOVAPHYSICS_MATRIX_H
#define NOVAPHYSICS_MATRIX_H

#include "novaphysics/internal.h"
#include "novaphysics/vector.h"


/**
 * @file matrix.h
 * 
 * @brief Matrix types and math.
 */


/**
 * @brief 2x2 column-major matrix type.
 */
typedef struct {
    nv_Vector2 col1;
    nv_Vector2 col2;
} nv_Mat2x2;


/**
 * @brief Initialize new rotation matrix.
 * 
 * @param angle Angle in radians
 * @return nv_Mat2x2
 */
static inline nv_Mat2x2 nv_Mat2x2_from_angle(nv_float angle) {
    nv_float c = nv_cos(angle);
    nv_float s = nv_sin(angle);

    return (nv_Mat2x2){
        NV_VEC2(c,  s),
        NV_VEC2(-s, c)
    };
}

/**
 * @brief Multiply matrix with a vector.
 * 
 * @param mat Matrix
 * @param vector Vector
 * @return nv_Vector2 
 */
static inline nv_Vector2 nv_Mat2x2_mulv(nv_Mat2x2 mat, nv_Vector2 vector) {
    return NV_VEC2(
        mat.col1.x * vector.x + mat.col2.x * vector.y,
        mat.col1.y * vector.x + mat.col2.y * vector.y
    );
}

/**
 * @brief Multiply two matrices.
 * 
 * @param a Left-hand matrix
 * @param b Right-hand matrix
 * @return nv_Mat2x2 
 */
static inline nv_Mat2x2 nv_Mat2x2_mul(nv_Mat2x2 a, nv_Mat2x2 b) {
    return (nv_Mat2x2){
        nv_Mat2x2_mulv(a, b.col1),
        nv_Mat2x2_mulv(a, b.col2)
    };
}

/**
 * @brief Transpose the matrix.
 * 
 * @param mat Matrix
 * @return nv_Mat2x2 
 */
static inline nv_Mat2x2 nv_Mat2x2_transpose(nv_Mat2x2 mat) {
    return (nv_Mat2x2){
        NV_VEC2(mat.col1.x, mat.col2.x),
        NV_VEC2(mat.col1.y, mat.col2.y)
    };
}


#endif