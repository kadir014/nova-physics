/*

  This file is a part of the Nova Physics Engine
  project and distributed under the MIT license.

  Copyright © Kadir Aksoy
  https://github.com/kadir014/nova-physics

*/

#ifndef NOVAPHYSICS_ARRAY_H
#define NOVAPHYSICS_ARRAY_H

#include "novaphysics/internal.h"


/**
 * @file core/array.h
 * 
 * @brief Type-generic dynamically growing array implementation.
 */


/**
 * @brief Type-generic dynamically growing array implementation.
 */
typedef struct {
    size_t size; /**< Length of the array. */
    size_t max; /**< Maximum size the array ever reached, this is basically the size of the array on HEAP. */
    float growth_factor; /**< Scaling factor for reallocations. */
    void **data; /**< Array of void pointers. */
} nvArray;

/**
 * @brief Create new array.
 * 
 * @return nvArray *
 */
nvArray *nvArray_new();

/**
 * @brief Create new array with more control than @ref nvArray_new
 * 
 * @param default_capacity Default allocation size at initialization
 * @param growth_factor Scaling factor for reallocations
 * @return nvArray *
 */
nvArray *nvArray_new_ex(size_t default_capacity, float growth_factor);

/**
 * @brief Free array.
 * 
 * It's safe to pass `NULL` to this function.
 * 
 * @param array Array to free
 */
void nvArray_free(nvArray *array);

typedef void (*nvArray_free_each_callback)(void *);

/**
 * @brief Free each element of array.
 * 
 * @param array Array
 * @param free_func Free function
 */
void nvArray_free_each(nvArray *array, nvArray_free_each_callback free_func);

/**
 * @brief Add new element to array.
 * 
 * Returns non-zero on error. Use @ref nv_get_error to get more information.
 * 
 * @param array Array to append to
 * @param elem Void pointer to element
 * @return int Status
 */
int nvArray_add(nvArray *array, void *elem);

/**
 * @brief Remove element by index from array and return the element. Returns `NULL` if failed.
 * 
 * @note The array is not sorted after removal, meaning the array gets slightly randomized every remove call.
 * 
 * @param array Array
 * @param index Index of element to remove
 * @return void *
 */
void *nvArray_pop(nvArray *array, size_t index);

/**
 * @brief Remove element from array and return the index. Returns `-1` if failed.
 * 
 * @note The array is not sorted after removal, meaning the array gets slightly randomized every remove call.
 * 
 * @param array Array
 * @param elem Element to remove
 * @return size_t
 */
size_t nvArray_remove(nvArray *array, void *elem);

/**
 * @brief Clear the array.
 * 
 * Elements are not freed if `NULL` is passed as freeing function. Use @ref nv_get_error to get more information.
 * 
 * Returns non-zero on error.
 * 
 * @param array Array
 * @param free_func Free function
 * @return int Status
 */
int nvArray_clear(nvArray *array, void (free_func)(void *));


#endif