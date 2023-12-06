/*

  This file is a part of the Nova Physics Engine
  project and distributed under the MIT license.

  Copyright © Kadir Aksoy
  https://github.com/kadir014/nova-physics

*/

#ifndef NOVAPHYSICS_THREADING_H
#define NOVAPHYSICS_THREADING_H

#include "novaphysics/internal.h"


/**
 * @file threading.h
 * 
 * @brief Cross-platform multi-threading API.
 * 
 * Multi-threading in Nova Physics is highly experimental and all of the API is
 * subject to change in later versions.
 */


/**
 * @brief Get the number of CPU cores on the system.
 * 
 * @return nv_uint32 
 */
nv_uint32 nv_get_cpu_count();


/**
 * @brief Cross-platform mutex implementation.
 */
typedef struct {
    void *_handle; /**< Win32 API object handle. */
} nv_Mutex;

/**
 * @brief Create new mutex.
 * 
 * @return nv_Mutex * 
 */
nv_Mutex *nv_Mutex_new();

/**
 * @brief Destroy mutex.
 * 
 * @param mutex Mutex
 */
void nv_Mutex_free(nv_Mutex *mutex);

/**
 * @brief Lock the mutex.
 * 
 * @param mutex Mutex
 * @return bool
 */
bool nv_Mutex_lock(nv_Mutex *mutex);

/**
 * @brief Unlock the mutex.
 * 
 * @param mutex Mutex
 * @return bool
 */
bool nv_Mutex_unlock(nv_Mutex *mutex);


/**
 * @brief Data that is passed to thread worker function.
 */
typedef struct {
    nv_uint64 id; /**< Thread's ID. */
    void *data; /**< User data. */
} nv_ThreadWorkerData;

/**
 * @brief Cross-platform thread implementation.
 */
typedef struct {
    nv_uint64 id; /**< Unique identity number of the thread. */
    nv_ThreadWorkerData *worker_data; /**< Data that is going to be passed to worker function. */

    void *_handle; /**< Win32 API object handle. */
} nv_Thread;

// Thread worker function type
typedef int (nv_ThreadWorker)(nv_ThreadWorkerData *data);

/**
 * @brief Create a new thread and start executing the worker function.
 * 
 * @param func Worker function
 * @param data Data to pass to worker function
 * @return nv_Thread *
 */
nv_Thread *nv_Thread_create(nv_ThreadWorker func, void *data);

/**
 * @brief Free thread.
 * 
 * @param thread Thread
 */
void nv_Thread_free(nv_Thread *thread);

/**
 * @brief Join the thread and wait until the worker is finished.
 * 
 * @param thread Thread
 */
void nv_Thread_join(nv_Thread *thread);

/**
 * @brief Join multiple threads and wait until all of the workers are finished.
 * 
 * This uses WaitForMultipleObjects API on Windows.
 * 
 * @param threads Array of thread pointers
 * @param length Length of the array
 */
void nv_Thread_join_multiple(nv_Thread **threads, size_t length);


#endif