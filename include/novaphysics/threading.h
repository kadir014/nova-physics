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
nv_uint32 get_cpu_count();


/**
 * @brief Cross-platform mutex implementation.
 */
typedef struct {
    void *_handle; /**< Win32 API object handle. */
} Mutex;

/**
 * @brief Create new mutex.
 * 
 * @return Mutex * 
 */
Mutex *Mutex_new();

/**
 * @brief Destroy mutex.
 * 
 * @param mutex Mutex
 */
void Mutex_free(Mutex *mutex);

/**
 * @brief Lock the mutex.
 * 
 * @param mutex Mutex
 * @return bool
 */
bool Mutex_lock(Mutex *mutex);

/**
 * @brief Unlock the mutex.
 * 
 * @param mutex Mutex
 * @return bool
 */
bool Mutex_unlock(Mutex *mutex);


/**
 * @brief Data that is passed to thread worker function.
 */
typedef struct {
    nv_uint64 id; /**< Thread's ID. */
    void *data; /**< User data. */
} ThreadWorkerData;

/**
 * @brief Cross-platform thread implementation.
 */
typedef struct {
    nv_uint64 id; /**< Unique identity number of the thread. */
    ThreadWorkerData *worker_data; /**< Data that is going to be passed to worker function. */

    void *_handle; /**< Win32 API object handle. */
} Thread;

// Thread worker function type
typedef nv_uint64 (ThreadWorker)(ThreadWorkerData *data);

/**
 * @brief Create a new thread and start executing the worker function.
 * 
 * @param func Worker function
 * @param data Data to pass to worker function
 * @return nv_Thread *
 */
Thread *Thread_create(ThreadWorker func, void *data);

/**
 * @brief Free thread.
 * 
 * @param thread Thread
 */
void Thread_free(Thread *thread);

/**
 * @brief Join the thread and wait until the worker is finished.
 * 
 * @param thread Thread
 */
void Thread_join(Thread *thread);

/**
 * @brief Join mulitple threads and wait until all of the workers are finished.
 * 
 * This uses WaitForMultipleObjects API on Windows.
 * 
 * @param threads Array of thread pointers
 * @param length Length of the array
 */
void Thread_join_multiple(Thread **threads, size_t length);


#endif