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
#include <immintrin.h>


/**
 * @file internal.h
 * 
 * @brief Nova Physics internal type definitions, utility functions
 *        and forward declarations.
 */


/**
 * Nova Physics floating type
 * 
 * Double precision float is used as default for higher accuracy
 * But the developer can define NV_USE_FLOAT globally for the project to use
 * single precision float.
 * This can be simply done by passing -f or --float to Nova Physics's build system.
 */

#ifdef NV_USE_FLOAT

    typedef float nv_float;

    #define nv_fabs fabsf
    #define nv_fmin fminf
    #define nv_fmax fmaxf
    #define nv_pow powf
    #define nv_exp expf
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
    #define nv_exp exp
    #define nv_sqrt sqrt
    #define nv_sin sin
    #define nv_cos cos
    #define nv_floor floor

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
    Platform and compiler detection.
*/

#if defined(_WIN32) || defined(__WIN32__) || defined(__WINDOWS__)
    #define NV_WINDOWS
#endif

// Does Clang also use GCC warning pragmas?
#if defined(__GNUC__) || defined(__MINGW32__) || defined(__MINGW64__)
    #define NV_COMPILER_GCC
#endif


/*
    SIMD detection and utility functions.
*/

#ifdef __AVX__

    #define NV_AVX

    #define NV_AVX_VECTOR_FROM_FLOAT(x) _mm256_set_ps(x, x, x, x, x, x, x, x)
    #define NV_AVX_VECTOR_FROM_DOUBLE(x) _mm256_set_pd(x, x, x, x)

#endif

#ifdef __AVX2__

    #define NV_AVX2

#endif


// This is forward declared to prevent circular includes
struct nv_Space;


// Utility macro to create objects on HEAP
#define NV_NEW(type) ((type *)malloc(sizeof(type)))


/**
 * Internal error function.
 */
#ifdef NV_COMPILER_GCC
    #pragma GCC diagnostic push
    #pragma GCC diagnostic ignored "-Wformat-security"
    #pragma GCC diagnostic ignored "-Wformat-overflow"
#endif

static inline void _nv_error(char *message, char *file, int line) {
    if (message == NULL) message = "\n";

    char errmsg[64];
    sprintf(errmsg, "Nova Physics error in %s, line %d\n", file, line);
    fprintf(stderr, errmsg);
    fprintf(stderr, message);
    exit(1);
}

#ifdef NV_COMPILER_GCC
    #pragma GCC diagnostic pop
#endif

/**
 * Internal assert function.
*/
static inline void _nv_assert(bool condition, char *message, char *file, int line) {
    if (!condition)
        _nv_error(message, file, line);
}

/**
 * @brief Assert the condition and exit if needed.
 * 
 * @param condition Condition bool
 * @param message Error message
 */
#define NV_ASSERT(condition, message) (_nv_assert(condition, message, __FILE__, __LINE__))

/**
 * @brief Raise error and exit.
 * 
 * @param message Error message
 */
#define NV_ERROR(message) (_nv_error(message, __FILE__, __LINE__))


/*
    Internal Tracy Profiler macros.
*/
#ifdef TRACY_ENABLE

    #include "../../src/tracy/TracyC.h"

    #define NV_TRACY_ZONE_START TracyCZone(_tracy_zone, true)
    #define NV_TRACY_ZONE_END TracyCZoneEnd(_tracy_zone)
    #define NV_TRACY_FRAMEMARK TracyCFrameMark

#else

    #define NV_TRACY_ZONE_START
    #define NV_TRACY_ZONE_END
    #define NV_TRACY_FRAMEMARK

#endif


#endif