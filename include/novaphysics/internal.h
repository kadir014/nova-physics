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
#include <math.h>


/**
 * @file internal.h
 * 
 * Nova Physics type definitions, utility functions and forward declarations
 */


/**
 * Nova Physics floating type
 * 
 * Double is used as default for higher precision. But developer can
 * define NV_USE_FLOAT before including Nova Physics to use 32-bit floats.
 */
#ifdef NV_USE_FLOAT

    typedef float nv_float;

    #define nv_fabs fabsf
    #define nv_fmin fminf
    #define nv_fmax fmaxf
    #define nv_pow powf
    #define nv_sqrt sqrtf

#else

    typedef double nv_float;

    #define nv_fabs fabs
    #define nv_fmin fmin
    #define nv_fmax fmax
    #define nv_pow pow
    #define nv_sqrt sqrt

#endif


/**
 * Nova Physics boolean type
 */
typedef unsigned char nv_bool;
#define nv_true 1
#define nv_false 0


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