/*

  This file is a part of the Nova Physics Engine
  project and distributed under the MIT license.

  Copyright © Kadir Aksoy
  https://github.com/kadir014/nova-physics

*/

#include "novaphysics/internal.h"
#include "novaphysics/hashmap.h"
#include "novaphysics/constants.h"
#include "novaphysics/math.h"


/**
 * @file hashmap.c
 * 
 * @details Hash map implementation specifically designed to
 *          store {uint32 -> void *} pairs.
 * 
 *          Function documentations are in novaphysics/hashmap.h
 */


void nv_HashMapEntry_free(nv_HashMapEntry *entry, void (free_func)(void *)) {
    if (free_func != NULL) free_func(entry->value);
}


nv_HashMap *nv_HashMap_new() {
    nv_HashMap *hashmap = NV_NEW(nv_HashMap);
    hashmap->size = 0;
    hashmap->capacity = NV_HASHMAP_CAPACITY;

    hashmap->entries = calloc(hashmap->capacity, sizeof(nv_HashMapEntry));

    for (size_t i = 0; i < hashmap->capacity; i++)
        hashmap->entries[i].key = -1;

    return hashmap;
}

void nv_HashMap_free(nv_HashMap *hashmap, void (free_func)(void *)) {
    for (size_t i = 0; i < hashmap->capacity; i++) {
        if (hashmap->entries[i].value != NULL)
            nv_HashMapEntry_free(&hashmap->entries[i], free_func);
    }

    free(hashmap->entries);
    free(hashmap);
}

void *nv_HashMap_get(nv_HashMap *hashmap, uint32_t key) {
    // & hash with capacity-1 to ensure it's within entries array
    uint32_t h = nv_hash(key);
    size_t index = (size_t)(h & (size_t)(hashmap->capacity - 1));

    while (hashmap->entries[index].key != -1) {
        // Found key
        if (key == hashmap->entries[index].key)
            return hashmap->entries[index].value;

        // Move to next bucket (linear probing)
        index++;
        if (index >= hashmap->capacity) {
            // Wrap around
            index = 0;
        }
    }

    return NULL;
}

static uint32_t nv_HashMap_set_entry(
    nv_HashMapEntry *entries,
    size_t capacity,
    uint32_t key,
    void *value,
    size_t *size_out
) {
    // & hash with capacity-1 to ensure it's within entries array
    uint64_t hash = nv_hash(key);
    size_t index = (size_t)(hash & (uint64_t)(capacity - 1));

    // Loop till we find an empty entry
    while (entries[index].key != -1) {
        if (key == entries[index].key) {
            // Found key (it already exists), update value.
            entries[index].value = value;
            return entries[index].key;
        }

        // Move to next bucket (linear probing).
        index++;

        // Wrap around
        if (index >= capacity) {
            index = 0;
        }
    }

    // Didn't find key, insert it.
    (*size_out)++;

    entries[index].key = key;
    entries[index].value = value;

    return key;
}

uint32_t nv_HashMap_set(nv_HashMap *hashmap, uint32_t key, void *value) {
    // TODO: capacity overflow checks and map expansion

    return nv_HashMap_set_entry(
        hashmap->entries,
        hashmap->capacity,
        key, value,
        &hashmap->size
    );
}

void nv_HashMap_remove(
    nv_HashMap *hashmap,
    uint32_t key,
    void (free_func)(void *)
) {
    // & hash with capacity-1 to ensure it's within entries array
    uint32_t h = nv_hash(key);
    size_t i = (size_t)(h & (size_t)(hashmap->capacity - 1));

    while (hashmap->entries[i].key != -1) {
        // Found key
        if (key == hashmap->entries[i].key)
            hashmap->entries[i].key = -1;
            if (free_func != NULL && hashmap->entries[i].value != NULL)
                free_func(hashmap->entries[i].value);
            hashmap->entries[i].value = NULL;
            hashmap->size--;
            return;

        // Move to next bucket (linear probing)
        i++;

        // Wrap around
        if (i >= hashmap->capacity) {
            i = 0;
        }
    }
}

void nv_HashMap_clear(nv_HashMap *hashmap, void (free_func)(void *)) {
    for (size_t i = 0; i < hashmap->capacity; i++) {
        if (hashmap->entries[i].key != -1) {
            if (free_func != NULL && hashmap->entries[i].value != NULL)
                free_func(hashmap->entries[i].value);
            hashmap->entries[i].value = NULL;
            hashmap->entries[i].key = -1;
        }
    }

    hashmap->size = 0;
}


nv_HashMapIterator nv_HashMapIterator_new(nv_HashMap *hashmap) {
    return (nv_HashMapIterator){
        ._hashmap = hashmap,
        ._index = 0
    };
}

bool nv_HashMapIterator_next(nv_HashMapIterator *iterator) {
    nv_HashMap *hashmap = iterator->_hashmap;

    while (iterator->_index < hashmap->capacity) {
        size_t i = iterator->_index++;

        if (hashmap->entries[i].key != -1) {
            nv_HashMapEntry entry = hashmap->entries[i];
            iterator->key = entry.key;
            iterator->value = entry.value;

            return true;
        }
    }

    return false;
}