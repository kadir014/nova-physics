/*

  This file is a part of the Nova Physics Engine
  project and distributed under the MIT license.

  Copyright © Kadir Aksoy
  https://github.com/kadir014/nova-physics

*/

#include <string.h>
#include "novaphysics/internal.h"
#include "novaphysics/hashmap.h"
#include "novaphysics/constants.h"
#include "novaphysics/math.h"


/**
 * @file hashmap.c
 * 
 * @brief Hash map implementation.
 * 
 * Thanks to @tidwall for their great hash map implementation that served as
 * a strong foundation for this one.
 * (https://github.com/tidwall/hashmap.c/tree/master)
 */


static inline nv_HashMapBucket *bucket_at(nv_HashMap *map, size_t index) {
    return (nv_HashMapBucket *)(((char *)map->buckets) + (map->bucketsz * index));
}

static inline void *bucket_item(nv_HashMapBucket *entry) {
    return ((char *)entry) + sizeof(nv_HashMapBucket);
}

static inline nv_uint64 clip_hash(nv_uint64 hash) {
    return hash & 0xFFFFFFFFFFFF;
}

static inline bool resize(nv_HashMap *hashmap, size_t new_cap) {
    nv_HashMap *hashmap2 = nv_HashMap_new(hashmap->elsize, new_cap, hashmap->hash_func);
    if (!hashmap2) return false;

    for (size_t i = 0; i < hashmap->nbuckets; i++) {
        nv_HashMapBucket *entry = bucket_at(hashmap, i);
        if (!entry->dib)continue;
        entry->dib = 1;

        size_t j = entry->hash & hashmap2->mask;
        while (true) {
            nv_HashMapBucket *bucket = bucket_at(hashmap2, j);

            if (bucket->dib == 0) {
                memcpy(bucket, entry, hashmap->bucketsz);
                break;
            }

            if (bucket->dib < entry->dib) {
                memcpy(hashmap2->spare, bucket, hashmap->bucketsz);
                memcpy(bucket, entry, hashmap->bucketsz);
                memcpy(entry, hashmap2->spare, hashmap->bucketsz);
            }

            j = (j + 1) & hashmap2->mask;
            entry->dib += 1;
        }
    }

    free(hashmap->buckets);

    hashmap->buckets = hashmap2->buckets;
    hashmap->nbuckets = hashmap2->nbuckets;
    hashmap->mask = hashmap2->mask;
    hashmap->growat = hashmap2->growat;
    hashmap->shrinkat = hashmap2->shrinkat;

    free(hashmap2);

    return true;
}


nv_HashMap *nv_HashMap_new(
    size_t item_size,
    size_t cap,
    nv_uint64 (*hash_func)(void *item)
) {
    // Capacity must be a power of 2 and higher than the default value.
    size_t ncap = NV_HASHMAP_CAPACITY;
    if (cap < ncap) cap = ncap;
    else {
        while (ncap < cap) ncap *= 2;
        cap = ncap;
    }

    size_t bucketsz = sizeof(nv_HashMapBucket) + item_size;
    while (bucketsz & (sizeof(uintptr_t) - 1)) {
        bucketsz++;
    }

    size_t size = sizeof(nv_HashMap)+bucketsz*2;
    nv_HashMap *hashmap = malloc(size);
    if (!hashmap) return NULL;

    hashmap->count = 0;
    hashmap->oom = false;
    hashmap->elsize = item_size;
    hashmap->hash_func = hash_func;
    hashmap->bucketsz = bucketsz;
    hashmap->spare = ((char*)hashmap) + sizeof(nv_HashMap);
    hashmap->edata = (char*)hashmap->spare + bucketsz;
    hashmap->cap = cap;
    hashmap->nbuckets = cap;
    hashmap->mask = hashmap->nbuckets - 1;

    hashmap->buckets = malloc(hashmap->bucketsz * hashmap->nbuckets);
    if (!hashmap->buckets) {
        free(hashmap);
        return NULL;
    }
    memset(hashmap->buckets, 0, hashmap->bucketsz * hashmap->nbuckets);

    hashmap->growpower = 1;
    hashmap->growat = hashmap->nbuckets * 0.6;
    hashmap->shrinkat = hashmap->nbuckets * 0.10;

    return hashmap;
}

void nv_HashMap_free(nv_HashMap *hashmap) {
    free(hashmap->buckets);
    free(hashmap);
}

