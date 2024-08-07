/*

  This file is a part of the Nova Physics Engine
  project and distributed under the MIT license.

  Copyright © Kadir Aksoy
  https://github.com/kadir014/nova-physics

*/

#include "novaphysics/core/pool.h"


/**
 * @file core/pool.c
 * 
 * @brief Fixed-size memory pool implementation.
 */


nvMemoryPool *nvMemoryPool_new(size_t chunk_size, size_t initial_num_chunks) {
    nvMemoryPool *pool = NV_NEW(nvMemoryPool);
    NV_MEM_CHECK(pool);

    pool->current_size = 0;
    pool->chunk_size = chunk_size;
    pool->pool_size = chunk_size * initial_num_chunks;
    pool->pool = NV_MALLOC(pool->pool_size);
    NV_MEM_CHECK(pool->pool);

    return pool;
}

void nvMemoryPool_free(nvMemoryPool *pool) {
    NV_FREE(pool->pool);
    NV_FREE(pool);
}

int nvMemoryPool_add(nvMemoryPool *pool, void *chunk) {
    // Expand the bool if necessary
    if (pool->current_size * pool->chunk_size >= pool->pool_size) {
        size_t new_pool_size = pool->pool_size * 2;
        void *new_pool = NV_REALLOC(pool->pool, new_pool_size);
        NV_MEM_CHECKI(new_pool);

        pool->pool = new_pool;
        pool->pool_size = new_pool_size;
    }

    memcpy(
        (char *)pool->pool + (pool->current_size++) * pool->chunk_size,
        chunk,
        pool->chunk_size
    );

    return 0;
}

void nvMemoryPool_clear(nvMemoryPool *pool) {
    pool->current_size = 0;
}