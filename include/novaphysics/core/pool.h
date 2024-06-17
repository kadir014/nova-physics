/*

  This file is a part of the Nova Physics Engine
  project and distributed under the MIT license.

  Copyright © Kadir Aksoy
  https://github.com/kadir014/nova-physics

*/

#ifndef NOVAPHYSICS_MEMORYPOOL_H
#define NOVAPHYSICS_MEMORYPOOL_H

#include "novaphysics/internal.h"


/**
 * @file core/pool.h
 * 
 * @brief Fixed-size memory pool implementation.
 */


/**
 * @brief Fixed-size memory pool implementation.
 */
typedef struct {
    size_t chunk_size; /**< Fixed chunk size. */
    size_t pool_size; /**< Pool size. */
    size_t current_size; /**< Current number of chunks. */
    void *pool; /**< Pool block. */
} nvMemoryPool;

/**
 * @brief Create new memory pool.
 * 
 * Returns `NULL` on error. Use @ref nv_get_error to get more information.
 * 
 * @param chunk_size Fixed chunk size
 * @param initial_num_chunks Initial number of chunks
 * @return nvMemoryPool *
 */
nvMemoryPool *nvMemoryPool_new(size_t chunk_size, size_t initial_num_chunks);

/**
 * @brief Free memory pool.
 * 
 * It's safe to pass `NULL` to this function.
 * 
 * @param pool Memory pool
 */
void nvMemoryPool_free(nvMemoryPool *pool);

/**
 * @brief Add a new chunk to the pool.
 * 
 * Returns non-zero on error. Use @ref nv_get_error to get more information.
 * 
 * @param pool Memory pool
 * @param chunk Chunk data
 * @return int Status
 */
int nvMemoryPool_add(nvMemoryPool *pool, void *chunk);

/**
 * @brief Clear pool.
 * 
 * @param pool Memory pool
 */
void nvMemoryPool_clear(nvMemoryPool *pool);


#endif