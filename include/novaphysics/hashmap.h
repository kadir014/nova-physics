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


/**
 * @file hashmap.h
 * 
 * @details Hash map implementation specifically
 *          designed to store {uint32 -> array} pairs
 */


/**
 * @brief Hash 32-bit integer
 * 
 * @param key Integer key to hash
 * @return uint32_t 
 */
uint32_t nv_hash(uint32_t key);


/**
 * @brief Combine two 16-bit integers into 32-bit one
 * 
 * @param x First integer
 * @param y Second ineger
 * @return uint32_t 
 */
uint32_t nv_pair(uint16_t x, uint16_t y);


/**
 * @brief Hash map entry
 * 
 * @param key 32-bit integer key
 * @param value Array
 */
typedef struct {
    uint32_t *key;
    nv_Array *value;
} nv_HashMapEntry;

/**
 * @brief Free hash map entry
 * 
 * @param entry Entry to free
 */
void nv_HashMapEntry_free(nv_HashMapEntry *entry);


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
 */
void nv_HashMap_free(nv_HashMap *hashmap);

/**
 * @brief Get value of key
 * 
 * @param hashmap Hash map
 * @param key Key
 * @return nv_Array * 
 */
nv_Array *nv_HashMap_get(nv_HashMap *hashmap, uint32_t key);

/**
 * @brief Set value of key
 * 
 * @param hashmap Hash map
 * @param key Key
 * @param value Value
 */
uint32_t *nv_HashMap_set(nv_HashMap *hashmap, uint32_t key, nv_Array *value);


#endif