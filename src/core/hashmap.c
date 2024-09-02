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
 * 
 * Thanks to @tidwall for their great hash map implementation that served as
 * a strong foundation for this one.
 * (https://github.com/tidwall/hashmap.c)
 */


static inline nvHashMapBucket *_nvHashMap_get_bucket_at(
    nvHashMap *map,
    size_t index
) {
    return (nvHashMapBucket *)(((char *)map->buckets) + (map->bucketsz * index));
}

static inline void *_nvHashMap_get_bucket_item(nvHashMapBucket *entry) {
    return ((char *)entry) + sizeof(nvHashMapBucket);
}

static inline nv_uint64 _nvHashMap_clip(nv_uint64 hash) {
    return hash & 0xFFFFFFFFFFFF;
}

static inline nv_bool _nvHashMap_resize(nvHashMap *hashmap, size_t new_cap) {
    nvHashMap *hashmap2 = nvHashMap_new(hashmap->elsize, new_cap, hashmap->hash_func);
    if (!hashmap2) return false;

    for (size_t i = 0; i < hashmap->nbuckets; i++) {
        nvHashMapBucket *entry = _nvHashMap_get_bucket_at(hashmap, i);
        if (!entry->dib)continue;
        entry->dib = 1;

        size_t j = entry->hash & hashmap2->mask;
        while (true) {
            nvHashMapBucket *bucket = _nvHashMap_get_bucket_at(hashmap2, j);

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

    NV_FREE(hashmap->buckets);

    hashmap->buckets = hashmap2->buckets;
    hashmap->nbuckets = hashmap2->nbuckets;
    hashmap->mask = hashmap2->mask;
    hashmap->growat = hashmap2->growat;
    hashmap->shrinkat = hashmap2->shrinkat;

    NV_FREE(hashmap2);

    return true;
}


nvHashMap *nvHashMap_new(
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

    size_t bucketsz = sizeof(nvHashMapBucket) + item_size;
    while (bucketsz & (sizeof(uintptr_t) - 1)) {
        bucketsz++;
    }

    size_t size = sizeof(nvHashMap)+bucketsz*2;
    nvHashMap *hashmap = NV_MALLOC(size);
    NV_MEM_CHECK(hashmap);

    hashmap->count = 0;
    hashmap->oom = false;
    hashmap->elsize = item_size;
    hashmap->hash_func = hash_func;
    hashmap->bucketsz = bucketsz;
    hashmap->spare = ((char*)hashmap) + sizeof(nvHashMap);
    hashmap->edata = (char*)hashmap->spare + bucketsz;
    hashmap->cap = cap;
    hashmap->nbuckets = cap;
    hashmap->mask = hashmap->nbuckets - 1;

    hashmap->buckets = NV_MALLOC(hashmap->bucketsz * hashmap->nbuckets);
    if (!hashmap->buckets) {
        NV_FREE(hashmap);
        nv_set_error("Failed to allocate memory.");
        return NULL;
    }
    memset(hashmap->buckets, 0, hashmap->bucketsz * hashmap->nbuckets);

    hashmap->growpower = 1;
    hashmap->growat = (size_t)(hashmap->nbuckets * 0.6);
    hashmap->shrinkat = (size_t)(hashmap->nbuckets * 0.1);

    return hashmap;
}

void nvHashMap_free(nvHashMap *hashmap) {
    NV_FREE(hashmap->buckets);
    NV_FREE(hashmap);
}

void nvHashMap_clear(nvHashMap *hashmap) {
    NV_TRACY_ZONE_START;

    hashmap->count = 0;
    if (hashmap->nbuckets != hashmap->cap) {
        void *new_buckets = NV_MALLOC(hashmap->bucketsz*hashmap->cap);
        if (new_buckets) {
            NV_FREE(hashmap->buckets);
            hashmap->buckets = new_buckets;
        }
        hashmap->nbuckets = hashmap->cap;
    }

    memset(hashmap->buckets, 0, hashmap->bucketsz*hashmap->nbuckets);
    hashmap->mask = hashmap->nbuckets - 1;
    hashmap->growat = (size_t)(hashmap->nbuckets * 0.75); // Why does growing factor change?
    hashmap->shrinkat = (size_t)(hashmap->nbuckets * 0.1);

    NV_TRACY_ZONE_END;
}

void *nvHashMap_set(nvHashMap *hashmap, void *item) {
    NV_TRACY_ZONE_START;

    nv_uint64 hash = _nvHashMap_clip(hashmap->hash_func(item));
    hash = _nvHashMap_clip(hash);

    // Does adding one more entry overflow memory?
    hashmap->oom = false;
    if (hashmap->count == hashmap->growat) {
        if (!_nvHashMap_resize(hashmap, hashmap->nbuckets * (1<<hashmap->growpower))) {
            hashmap->oom = true;
            NV_TRACY_ZONE_END;
            return NULL;
        }
    }

    nvHashMapBucket *entry = hashmap->edata;
    entry->hash = hash;
    entry->dib = 1;
    void *eitem = _nvHashMap_get_bucket_item(entry);
    memcpy(eitem, item, hashmap->elsize);

    void *bitem;
    size_t i = entry->hash & hashmap->mask;
    while (true) {
        nvHashMapBucket *bucket = _nvHashMap_get_bucket_at(hashmap, i);

        if (bucket->dib == 0) {
            memcpy(bucket, entry, hashmap->bucketsz);
            hashmap->count++;
            NV_TRACY_ZONE_END;
            return NULL;
        }

        bitem = _nvHashMap_get_bucket_item(bucket);

        if (entry->hash == bucket->hash) {
            memcpy(hashmap->spare, bitem, hashmap->elsize);
            memcpy(bitem, eitem, hashmap->elsize);
            NV_TRACY_ZONE_END;
            return hashmap->spare;
        }

        if (bucket->dib < entry->dib) {
            memcpy(hashmap->spare, bucket, hashmap->bucketsz);
            memcpy(bucket, entry, hashmap->bucketsz);
            memcpy(entry, hashmap->spare, hashmap->bucketsz);
            eitem = _nvHashMap_get_bucket_item(entry);
        }

        i = (i + 1) & hashmap->mask;
        entry->dib += 1;
    }

    NV_TRACY_ZONE_END;
}

void *nvHashMap_get(nvHashMap *hashmap, void *key) {
    NV_TRACY_ZONE_START;

    nv_uint64 hash = _nvHashMap_clip(hashmap->hash_func(key));
    hash = _nvHashMap_clip(hash);

    size_t i = hash & hashmap->mask;
    while (true) {
        nvHashMapBucket *bucket = _nvHashMap_get_bucket_at(hashmap, i);

        if (!bucket->dib) {
            NV_TRACY_ZONE_END;
            return NULL;
        }

        if (bucket->hash == hash) {
            void *bitem = _nvHashMap_get_bucket_item(bucket);
            if (bitem != NULL) {
                NV_TRACY_ZONE_END;
                return bitem;
            }
        }

        i = (i + 1) & hashmap->mask;
    }
    NV_TRACY_ZONE_END;
}

void *nvHashMap_remove(nvHashMap *hashmap, void *key) {
    NV_TRACY_ZONE_START;

    nv_uint64 hash = _nvHashMap_clip(hashmap->hash_func(key));
    hash = _nvHashMap_clip(hash);

    hashmap->oom = false;
    size_t i = hash & hashmap->mask;
    
    while (true) {
        nvHashMapBucket *bucket = _nvHashMap_get_bucket_at(hashmap, i);
        if (!bucket->dib) {
            NV_TRACY_ZONE_END;
            return NULL;
        }

        void *bitem = _nvHashMap_get_bucket_item(bucket);
        if (bucket->hash == hash) {
            memcpy(hashmap->spare, bitem, hashmap->elsize);
            bucket->dib = 0;
            while (true) {
                nvHashMapBucket *prev = bucket;
                i = (i + 1) & hashmap->mask;
                
                bucket = _nvHashMap_get_bucket_at(hashmap, i);
                if (bucket->dib <= 1) {
                    prev->dib = 0;
                    break;
                }

                memcpy(prev, bucket, hashmap->bucketsz);
                prev->dib--;
            }

            hashmap->count--;

            if (hashmap->nbuckets > hashmap->cap && hashmap->count <= hashmap->shrinkat) {
                // It's OK for the _nvHashMap_resize operation to fail to allocate enough
                // memory because shriking does not change the integrity of the data.
                _nvHashMap_resize(hashmap, hashmap->nbuckets / 2);
            }

            NV_TRACY_ZONE_END;
            return hashmap->spare;
        }
        i = (i + 1) & hashmap->mask;
    }

    NV_TRACY_ZONE_END;
}

nv_bool nvHashMap_iter(nvHashMap *hashmap, size_t *index, void **item) {
    NV_TRACY_ZONE_START;

    nvHashMapBucket *bucket;
    do {
        if (*index >= hashmap->nbuckets) {
            NV_TRACY_ZONE_END;
            return false;
        }
        bucket = _nvHashMap_get_bucket_at(hashmap, *index);
        (*index)++;
    } while (!bucket->dib);

    *item = _nvHashMap_get_bucket_item(bucket);
    
    NV_TRACY_ZONE_END;
    return true;
}