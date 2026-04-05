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
 * @brief Type-generic dynamically growing array.
 */


/**
 * @brief Type-generic dynamically growing array.
 * 
 * A dynamic array that stores only *pointers* to external data.
 * It is intended for managing collections of heap-allocated, long-lived objects.
 * 
 * The array itself does not own or manage the lifetime of the pointed-to data,
 * it simply maintains a dense resizable list of references.
 */
typedef struct {
    size_t size; /**< Current length of the array. */
    size_t max; /**< Maximum size the array ever reached, this is basically the size of the array on HEAP. */
    float growth_factor; /**< Scaling factor for reallocations. */
    void **data; /**< Array of void pointers. */
} nvArray;

/**
 * @brief Create new array.
 * 
 * Returns `NULL` on error. Use @ref nv_get_error to get more information.
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
 * @brief Remove element by index from array and return the element.
 * 
 * Returns `NULL` if removal failed.
 * 
 * @note This method does not preserve order. Removed element is swapped with last element.
 * 
 * @param array Array
 * @param index Index of element to remove
 * @return void *
 */
void *nvArray_pop(nvArray *array, size_t index);

/**
 * @brief Remove element from array and return the index.
 * 
 * Returns `(size_t)(-1)` if removal failed.
 * 
 * @note This method does not preserve order. Removed element is swapped with last element.
 * 
 * @param array Array
 * @param elem Element to remove
 * @return size_t
 */
size_t nvArray_remove(nvArray *array, void *elem);

/**
 * @brief Clear the array.
 * 
 * Elements are not freed if `NULL` is passed as freeing function.
 * 
 * Returns non-zero on error. Use @ref nv_get_error to get more information.
 * 
 * @param array Array
 * @param free_func Free function
 * @return int Status
 */
int nvArray_clear(nvArray *array, void (free_func)(void *));

/**
 * @brief Get the total amount of memory used by this array instance.
 * 
 * Elements are counted as pointers.
 * 
 * @param array Array
 * @return size_t 
 */
size_t nvArray_total_memory_used(nvArray *array);


#endif