/*

  This file is a part of the Nova Physics Engine
  project and distributed under the MIT license.

  Copyright © Kadir Aksoy
  https://github.com/kadir014/nova-physics

*/

#ifndef NOVAPHYSICS_THREADING_H
#define NOVAPHYSICS_THREADING_H

#include "novaphysics/internal.h"
#include "novaphysics/array.h"


/**
 * @file threading.h
 * 
 * @brief Cross-platform multi-threading API.
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
} nvMutex;

/**
 * @brief Create new mutex.
 * 
 * @return nvMutex * 
 */
nvMutex *nvMutex_new();

/**
 * @brief Free mutex.
 * 
 * @param mutex Mutex
 */
void nvMutex_free(nvMutex *mutex);

/**
 * @brief Lock the mutex.
 * 
 * @param mutex Mutex
 * @return bool
 */
bool nvMutex_lock(nvMutex *mutex);

/**
 * @brief Unlock the mutex.
 * 
 * @param mutex Mutex
 * @return bool
 */
bool nvMutex_unlock(nvMutex *mutex);


/**
 * @brief Cross-platform event / condition variable implementation.
 */
typedef struct {
    void *_handle;
} nvCondition;

/**
 * @brief Create new condition.
 * 
 * @return nvCondition * 
 */
nvCondition *nvCondition_new();

/**
 * @brief Free condition.
 * 
 * @param condition Condition
 */
void nvCondition_free(nvCondition *cond);

/**
 * @brief Wait for condition to be signaled.
 * 
 * @param cond Condition
 */
void nvCondition_wait(nvCondition *cond, nvMutex *mutex);

/**
 * @brief Signal condition.
 * 
 * @param cond Condition
 */
void nvCondition_signal(nvCondition *cond);


/**
 * @brief Data that is passed to thread worker function.
 */
typedef struct {
    nv_uint64 id; /**< Thread's ID. */
    void *data; /**< User data. */
} nvThreadWorkerData;

/**
 * @brief Cross-platform thread implementation.
 */
typedef struct {
    nv_uint64 id; /**< Unique identity number of the thread. */
    nvThreadWorkerData *worker_data; /**< Data that is going to be passed to worker function. */

    void *_handle; /**< Win32 API object handle. */
} nvThread;

// Thread worker function type
typedef int ( *nvThreadWorker)(nvThreadWorkerData *);

/**
 * @brief Create a new thread and start executing the worker function.
 * 
 * @param func Worker function
 * @param data Data to pass to worker function
 * @return nvThread *
 */
nvThread *nvThread_create(nvThreadWorker func, void *data);

/**
 * @brief Free thread.
 * 
 * @param thread Thread
 */
void nvThread_free(nvThread *thread);

/**
 * @brief Join the thread and wait until the worker is finished.
 * 
 * @param thread Thread
 */
void nvThread_join(nvThread *thread);

/**
 * @brief Join multiple threads and wait until all of the workers are finished.
 * 
 * This uses WaitForMultipleObjects API on Windows.
 * 
 * @param threads Array of thread pointers
 * @param length Length of the array
 */
void nvThread_join_multiple(nvThread **threads, size_t length);


/**
 * @brief Task executor.
 * 
 * The task executor is a background thread pool that continuously runs,
 * ready to execute tasks whenever they are assigned.
 */
typedef struct {
    nvArray *threads; /**< Array of threads. */
    nvArray *data; /**< Array of thread data. */
} nvTaskExecutor;

// Task executor task callback function type
typedef int ( *nvTaskCallback)(void *);

/**
 * @brief Task struct.
 * 
 * You don't manually create this. It is created when tasks are added using
 * @ref nvTaskExecutor_add_task or @ref nvTaskExecutor_add_task_to functions.
 */
typedef struct {
    nvTaskCallback task_func;
    void *data;
} nvTask;

/**
 * @brief Task executor thread data.
 * 
 * This struct is passed to main pool threads of the task executor.
 */
typedef struct {
    bool is_active; /**< Is this thread still running? */
    bool is_busy; /**< Is this thread currently executing a task? */
    bool task_arrived; /**< Did the task arrive to thread? */
    nvTask *task; /**< Task assigned to this thread. */
    nvMutex *task_mutex; /**< Task mutex. */
    nvCondition *task_event; /**< Signaled when a new task is assigned.
                                  Listened by the executor thread. */
    nvCondition *done_event; /**< Signaled when the thread finishes executing task.
                                  Should be listened by other threads. */
} nvTaskExecutorData;

/**
 * @brief Create new task executor.
 * 
 * @param size Number of executor threads to initialize
 * @return nvTaskExecutor *
 */
nvTaskExecutor *nvTaskExecutor_new(size_t size);

/**
 * @brief Free the task executor and its threads.
 * 
 * @param task_executor Task executor
 */
void nvTaskExecutor_free(nvTaskExecutor *task_executor);

/**
 * @brief Stop the task executor and wait for thread pool to finish.
 * 
 * You cannot reinitialize the task executor after stopping it.
 * 
 * @param task_executor Task executor
 */
void nvTaskExecutor_close(nvTaskExecutor *task_executor);

/**
 * @brief Add a task to an available thread.
 * 
 * Returns false if it fails to find an available thread or fails to allocate task.
 * 
 * @param task_executor Task executor
 * @param task_func Task callback function
 * @param task_data Data that is passed to task callback
 * @return bool
 */
bool nvTaskExecutor_add_task(
    nvTaskExecutor *task_executor,
    nvTaskCallback task_func,
    void *task_data
);

/**
 * @brief Add a task to a specific thread in the pool.
 * 
 * Returns false if the thread is busy or fails to allocate task.
 * 
 * @param task_executor Task executor
 * @param task_func Task callback function
 * @param task_data Data that is passed to task callback
 * @param thread_no Index of the thread in the pool
 * @return bool
 */
bool nvTaskExecutor_add_task_to(
    nvTaskExecutor *task_executor,
    nvTaskCallback task_func,
    void *task_data,
    size_t thread_no
);

/**
 * @brief Wait for all tasks to be executed.
 * 
 * @param task_executor Task executor
 */
void nvTaskExecutor_wait_tasks(nvTaskExecutor *task_executor);


#endif