void nv_HashMap_clear(nv_HashMap *hashmap) {
    hashmap->count = 0;
    if (hashmap->nbuckets != hashmap->cap) {
        void *new_buckets = malloc(hashmap->bucketsz*hashmap->cap);
        if (new_buckets) {
            free(hashmap->buckets);
            hashmap->buckets = new_buckets;
        }
        hashmap->nbuckets = hashmap->cap;
    }

    memset(hashmap->buckets, 0, hashmap->bucketsz*hashmap->nbuckets);
    hashmap->mask = hashmap->nbuckets - 1;
    hashmap->growat = hashmap->nbuckets * 0.75;
    hashmap->shrinkat = hashmap->nbuckets * 0.1;
}

void *nv_HashMap_set(nv_HashMap *hashmap, void *item) {
    nv_uint64 hash = clip_hash(hashmap->hash_func(item));
    hash = clip_hash(hash);

    // Does adding one more entry overflow memory?
    hashmap->oom = false;
    if (hashmap->count == hashmap->growat) {
        if (!resize(hashmap, hashmap->nbuckets*(1<<hashmap->growpower))) {
            hashmap->oom = true;
            return NULL;
        }
    }

    nv_HashMapBucket *entry = hashmap->edata;
    entry->hash = hash;
    entry->dib = 1;
    void *eitem = bucket_item(entry);
    memcpy(eitem, item, hashmap->elsize);

    void *bitem;
    size_t i = entry->hash & hashmap->mask;
    while (true) {
        nv_HashMapBucket *bucket = bucket_at(hashmap, i);

        if (bucket->dib == 0) {
            memcpy(bucket, entry, hashmap->bucketsz);
            hashmap->count++;
            return NULL;
        }

        bitem = bucket_item(bucket);

        if (entry->hash == bucket->hash)
        {
            memcpy(hashmap->spare, bitem, hashmap->elsize);
            memcpy(bitem, eitem, hashmap->elsize);
            return hashmap->spare;
        }

        if (bucket->dib < entry->dib) {
            memcpy(hashmap->spare, bucket, hashmap->bucketsz);
            memcpy(bucket, entry, hashmap->bucketsz);
            memcpy(entry, hashmap->spare, hashmap->bucketsz);
            eitem = bucket_item(entry);
        }

        i = (i + 1) & hashmap->mask;
        entry->dib += 1;
    }
}

void *nv_HashMap_get(nv_HashMap *hashmap, void *key) {
    nv_uint64 hash = clip_hash(hashmap->hash_func(key));
    hash = clip_hash(hash);

    size_t i = hash & hashmap->mask;
    while (true) {
        nv_HashMapBucket *bucket = bucket_at(hashmap, i);
        if (!bucket->dib) return NULL;
        if (bucket->hash == hash) {
            void *bitem = bucket_item(bucket);
            if (bitem != NULL) {
                return bitem;
            }
        }
        i = (i + 1) & hashmap->mask;
    }
}

void *nv_HashMap_remove(nv_HashMap *hashmap, void *key) {
    nv_uint64 hash = clip_hash(hashmap->hash_func(key));
    hash = clip_hash(hash);

    hashmap->oom = false;
    size_t i = hash & hashmap->mask;
    while (true) {
        nv_HashMapBucket *bucket = bucket_at(hashmap, i);
        if (!bucket->dib) return NULL;

        void *bitem = bucket_item(bucket);
        if (bucket->hash == hash) {
            memcpy(hashmap->spare, bitem, hashmap->elsize);
            bucket->dib = 0;
            while (true) {
                nv_HashMapBucket *prev = bucket;
                i = (i + 1) & hashmap->mask;
                bucket = bucket_at(hashmap, i);
                if (bucket->dib <= 1) {
                    prev->dib = 0;
                    break;
                }
                memcpy(prev, bucket, hashmap->bucketsz);
                prev->dib--;
            }
            hashmap->count--;
            if (hashmap->nbuckets > hashmap->cap && hashmap->count <= hashmap->shrinkat) {
                // It's OK for the resize operation to fail to allocate enough
                // memory because shriking does not change the integrity of the data.
                resize(hashmap, hashmap->nbuckets / 2);
            }
            return hashmap->spare;
        }
        i = (i + 1) & hashmap->mask;
    }
}

bool nv_HashMap_iter(nv_HashMap *hashmap, size_t *index, void **item) {
    nv_HashMapBucket *bucket;
    do {
        if (*index >= hashmap->nbuckets) return false;
        bucket = bucket_at(hashmap, *index);
        (*index)++;
    } while (!bucket->dib);

    *item = bucket_item(bucket);
    
    return true;
}