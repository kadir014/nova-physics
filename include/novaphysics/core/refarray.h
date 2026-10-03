/*

  This file is a part of the Nova Physics Engine
  project and distributed under the MIT license.

  Copyright © Kadir Aksoy
  https://github.com/kadir014/nova-physics

*/

#ifndef NOVAPHYSICS_CORE_REFARRAY_H
#define NOVAPHYSICS_CORE_REFARRAY_H

#include "novaphysics/internal.h"


/**
 * @file core/refarray.h
 * 
 * @brief Dynamically growing type-generic reference array.
 */


/**
 * @brief Dynamically growing type-generic reference array.
 * 
 * A dynamic array that stores pointers to manually-managed objects.
 * 
 * The array itself does not own or manage the lifetime of the pointed-to data,
 * it simply maintains a dense resizable array of references.
 */
typedef struct {
    size_t size; /**< Current number of elements in the array. */
    size_t capacity; /**< Number of elements that can be stored without reallocating. (Basically size of the currently allocated space.) */
    float growth_factor; /**< Capacity multiplier used during reallocations. Must be higher than 1. */
    void **data; /**< Array of void pointers to user-managed data. */
} nvRefArray;

/**
 * @brief Create a new reference array.
 * 
 * Use @ref nvRefArray_valid to see if creation was successful. If failed,
 * use @ref nv_get_error to get more information.
 * 
 * @return nvRefArray *
 */
nvRefArray *nvRefArray_new();

/**
 * @brief Create a new reference array.
 * 
 * Use @ref nvRefArray_valid to see if creation was successful. If failed,
 * use @ref nv_get_error to get more information.
 * 
 * @param default_capacity Initial number of elements to allocate the array with.
 * @param growth_factor Capacity multiplier used during reallocations. Must be higher than 1.
 * @return nvRefArray *
 */
nvRefArray *nvRefArray_new_ex(size_t default_capacity, float growth_factor);

/**
 * @brief Destroy the reference array.
 * 
 * It's safe to pass `NULL` to this function.
 * 
 * @param refarray Reference array to destroy.
 */
void nvRefArray_free(nvRefArray *refarray);

/**
 * @brief Check if reference array is valid.
 * 
 * @param refarray Reference array.
 * @return Whether the state is valid or not.
 */
nv_bool nvRefArray_valid(const nvRefArray *refarray);

/**
 * @brief Append new element at the end of the reference array.
 * 
 * @param refarray Reference array.
 * @param elem Pointer to add.
 * @return Non-zero on error, use @ref nv_get_error to get more information.
 */
int nvRefArray_add(nvRefArray *refarray, void *elem);

/**
 * @brief Remove an element by index and return the element.
 * 
 * @param refarray Reference array.
 * @param index Index of the element to remove.
 * @return Removed element if successful. `NULL` if failed, use @ref nv_get_error to get more information.
 */
void *nvRefArray_pop(nvRefArray *refarray, size_t index);

/**
 * @brief Remove the first occurence of element and return the index.
 * 
 * @param refarray Reference array.
 * @param elem Element to remove.
 * @return Index of the element if successful.
 *         `NV_INVALID_INDEX_Z` if failed, use @ref nv_get_error to get more information.
 */
size_t nvRefArray_remove(nvRefArray *refarray, void *elem);

/**
 * @brief Clear the array contents.
 * 
 * @note The space is not reallocated, only the elements are cleared.
 * 
 * @param refarray Reference array.
 * @param free_func Free function to call on each element before removing. Can be `NULL`.
 */
void nvRefArray_clear(nvRefArray *refarray, void (free_func)(void *));

typedef void (*nvRefArray_for_each_callback)(void *elem, void *user_data);

/**
 * @brief Run the callback function on each element in the array.
 * 
 * @param refarray Reference array.
 * @param callback Callback function to run on each element.
 * @param user_data Optional user data, can be `NULL`.
 */
void nvRefArray_for_each(
    nvRefArray *refarray,
    nvRefArray_for_each_callback callback,
    void *user_data
);

/**
 * @brief Get a shallow, independent copy of the reference array.
 * 
 * Use @ref nvRefArray_valid to see if creation was successful.
 * 
 * @param refarray Reference array.
 * @return nvRefArray *
 */
nvRefArray *nvRefArray_copy(const nvRefArray *refarray);

/**
 * @brief Synchronize the reserved space with current size.
 * 
 * Use this function only if you manually updated the `size` member.
 * 
 * @param refarray Reference array.
 * @return Non-zero on error, use @ref nv_get_error to get more information.
 */
int nvRefArray_resize(nvRefArray *refarray);

/**
 * @brief Get the total amount of memory used by this refarray instance.
 * 
 * Elements are counted as pointers.
 * 
 * @param refarray Reference array.
 * @return Number of bytes allocated.
 */
size_t nvRefArray_total_memory_used(const nvRefArray *refarray);


#endif