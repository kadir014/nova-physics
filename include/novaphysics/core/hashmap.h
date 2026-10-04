/*

  This file is a part of the Nova Physics Engine
  project and distributed under the MIT license.

  Copyright © Kadir Aksoy
  https://github.com/kadir014/nova-physics

*/

#ifndef NOVAPHYSICS_CORE_HASHMAP_H
#define NOVAPHYSICS_CORE_HASHMAP_H

#include "novaphysics/internal.h"
#include "novaphysics/core/refarray.h"


/**
 * @file core/hashmap.h
 * 
 * @brief Hash map implementation.
 */


/**
 * @brief User hasher function.
 * 
 * This should generate a hash for the given item,
 * it should be distributed as uniformly as possible to reduce collisions.
 * 
 * @param item Pointer to item.
 * @return 64-bit unsigned integer hash.
 */
typedef nv_uint64 (*nvHashMap_hasher)(void *item);

/**
 * @brief User comparer function.
 * 
 * Determines whether two items are considered equal. This is used
 * during lookup and similar operations to identify an existing item.
 * 
 * @param a First item.
 * @param b Second item.
 * @return Result of user-defined comparison if two items are equal or not.
 */
typedef nv_bool (*nvHashMap_comparer)(void *a, void *b);

typedef struct {
    size_t size; /**< Current number of elements in the hashmap. */
    size_t capacity; /**< Current allocated space for maximum number of elements. */
    size_t item_size; /**< Size of one item in bytes. */
    float growth_factor; /**< Capacity multipler used during reallocations. Must be higher than 1. */
    float growth_rule; /**< Capacity percentage to decide when to resize. Must be in range (0, 1]. */
    void *data; /**< Item array. */
    nv_bool *data_state; /**< Occupancy array. */
    nv_uint64 *data_hashes; /**< Cached item hashes. */
    void *removed_item; /**< Reusable removed-value buffer. */
    nvHashMap_hasher hasher;
    nvHashMap_comparer comparer;
} nvHashMap;

/**
 * @brief Create a new hashmap.
 * 
 * Use @ref nvHashMap_valid to see if creation was successful. If failed,
 * use @ref nv_get_error for more information.
 * 
 * Usage:
 * ```
 * typedef struct {
 *     char name[16];
 *     char info[256];
 *     int age;
 * } MyEntry;
 * 
 * nv_uint64 my_hasher(void *item) {
 *     MyEntry *entry = item;
 * 
 *     return some_64bit_hash_function(entry.name);
 * }
 * 
 * nv_bool my_comparer(void *a, void *b) {
 *     MyEntry *entry_a = a;
 *     MyEntry *entry_b = b;
 *     
 *     return strcmp(entry_a->name, entry_b->name) == 0;
 * }
 * 
 * nvHashMap mymap = nvHashMap_new(sizeof(MyEntry), my_hasher);
 * ```
 * 
 * @param item_size Size of one item in bytes.
 * @param hasher User hasher function, see @ref nvHashMap_hasher for details.
 * @param comparer User comparer function, see @ref nvHashMap_comparer for details.
 * @return nvHashMap *
 */
nvHashMap *nvHashMap_new(
    size_t item_size,
    nvHashMap_hasher hasher,
    nvHashMap_comparer comparer
);

/**
 * @brief Create a new hashmap.
 * 
 * Use @ref nvHashMap_valid to see if creation was successful. If failed,
 * use @ref nv_get_error for more information.
 * 
 * Refer to @ref nvHashMap_new for example usage.
 * 
 * @param item_size Size of one item in bytes.
 * @param hasher User hasher function, see @ref nvHashMap_hasher for details.
 * @param comparer User comparer function, see @ref nvHashMap_comparer for details.
 * @param default_capacity Initial number of items to allocate for.
 * @param growth_factor Capacity multipler used during reallocations. Must be higher than 1.
 * @param growth_rule Capacity percentage to decide when to resize. Must be in range (0, 1].
 * @return nvHashMap *
 */
