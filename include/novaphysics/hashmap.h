/*

  This file is a part of the Nova Physics Engine
  project and distributed under the MIT license.

  Copyright © Kadir Aksoy
  https://github.com/kadir014/nova-physics

*/

#ifndef NOVAPHYSICS_HASHMAP_H
#define NOVAPHYSICS_HASHMAP_H

#include <stdlib.h>
#include <stdint.h>


/**
 * @file hashmap.h
 * 
 * @brief Hash map implementation.
 */


typedef struct {
    nv_uint64 hash: 48;
    nv_uint64 dib: 16;
} nv_HashMapBucket;


/**
 * @brief Hash map.
 */
typedef struct {
    size_t elsize; /**< Size of each entry in hash map. */
    size_t cap; /**< Capacity of hash map. */
    nv_uint64 (*hash_func)(void *item); /**< Hashing callback function. */

    size_t count; /**< Current number of entries in the hash map. */
    bool oom; /**< Flag reporting if the last set query overflowed memory. */

    size_t bucketsz;
    size_t nbuckets;
    size_t mask;
    size_t growat;
    size_t shrinkat;
    nv_uint8 growpower;

    void *buckets;
    void *spare;
    void *edata;
} nv_HashMap;

/**
 * @brief Create new hash map.
 * 
 * @param item_size Size of the entries stored in the hash map
 * @param cap Starting capacity of the hash map
 * @param hash_func Hash function callback
 * @return nv_HashMap * 
 */
nv_HashMap *nv_HashMap_new(
    size_t item_size,
    size_t cap,
    nv_uint64 (*hash_func)(void *item)
);

/**
 * @brief Free hash map.
 * 
 * @param hashmap Hash map to free
 */
void nv_HashMap_free(nv_HashMap *hashmap);

/**
 * @brief Remove all entries in the hash map.
 * 
 * @param hashmap Hash map to clear
 */
void nv_HashMap_clear(nv_HashMap *hashmap);

/**
 * @brief Get entry from key.
 * 
 * @param hashmap Hash map
 * @param key Key
 * @return void * 
 */
void *nv_HashMap_get(nv_HashMap *map, void *item);

/**
 * @brief Set entry.
 * 
 * @param hashmap Hash map
 * @param item Entry item
 */
void *nv_HashMap_set(nv_HashMap *map, void *item);

/**
 * @brief Remove entry from hash map with key.
 * 
 * If removed during iteration, set counter back to 0.
 * 
 * @param hashmap Hash map
 * @param key Key
 * @return void *
 */
void *nv_HashMap_remove(nv_HashMap *hashmap, void *key);

/**
 * @brief Iterate over hash map entries.
 * 
 * @param hashmap Hash map
 * @param index Pointer to index counter
 * @param item Pointer to entry pointer
 * @return bool
 */
bool nv_HashMap_iter(nv_HashMap *hashmap, size_t *index, void **item);


#endif