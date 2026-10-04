/*

  This file is a part of the Nova Physics Engine
  project and distributed under the MIT license.

  Copyright © Kadir Aksoy
  https://github.com/kadir014/nova-physics

*/

#include "novaphysics/internal.h"
#include "novaphysics/core/hashmap.h"
#include "novaphysics/constants.h"
#include "novaphysics/math.h"


/**
 * @file core/hashmap.c
 * 
 * @brief Hash map implementation.
 */


nvHashMap *nvHashMap_new(
    size_t item_size,
    nvHashMap_hasher hasher,
    nvHashMap_comparer comparer
) {
    return nvHashMap_new_ex(item_size, hasher, comparer, 64, 2.0f, 0.75f);
}

nvHashMap *nvHashMap_new_ex(
    size_t item_size,
    nvHashMap_hasher hasher,
    nvHashMap_comparer comparer,
    size_t default_capacity,
    float growth_factor,
    float growth_rule
) {
    nvHashMap *hashmap = NV_NEW(nvHashMap);
    NV_MEM_CHECK(hashmap);

    hashmap->size = 0;
    hashmap->capacity = default_capacity;
    hashmap->item_size = item_size;
    hashmap->growth_factor = growth_factor;
    hashmap->growth_rule = growth_rule;
    hashmap->data = NULL;
    hashmap->data_state = NULL;
    hashmap->data_hashes = NULL;
    hashmap->removed_item = NULL;
    hashmap->hasher = hasher;
    hashmap->comparer = comparer;

    if (
        growth_factor <= 1.0f ||
        growth_rule <= 0.0f ||
        growth_rule > 1.0f ||
        default_capacity == 0 ||
        item_size == 0
    ) {
        return hashmap;
    }

    hashmap->data = NV_MALLOC(item_size * default_capacity);
    if (!hashmap->data) {
        nv_set_error("Failed to allocate memory.");
        return hashmap;
    }

    hashmap->data_state = NV_CALLOC(default_capacity, sizeof(nv_bool));
    if (!hashmap->data_state) {
        NV_FREE(hashmap->data);
        hashmap->data = NULL;
        nv_set_error("Failed to allocate memory.");
        return hashmap;
    }

    hashmap->data_hashes = NV_MALLOC(sizeof(nv_uint64) * default_capacity);
    if (!hashmap->data_hashes) {
        NV_FREE(hashmap->data_state);
        NV_FREE(hashmap->data);
        hashmap->data_state = NULL;
        hashmap->data = NULL;
        nv_set_error("Failed to allocate memory.");
        return hashmap;
    }

    return hashmap;
}

void nvHashMap_free(nvHashMap *hashmap) {
    if (!hashmap) {
        return;
    }

    if (hashmap->removed_item)
        NV_FREE(hashmap->removed_item);
    NV_FREE(hashmap->data_hashes);
    NV_FREE(hashmap->data_state);
    NV_FREE(hashmap->data);
    NV_FREE(hashmap);
}

nv_bool nvHashMap_valid(const nvHashMap *hashmap) {
    return !(
        !hashmap ||
        !hashmap->data ||
        !hashmap->data_state ||
        !hashmap->data_hashes ||
        hashmap->growth_factor <= 1.0f ||
        hashmap->growth_rule <= 0.0f || hashmap->growth_rule > 1.0f ||
        hashmap->capacity == 0 ||
        hashmap->size > hashmap->capacity ||
        hashmap->item_size == 0 ||
        !hashmap->hasher ||
        !hashmap->comparer
    );
}

void nvHashMap_clear(nvHashMap *hashmap) {
    hashmap->size = 0;
    memset(hashmap->data, 0, hashmap->item_size * hashmap->capacity);
    memset(hashmap->data_state, false, sizeof(nv_bool) * hashmap->capacity);
    memset(hashmap->data_hashes, 0, sizeof(nv_uint64) * hashmap->capacity);

    // Technically, the only important state is `data_state` array, the others
    // do not need cleaning, they are not read unless one slot is occupied.
}

