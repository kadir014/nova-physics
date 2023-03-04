/*

  This file is a part of the Nova Physics Engine
  project and distributed under the MIT license.

  Copyright © Kadir Aksoy
  https://github.com/kadir014/nova-physics

*/

#ifndef NOVAPHYSICS_INTERNAL_H
#define NOVAPHYSICS_INTERNAL_H

#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <math.h>


/**
 * @file internal.h
 * 
 * @details Nova Physics type definitions, utility functions
 *          and forward declarations
 */


/**
 * Nova Physics floating type
 * 
 * Double is used as default for higher precision. But developer can
 * either use the Nova Physics Engine's own build system to change
 * floating point type (with option -f or --float), or pass -DNV_USE_FLOAT
 * to compiler if building from scratch (which should not be the case)
 */

#ifdef NV_USE_FLOAT

    typedef float nv_float;

    #define nv_fabs fabsf
    #define nv_fmin fminf
    #define nv_fmax fmaxf
    #define nv_pow powf
    #define nv_sqrt sqrtf
    #define nv_sin sinf
    #define nv_cos cosf
    #define nv_floor floorf

#else

    typedef double nv_float;

    #define nv_fabs fabs
    #define nv_fmin fmin
    #define nv_fmax fmax
    #define nv_pow pow
    #define nv_sqrt sqrt
    #define nv_sin sin
    #define nv_cos cos
    #define nv_floor floor

#endif


/*
    Nova Physics integer types
*/
typedef int8_t nv_int8;
typedef int16_t nv_int16;
typedef int32_t nv_int32;
typedef int64_t nv_int64;
typedef uint8_t nv_uint8;
typedef uint16_t nv_uint16;
typedef uint32_t nv_uint32;
typedef uint64_t nv_uint64;


// This is forward declared to prevent circular includes
struct _nv_Space;


// Utility macro to create objects on HEAP
#define NV_NEW(type) ((type *)malloc(sizeof(type)))


/**
 * Internal error function
 */
static inline void _nv_error(char *message, char *file, int line) {
    if (message == NULL) message = "\n";

    char errmsg[64];
    sprintf(errmsg, "Nova Physics error in %s, line %d\n", file, line);
    fprintf(stderr, errmsg);
    fprintf(stderr, message);
    exit(1);
}

/**
 * Internal assert function
*/
static inline void _nv_assert(bool condition, char *message, char *file, int line) {
    if (!condition)
        _nv_error(message, file, line);
}

/**
 * @brief Assert the condition and exit if needed
 * 
 * @param condition Condition bool
 * @param message Error message
 */
#define NV_ASSERT(condition, message) (_nv_assert(condition, message, __FILE__, __LINE__))

/**
 * @brief Raise error and exit
 * 
 * @param message Error message
 */
#define NV_ERROR(message) (_nv_error(message, __FILE__, __LINE__))


#endif