nvHashMap *nvHashMap_new_ex(
    size_t item_size,
    nvHashMap_hasher hasher,
    nvHashMap_comparer comparer,
    size_t default_capacity,
    float growth_factor,
    float growth_rule
);

/**
 * @brief Destroy the hashmap.
 * 
 * It's safe to pass `NULL` to this function.
 * 
 * @param hashmap 
 */
void nvHashMap_free(nvHashMap *hashmap);

/**
 * @brief Check if hashmap is valid.
 * 
 * @param hashmap Hashmap.
 * @return Whether the state is valid or not.
 */
nv_bool nvHashMap_valid(const nvHashMap *hashmap);

/**
 * @brief Clear the contents of the hashmap.
 * 
 * @note The space is not reallocated, only the elements are cleared.
 * 
 * @param hashmap Hashmap.
 */
void nvHashMap_clear(nvHashMap *hashmap);

/**
 * @brief Fetch item from the hashmap.
 * 
 * Only the key member must be initialized in the given item.
 * 
 * @param hashmap Hashmap.
 * @param item Pointer to item.
 * @return Pointer to item inside hashmap.
 *         `NULL` if failed, use @ref nv_get_error for more information.
 */
void *nvHashMap_get(const nvHashMap *hashmap, void *item);

/**
 * @brief Set an existing item or add new one to hashmap.
 * 
 * Only the key member must be initialized in the given item.
 * 
 * @param hashmap Hashmap.
 * @param item Pointer to item.
 * @return Pointer to item inside hashmap.
 *         `NULL` if failed, use @ref nv_get_error for more information.
 */
void *nvHashMap_set(nvHashMap *hashmap, void *item);

/**
 * @brief Remove an existing item from hashmap.
 * 
 * Only the key member must be initialized in the given item.
 * 
 * @warning Removal can relocate entries and invalidate pointers to stored items.
 * 
 * @param hashmap Hashmap.
 * @param item Pointer to item.
 * @return Map-owned copy of the removed item, valid until the next successful
 *         removal or map destruction. Do not free it. `NULL` if absent or failed.
 */
void *nvHashMap_remove(nvHashMap *hashmap, void *item);

/**
 * @brief Check whether an item exists inside hashmap.
 * 
 * Only the key member must be initialized in the given item.
 * 
 * @param hashmap Hashmap.
 * @param item Pointer to item.
 * @return Whether the hashmap contains the item or not.
 */
nv_bool nvHashMap_contains(const nvHashMap *hashmap, void *item);

/**
 * @brief Iterate every item in the hashmap in no definite order.
 * 
 * @warning Do not mutate the hashmap while iterating.
 * 
 * Usage:
 * ```
 * size_t idx = 0;
 * void *item = NULL;
 * while (nvHashMap_iter(&my_hashmap, &idx, &item)) {
 *     // Do stuff with 'item'...
 * }
 * ```
 * 
 * @param hashmap Hashmap.
 * @param index Pointer to set the current index.
 * @param item Pointer to set the current item.
 * @return Is the iteration still running?
 */
nv_bool nvHashMap_iter(const nvHashMap *hashmap, size_t *index, void **item);

/**
 * @brief Get a view array filled with references to hashmap's items.
 * 
 * Use @ref nvRefArray_valid to see if creation was successful.
 * 
 * It's caller's responsibility to free the newly created nvRefArray.
 * 
 * @warning If the hashmap is altered in anyway while the view array is alive,
 *          the references might point to garbage data.
 * 
 * @param hashmap Hashmap.
 * @return nvRefArray *
 */
nvRefArray *nvHashMap_get_view(nvHashMap *hashmap);

/**
 * @brief Get the total amount of memory used by this hashmap instance.
 * 
 * @param refarray Hashmap.
 * @return Number of bytes allocated.
 */
size_t nvHashMap_total_memory_used(nvHashMap *hashmap);


#endif