void *nvHashMap_get(const nvHashMap *hashmap, void *item) {
    if (!hashmap || !item) {
        nv_set_error("Invalid arguments, either hashmap or item is NULL.");
        return NULL;
    }

    nv_uint64 hash = hashmap->hasher(item);
    nv_uint64 hash_mod = hash % hashmap->capacity;

    // Linear probing
    while (hashmap->data_state[hash_mod] == true) {
        if (hashmap->data_hashes[hash_mod] == hash) {
            void *existing_item = (char *)hashmap->data + hash_mod * hashmap->item_size;

            if (hashmap->comparer(existing_item, item)) {
                return existing_item;
            }
        }

        // Avoid modulo inside linear probin loop
        if (++hash_mod == hashmap->capacity) {
            hash_mod = 0;
        }
    }

    return NULL;
}

static nv_bool resize_and_reinsert(nvHashMap *hashmap, size_t new_capacity) {
    void *new_data = NV_MALLOC(new_capacity * hashmap->item_size);
    if (!new_data) {
        nv_set_error("Failed to allocate.");
        return false;
    }

    nv_bool *new_data_state = NV_CALLOC(new_capacity, sizeof(nv_bool));
    if (!new_data_state) {
        NV_FREE(new_data);
        nv_set_error("Failed to allocate.");
        return false;
    }

    nv_uint64 *new_data_hashes = NV_MALLOC(sizeof(nv_uint64) * new_capacity);
    if (!new_data_hashes) {
        NV_FREE(new_data_state);
        NV_FREE(new_data);
        nv_set_error("Failed to allocate.");
        return false;
    }

    // Re-hash and reinsert every item back into the reallocated space.
    for (size_t i = 0; i < hashmap->capacity; i++) {
        if (!hashmap->data_state[i]) {
            continue;
        }

        void *item = (char *)hashmap->data + i * hashmap->item_size;

        nv_uint64 hash = hashmap->data_hashes[i];
        size_t index = hash % new_capacity;

        while (new_data_state[index]) {
            if (++index == new_capacity) {
                index = 0;
            }
        }

        new_data_state[index] = true;
        new_data_hashes[index] = hash;
        memcpy(
            (char *)new_data + index * hashmap->item_size,
            item,
            hashmap->item_size
        );
    }

    NV_FREE(hashmap->data_hashes);
    NV_FREE(hashmap->data_state);
    NV_FREE(hashmap->data);

    hashmap->data = new_data;
    hashmap->data_state = new_data_state;
    hashmap->data_hashes = new_data_hashes;
    hashmap->capacity = new_capacity;

    return true;
}

void *nvHashMap_set(nvHashMap *hashmap, void *item) {
    if (!hashmap || !item) {
        nv_set_error("Invalid arguments, either hashmap or item is NULL.");
        return NULL;
    }

    if ((float)hashmap->size >= (float)hashmap->capacity * hashmap->growth_rule) {
        size_t new_capacity = (size_t)((float)hashmap->capacity * hashmap->growth_factor);

        if (!resize_and_reinsert(hashmap, new_capacity)) {
            return NULL;
        }
    }

    nv_uint64 hash = hashmap->hasher(item);
    nv_uint64 hash_mod = hash % hashmap->capacity;

    // Linear probing
    while (hashmap->data_state[hash_mod] == true) {

        // If item already exists, update it
        if (hashmap->data_hashes[hash_mod] == hash) {
            void *existing_item = (char *)hashmap->data + hash_mod * hashmap->item_size;

            if (hashmap->comparer(existing_item, item)) {
                memcpy(existing_item, item, hashmap->item_size);

                return existing_item;
            }
        }

        // If not, keep searching
        if (++hash_mod == hashmap->capacity) {
            hash_mod = 0;
        }
    }

    // Item doesn't exist in the hashmap, insert into first found empty slot
    hashmap->data_state[hash_mod] = true;
    hashmap->data_hashes[hash_mod] = hash;
    void *new_item = (char *)hashmap->data + hash_mod * hashmap->item_size;
    memcpy(new_item, item, hashmap->item_size);

    hashmap->size++;

    return new_item;
}

