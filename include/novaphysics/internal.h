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


/**
 * @file internal.h
 * 
 * Nova Physics utility functions and struct forward declarations
 * to prevent circular includes
 */

struct _nv_Space;


// Utility macro to create objects on HEAP
#define NV_NEW(type) ((type *)malloc(sizeof(type)))


static inline void _nv_error(char *message, char *file, int line) {
    if (message == NULL) message = "\n";

    char errmsg[64];
    sprintf(errmsg, "Nova Physics error in %s, line %d\n", file, line);
    fprintf(stderr, errmsg);
    fprintf(stderr, message);
    exit(1);
}

static inline void _nv_assert(bool condition, char *message, char *file, int line) {
    if (!condition)
        _nv_error(message, file, line);
}

// Hard assertion
#define NV_ASSERT(condition, message) (_nv_assert(condition, message, __FILE__, __LINE__))

// Error
#define NV_ERROR(message) (_nv_error(message, __FILE__, __LINE__))


#endif