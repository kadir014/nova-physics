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
 * @brief Hash map struct and its methods.
 * 
 * Notes:
 * - This hash map is designed to store {uint32 -> void *} pairs.
 * - x & (n - 1) is faster than x % n if n is a power of 2.
 * - Empty keys are indicated with -1 (Since keys are unsigned they wrap).
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

    hashmap->_iter_entries = nv_Array_new();

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

void *nv_HashMap_get(nv_HashMap *hashmap, nv_uint32 key) {
    nv_uint32 h = nv_hash(key);
    size_t index = (size_t)(h & (size_t)(hashmap->capacity - 1));

    size_t loop = 0;
    while (hashmap->entries[index].key != -1) {
        //printf("key: %u %u\n", key, hashmap->entries[index].key);
        // Found key
        if (key == hashmap->entries[index].key) {
            //if (loop > 5) printf("get loop: %zu ", loop);
            return hashmap->entries[index].value;
        }

        // Move to next bucket and wrap around if needed (linear probing)
        index++;
        if (index >= hashmap->capacity) index = 0;
        loop++;
    }

    //if (loop > 5) printf("get loop: %zu ", loop);
    return NULL;
}

static nv_uint32 nv_HashMap_set_entry(
    nv_HashMap *hashmap,
    nv_HashMapEntry *entries,
    size_t capacity,
    nv_uint32 key,
    void *value,
    size_t *size_out
) {
    uint64_t hash = nv_hash(key);
    size_t index = (size_t)(hash & (size_t)(capacity - 1));

    //printf("set hash: %zu\n", hash);
    //printf("set hash index: %zu\n", index);

    // Loop till we find an empty entry
    while (entries[index].key != -1) {
        //printf("i: %zu, key: %lu %lu   ", index, entries[index].key, key);
        if (key == entries[index].key) {
            entries[index].value = value;
            return entries[index].key;
        }

        // Move to next bucket and wrap around if needed (linear probing)
        index++;
        if (index >= capacity) index = 0;
    }

    // Didn't find key, insert it.
    if (size_out != NULL)
        (*size_out)++;

    entries[index].key = key;
    entries[index].value = value;

    // nv_HashMapIteratorItem *iter_item = NV_NEW(nv_HashMapIteratorItem);
    // iter_item->entry = &entries[index];
    // nv_Array_add(hashmap->_iter_entries, iter_item);

    return key;
}

bool expand(nv_HashMap *hashmap) {
    size_t new_capacity = hashmap->capacity * 2;
    // TODO: handle overflow

    nv_HashMapEntry *new_entries = calloc(new_capacity, sizeof(nv_HashMapEntry));

    for (size_t i = 0; i < new_capacity; i++)
        new_entries[i].key = -1;

    for (size_t i = 0; i < hashmap->capacity; i++) {
        nv_HashMapEntry entry = hashmap->entries[i];
        if (entry.key != -1) {
            nv_HashMap_set_entry(hashmap, new_entries, new_capacity, entry.key, entry.value, NULL);
        }
    }

    free(hashmap->entries);
    hashmap->entries = new_entries;
    hashmap->capacity = new_capacity;
    return true;
}

nv_uint32 nv_HashMap_set(nv_HashMap *hashmap, nv_uint32 key, void *value) {
    if (hashmap->size >= hashmap->capacity / 2) {
        expand(hashmap);
    }

    return nv_HashMap_set_entry(
        hashmap,
        hashmap->entries,
        hashmap->capacity,
        key, value,
        &hashmap->size
    );
}

void nv_HashMap_remove(
    nv_HashMap *hashmap,
    nv_uint32 key,
    void (free_func)(void *)
) {
    nv_uint32 h = nv_hash(key);
    size_t i = (size_t)(h & (size_t)(hashmap->capacity - 1));

    while (hashmap->entries[i].key != -1) {
        // Found key
        if (key == hashmap->entries[i].key) {
            hashmap->entries[i].key = -1;
            if (free_func != NULL && hashmap->entries[i].value != NULL)
                free_func(hashmap->entries[i].value);
            hashmap->entries[i].value = NULL;
            hashmap->size--;

            // for (size_t j = 0; j < hashmap->_iter_entries->size; j++) {
            //     if (hashmap->entries->key == ((nv_HashMapIteratorItem *)hashmap->_iter_entries->data[j])->entry->key) {
            //         free(nv_Array_pop(hashmap->_iter_entries, j));
            //         break;
            //     }
            // }
        }

        // Move to next bucket and wrap around if needed (linear probing)
        i++;
        if (i >= hashmap->capacity) i = 0;
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

    // for (size_t i = 0; i < hashmap->_iter_entries->size; i++)
    //     free(nv_Array_pop(hashmap->_iter_entries, 0));

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

            // TODO
            void *value = nv_HashMap_get(iterator->_hashmap, entry.key);
            if (value == NULL) continue;

            iterator->key = entry.key;
            iterator->value = entry.value;

            return true;
        }
    }

    return false;

    // while (iterator->_index < hashmap->_iter_entries->size) {
    //     size_t i = iterator->_index++;

    //     nv_HashMapEntry *entry = ((nv_HashMapIteratorItem *)hashmap->_iter_entries->data[i])->entry;

    //     // TODO
    //     void *value = entry->value;
    //     if (value == NULL) continue;

    //     iterator->key = entry->key;
    //     iterator->value = entry->value;

    //     return true;
    // }

    // return false;
}