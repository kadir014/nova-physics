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
#include "novaphysics/array.h"
#include "novaphysics/math.h"


/**
 * @file hashmap.h
 * 
 * @brief Hash map implementation specifically
 *        designed to store {uint32 -> array} pairs
 */


/**
 * @brief Hash map entry
 * 
 * @param key 32-bit integer key
 * @param value Void pointer to data
 */
typedef struct {
    nv_uint32 key;
    void *value;
} nv_HashMapEntry;

/**
 * @brief Free hash map entry
 * 
 * @param entry Entry to free
 */
void nv_HashMapEntry_free(nv_HashMapEntry *entry, void (free_func)(void *));


/**
 * @brief Hash map
 * 
 * @param size Current size
 * @param capacity Capacity
 * @param entries Entry buckets
 */
typedef struct {
    size_t size;
    size_t capacity;
    nv_HashMapEntry *entries;
    nv_Array *_iter_entries;
} nv_HashMap;

/**
 * @brief Create new hash map
 * 
 * @return nv_HashMap * 
 */
nv_HashMap *nv_HashMap_new();

/**
 * @brief Free hash map
 * 
 * @param hashmap Hash map to free
 * @param free_func Free function (can be NULL) 
 */
void nv_HashMap_free(nv_HashMap *hashmap, void (free_func)(void *));

/**
 * @brief Get value of key
 * 
 * @param hashmap Hash map
 * @param key Key
 * @return void * 
 */
void *nv_HashMap_get(nv_HashMap *hashmap, nv_uint32 key);

/**
 * @brief Set value of key
 * 
 * @param hashmap Hash map
 * @param key Key
 * @param value Value
 */
nv_uint32 nv_HashMap_set(nv_HashMap *hashmap, nv_uint32 key, void *value);

/**
 * @brief Remove entry from hash map
 * 
 * @param hashmap Hash map
 * @param key Key
 * @param free_func Free function (can be NULL) 
 */
void nv_HashMap_remove(
    nv_HashMap *hashmap,
    nv_uint32 key,
    void (free_func)(void *)
);

/**
 * @brief Remove all entries in hash map
 * 
 * @param hashmap Hash map to clear
 * @param free_func Free function (can be NULL) 
 */
void nv_HashMap_clear(nv_HashMap *hashmap, void (free_func)(void *));


/**
 * @brief Hash map iterator
 * 
 * @param key Current key
 * @param value Current value
 */
typedef struct {
    nv_uint32 key;
    void *value;
    nv_HashMap *_hashmap;
    size_t _index;
} nv_HashMapIterator;

typedef struct {
    nv_HashMapEntry *entry;
} nv_HashMapIteratorItem;

/**
 * @brief Create a new hash map iterator
 * 
 * @param hashmap Hash map
 * @return nv_HashMapIterator 
 */
nv_HashMapIterator nv_HashMapIterator_new(nv_HashMap *hashmap);

/**
 * @brief Iterate over the hash map. Returns false when all pairs are iterated.
 * 
 * @param iterator Hash map iterator
 * @return bool
 */
bool nv_HashMapIterator_next(nv_HashMapIterator *iterator); 


#endif