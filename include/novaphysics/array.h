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
 * @details Type-generic dynamically growing array implementation
 */


/**
 * @brief Generic array
 * 
 * @param size Length of the array
 * @param max Maximum size the array ever reached
 * @param data Array of void pointers
 */
typedef struct {
    size_t size;
    size_t max;
    void **data;
} nv_Array;

/**
 * @brief Create new array
 * 
 * @return nv_Array *
 */
nv_Array *nv_Array_new();

/**
 * @brief Free array
 * 
 * @param array Array to free
 */
void nv_Array_free(nv_Array *array);

/**
 * @brief Call free function of each element
 * 
 * @param array Array
 * @param free_func Free function
 */
void nv_Array_free_each(nv_Array *array, void (free_func)(void *));

/**
 * @brief Add new element to array
 * 
 * @param array Array to append to
 * @param elem Void pointer to element
 */
void nv_Array_add(nv_Array *array, void *elem);

/**
 * @brief Remove element by index from array and return the element
 *        Returns NULL if failed
 * 
 *        WARNING: The array is not sorted after removal, meaning the
 *                 array gets slightly randomized every remove call
 * 
 * @param array Array
 * @param index Index of element to remove
 * @return void *
 */
void *nv_Array_pop(nv_Array *array, size_t index);

/**
 * @brief Remove element from array and return the index
 *        Returns -1 if failed
 * 
 *        WARNING: The array is not sorted after removal, meaning the
 *                 array gets slightly randomized every remove call
 * 
 * @param array Array
 * @param elem Element to remove
 * @return void *
 */
size_t nv_Array_remove(nv_Array *array, void *elem);


#endif