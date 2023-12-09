/*

  This file is a part of the Nova Physics Engine
  project and distributed under the MIT license.

  Copyright © Kadir Aksoy
  https://github.com/kadir014/nova-physics

*/

#ifndef NOVAPHYSICS_ARRAY_H
#define NOVAPHYSICS_ARRAY_H

#include <stdlib.h>


/**
 * @file array.h
 * 
 * @brief Type-generic dynamically growing array implementation.
 */


/**
 * @brief Type-generic dynamically growing array implementation.
 */
typedef struct {
    size_t size; /**< Length of the array. */
    size_t max; /**< Maximum size the array ever reached, this is basically the size of the array on HEAP. */
    void **data; /**< Array of void pointers. */
} nvArray;

/**
 * @brief Create new array.
 * 
 * @return nvArray *
 */
nvArray *nvArray_new();

/**
 * @brief Free array.
 * 
 * @param array Array to free
 */
void nvArray_free(nvArray *array);

/**
 * @brief Free each element of array.
 * 
 * @param array Array
 * @param free_func Free function
 */
void nvArray_free_each(nvArray *array, void (free_func)(void *));

/**
 * @brief Add new element to array.
 * 
 * @param array Array to append to
 * @param elem Void pointer to element
 */
void nvArray_add(nvArray *array, void *elem);

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
 * Elements are not freed if `NULL` is passed as freeing function.
 * 
 * @param array Array
 * @param free_func Free function
 */
void nvArray_clear(nvArray *array, void (free_func)(void *));


#endif