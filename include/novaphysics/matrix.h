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
    nvVector2 col1;
    nvVector2 col2;
} nvMat2x2;


/**
 * @brief Initialize new rotation matrix.
 * 
 * @param angle Angle in radians
 * @return nvMat2x2
 */
static inline nvMat2x2 nvMat2x2_from_angle(nv_float angle) {
    nv_float c = nv_cos(angle);
    nv_float s = nv_sin(angle);

    return (nvMat2x2){
        NV_VEC2(c,  s),
        NV_VEC2(-s, c)
    };
}

/**
 * @brief Multiply matrix with a vector.
 * 
 * @param mat Matrix
 * @param vector Vector
 * @return nvVector2 
 */
static inline nvVector2 nvMat2x2_mulv(nvMat2x2 mat, nvVector2 vector) {
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
 * @return nvMat2x2 
 */
static inline nvMat2x2 nvMat2x2_mul(nvMat2x2 a, nvMat2x2 b) {
    return (nvMat2x2){
        nvMat2x2_mulv(a, b.col1),
        nvMat2x2_mulv(a, b.col2)
    };
}

/**
 * @brief Transpose the matrix.
 * 
 * @param mat Matrix
 * @return nvMat2x2 
 */
static inline nvMat2x2 nvMat2x2_transpose(nvMat2x2 mat) {
    return (nvMat2x2){
        NV_VEC2(mat.col1.x, mat.col2.x),
        NV_VEC2(mat.col1.y, mat.col2.y)
    };
}


#endif