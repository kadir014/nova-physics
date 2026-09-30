/*

  This file is a part of the Nova Physics Engine
  project and distributed under the MIT license.

  Copyright © Kadir Aksoy
  https://github.com/kadir014/nova-physics

*/

#ifndef NOVAPHYSICS_INTERNAL_ALLOC_H
#define NOVAPHYSICS_INTERNAL_ALLOC_H

#include <stdlib.h>


/**
 * @file internal_alloc.h
 * 
 * @brief Nova Physics internal debugging allocations.
 */


/**
 * @brief Report current memory leaks.
 * 
 * Call this function at the end of your application to get a brief report
 * on the tracked memory allocations so far.
 */
void nv_check_leaks();


void *_nv_malloc(size_t size, const char *file, unsigned int line);

void *_nv_calloc(size_t count, size_t size, const char *file, unsigned int line);

void *_nv_realloc(void *ptr, size_t new_size, const char *file, unsigned int line);

void _nv_free(void *ptr, const char *file, unsigned int line);


#endif