/*

  This file is a part of the Nova Physics Engine
  project and distributed under the MIT license.

  Copyright © Kadir Aksoy
  https://github.com/kadir014/nova-physics

*/

#ifndef NOVAPHYSICS_HASHMAP_H
#define NOVAPHYSICS_HASHMAP_H

#include "novaphysics/internal.h"


/**
 * @file core/hashmap.h
 * 
 * @brief Hash map implementation.
 */


typedef struct {
    nv_uint64 hash: 48;
    nv_uint64 dib: 16;
} nvHashMapBucket;

/**
 * @brief Hash map.
 */
typedef struct {
    size_t elsize; /**< Size of each entry in hash map. */
    size_t cap; /**< Capacity of hash map. */
    nv_uint64 (*hash_func)(void *item); /**< Hashing callback function. */

    size_t count; /**< Current number of entries in the hash map. */
    nv_bool oom; /**< Flag reporting if the last set query overflowed memory. */

    size_t bucketsz;
    size_t nbuckets;
    size_t mask;
    size_t growat;
    size_t shrinkat;
    nv_uint8 growpower;

    void *buckets;
    void *spare;
    void *edata;
} nvHashMap;

/**
 * @brief Create new hash map.
 * 
 * Returns `NULL` on error. Use @ref nv_get_error to get more information.
 * 
 * @param item_size Size of the entries stored in the hash map
 * @param cap Starting capacity of the hash map
 * @param hash_func Hash function callback
 * @return nvHashMap * 
 */
nvHashMap *nvHashMap_new(
    size_t item_size,
    size_t cap,
    nv_uint64 (*hash_func)(void *item)
);

/**
 * @brief Free hash map.
 * 
 * @param hashmap Hash map to free
 */
void nvHashMap_free(nvHashMap *hashmap);

/**
 * @brief Remove all entries in the hash map.
 * 
 * @param hashmap Hash map to clear
 */
void nvHashMap_clear(nvHashMap *hashmap);

/**
 * @brief Get entry from key.
 * 
 * @param hashmap Hash map
 * @param key Key
 * @return void * 
 */
void *nvHashMap_get(nvHashMap *map, void *item);

/**
 * @brief Set entry.
 * 
 * @param hashmap Hash map
 * @param item Entry item
 */
void *nvHashMap_set(nvHashMap *map, void *item);

/**
 * @brief Remove entry from hash map with key.
 * 
 * If removed during iteration, set counter back to 0.
 * 
 * @param hashmap Hash map
 * @param key Key
 * @return void *
 */
void *nvHashMap_remove(nvHashMap *hashmap, void *key);

/**
 * @brief Iterate over hash map entries.
 * 
 * @param hashmap Hash map
 * @param index Pointer to index counter
 * @param item Pointer to entry pointer
 * @return nv_bool
 */
nv_bool nvHashMap_iter(nvHashMap *hashmap, size_t *index, void **item);


#endif