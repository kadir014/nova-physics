/*

  This file is a part of the Nova Physics Engine
  project and distributed under the MIT license.

  Copyright © Kadir Aksoy
  https://github.com/kadir014/nova-physics

*/

#ifndef NOVAPHYSICS_TYPES_H
#define NOVAPHYSICS_TYPES_H

#include <stdint.h>
#include <math.h>


/**
 * @file types.h
 * 
 * @brief Nova Physics type definitions.
 */


/*
    Nova Physics floating-point type.

    Double-precision mode can be used for bigger worlds and higher accuracy.
    But it might be slower depending on the arch since all operations will
    be done with doubles.

    See build instructions on how to enable double precision and other options.
*/

#ifdef NV_USE_DOUBLE_PRECISION

    typedef double nv_float;

    #define nv_fabs fabs
    #define nv_fmin fmin
    #define nv_fmax fmax
    #define nv_pow pow
    #define nv_exp exp
    #define nv_sqrt sqrt
    #define nv_sin sin
    #define nv_cos cos
    #define nv_atan2 atan2
    #define nv_floor floor

#else

    typedef float nv_float;

    #define nv_fabs fabsf
    #define nv_fmin fminf
    #define nv_fmax fmaxf
    #define nv_pow powf
    #define nv_exp expf
    #define nv_sqrt sqrtf
    #define nv_sin sinf
    #define nv_cos cosf
    #define nv_atan2 atan2f
    #define nv_floor floorf

#endif


/*
    Nova Physics integer types.
*/

typedef int8_t nv_int8;
typedef int16_t nv_int16;
typedef int32_t nv_int32;
typedef int64_t nv_int64;
typedef uint8_t nv_uint8;
typedef uint16_t nv_uint16;
typedef uint32_t nv_uint32;
typedef uint64_t nv_uint64;


/*
    Nova Physics boolean type.
*/

typedef int nv_bool;
#define true 1
#define false 0


#endif