void *nvHashMap_remove(nvHashMap *hashmap, void *item) {
    // Backward-shift deletion implementation
    // Tombstones degrade hashmap performance over time, I do not want to implement
    // automatic shriking and tombstone cleanup...

    if (!hashmap || !item) {
        nv_set_error("Invalid arguments, either hashmap or item is NULL.");
        return NULL;
    }

    nv_uint64 hash = hashmap->hasher(item);
    size_t hole = hash % hashmap->capacity;
    size_t probed = 0;

    // Bound the search even when the configured load factor permits a full table
    while (probed < hashmap->capacity && hashmap->data_state[hole]) {
        void *existing_item = (char *)hashmap->data + hole * hashmap->item_size;

        if (
            hashmap->data_hashes[hole] == hash &&
            hashmap->comparer(existing_item, item)
        ) {
            break;
        }

        if (++hole == hashmap->capacity) {
            hole = 0;
        }
        probed++;
    }

    if (probed == hashmap->capacity || !hashmap->data_state[hole]) {
        return NULL;
    }

    // Allocate once before shifting. Failure leaves the table unchanged.
    if (!hashmap->removed_item) {
        hashmap->removed_item = NV_MALLOC(hashmap->item_size);
        if (!hashmap->removed_item) {
            return NULL;
        }
    }
    memcpy(
        hashmap->removed_item,
        (char *)hashmap->data + hole * hashmap->item_size,
        hashmap->item_size
    );
    hashmap->data_state[hole] = false;

    size_t scan = hole;
    if (++scan == hashmap->capacity) {
        scan = 0;
    }
    for (size_t examined = 0; examined < hashmap->capacity - 1; examined++) {
        if (!hashmap->data_state[scan]) break;

        size_t home = hashmap->data_hashes[scan] % hashmap->capacity;
        size_t distance_to_scan =
            scan >= home
            ? scan - home
            : hashmap->capacity - home + scan;
        size_t distance_to_hole =
            hole >= home
            ? hole - home
            : hashmap->capacity - home + hole;

        // Move only if this entry's probe path crosses the hole
        if (distance_to_hole < distance_to_scan) {
            memcpy(
                (char *)hashmap->data + hole * hashmap->item_size,
                (char *)hashmap->data + scan * hashmap->item_size,
                hashmap->item_size
            );
            hashmap->data_hashes[hole] = hashmap->data_hashes[scan];
            hashmap->data_state[hole] = true;
            hashmap->data_state[scan] = false;
            hole = scan;
        }
        if (++scan == hashmap->capacity) {
            scan = 0;
        }
    }

    hashmap->size--;
    return hashmap->removed_item;
}

nv_bool nvHashMap_contains(const nvHashMap *hashmap, void *item) {
    if (!hashmap || !item) {
        return false;
    }

    void *lookup = nvHashMap_get(hashmap, item);
    return lookup != NULL;
}

nv_bool nvHashMap_iter(const nvHashMap *hashmap, size_t *index, void **item) {
    if (!hashmap || !index || !item) {
        return false;
    }

    while (*index < hashmap->capacity) {
        size_t current = (*index)++;

        if (hashmap->data_state[current]) {
            *item = (char *)hashmap->data + current * hashmap->item_size;
            return true;
        }
    }

    *item = NULL;
    return false;
}

nvRefArray *nvHashMap_get_view(nvHashMap *hashmap) {
    nvRefArray *view = nvRefArray_new();
    if (!nvRefArray_valid(view)) return view;

    void *item = NULL;
    size_t index = 0;
    while (nvHashMap_iter(hashmap, &index, &item)) {
        if (nvRefArray_add(view, item) != 0) {
            return view;
        }
    }

    return view;
}

size_t nvHashMap_total_memory_used(nvHashMap *hashmap) {
    size_t size = 0;
    if (!nvHashMap_valid(hashmap)) return size;

    // nvHashMap
    size += sizeof(nvHashMap);

    // nvHashMap->data
    size += hashmap->capacity * hashmap->item_size;

    // nvHashMap->data_state
    size += hashmap->capacity * sizeof(nv_bool);

    // nvHashMap->data_hashes
    size += hashmap->capacity * sizeof(nv_uint64);

    return size;
}