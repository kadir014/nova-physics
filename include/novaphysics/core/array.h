/*

  This file is a part of the Nova Physics Engine
  project and distributed under the MIT license.

  Copyright © Kadir Aksoy
  https://github.com/kadir014/nova-physics

*/

#ifndef NOVAPHYSICS_CORE_ARRAY_H
#define NOVAPHYSICS_CORE_ARRAY_H

#include "novaphysics/internal.h"


/**
 * @file core/array.h
 * 
 * @brief Dynamically growing type-specific array.
 */


/**
 * @brief Dynamically growing type-specific array.
 */
typedef struct {
    size_t size; /**< Current number of elements in the array. */
    size_t capacity; /**< Number of elements that can be stored without reallocating. (Basically size of the currently allocated space.) */
    size_t element_size; /**< Size of one element in bytes. */
    float growth_factor; /**< Capacity multiplier used during reallocations. Must be higher than 1. */
    void *data; /**< Value array. */
} nvArray;

/**
 * @brief Fetch element at given index.
 * 
 * @param array Pointer to array.
 * @param index Index of the element.
 * @param type Type of the element.
 * @return Element at given index. Note that no bounds check is done.
 */
#define NV_ARRAY_AT(array, index, type) (*(type *)((char *)(array)->data + (index) * (array)->element_size))

/**
 * @brief Get pointer of element at given index.
 * 
 * @param array Pointer to array.
 * @param index Index of the element.
 * @param type Type of the element.
 * @return Element at given index. Note that no bounds check is done.
 */
#define NV_ARRAY_PTR_AT(array, index, type) ((type *)((char *)(array)->data + (index) * (array)->element_size))

/**
 * @brief Create a new array.
 * 
 * Use @ref nvArray_valid to see if creation was successful. If failed, use
 * @ref nv_get_error to get more information.
 * 
 * @param elem_size Size of one element.
 * @return nvArray *
 */
nvArray *nvArray_new(size_t elem_size);

/**
 * @brief Create a new array.
 * 
 * Use @ref nvArray_valid to see if creation was successful. If failed, use
 * @ref nv_get_error to get more information.
 * 
 * @param elem_size Size of one element.
 * @param default_capacity Initial number of elements to allocate the array with.
 * @param growth_factor Capacity multiplier used during reallocations. Must be higher than 1.
 * @return nvArray *
 */
nvArray *nvArray_new_ex(
    size_t elem_size,
    size_t default_capacity,
    float growth_factor
);

/**
 * @brief Destroy the array.
 * 
 * It's safe to pass `NULL` to this function.
 * 
 * @param array Array to destroy.
 */
void nvArray_free(nvArray *array);

/**
 * @brief Check if array is valid.
 * 
 * @param array Array.
 * @return Whether the state is valid or not.
 */
nv_bool nvArray_valid(const nvArray *array);

/**
 * @brief Append new element at the end of the array.
 * 
 * @param array Array.
 * @param elem Element to add.
 * @return Non-zero on error, use @ref nv_get_error to get more information.
 */
int nvArray_add(nvArray *array, void *elem);

/**
 * @brief Clear the array contents.
 * 
 * @node The space is not reallocated, only the elements are cleared.
 * 
 * @param array Array.
 */
void nvArray_clear(nvArray *array);

typedef void (*nvArray_for_each_callback)(void *elem, void *user_data);

/**
 * @brief Run the callback function on each element in the array.
 * 
 * @param array Array.
 * @param callback Callback function to run on each element.
 * @param user_data Optional user data, can be `NULL`.
 */
void nvArray_for_each(
    nvArray *array,
    nvArray_for_each_callback callback,
    void *user_data
);

/**
 * @brief Get a shallow, independent copy of the array.
 * 
 * Use @ref nvArray_valid to see if creation was successful.
 * 
 * @param array Array.
 * @return nvArray *
 */
nvArray *nvArray_copy(const nvArray *array);

/**
 * @brief Synchronize the reserved space with current size.
 * 
 * Use this function only if you manually updated the `size` member.
 * 
 * @param array Array.
 * @return Non-zero on error, use @ref nv_get_error to get more information.
 */
int nvArray_resize(nvArray *array);

/**
 * @brief Get the total amount of memory used by this array instance.
 * 
 * @param array Array.
 * @return Number of bytes allocated.
 */
size_t nvArray_total_memory_used(const nvArray *array);


#endif