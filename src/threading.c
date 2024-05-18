/*

  This file is a part of the Nova Physics Engine
  project and distributed under the MIT license.

  Copyright © Kadir Aksoy
  https://github.com/kadir014/nova-physics

*/

#include "novaphysics/threading.h"


/**
 * @file threading.c
 * 
 * @brief Cross-platform multi-threading API.
 */


/*
    Notes
    -----
    Multi-threading in Nova Physics is highly experimental and all of the API
    is subject to change in later versions.

    On Windows, instead of condition variables I use events, which doesn't
    require a mutex to wait. So the nvCondition_wait function parameters
    and task executor functions gets inconsistent.

    I'm still not satisfied with how the task executor works in general.
    I will need to rewrite it before 1.0.0.
*/


#ifdef NV_WEB

    /* Do not expose any OS-threads for web. */

    nv_uint32 nv_get_cpu_count() {
        return 0;
    }

    nvMutex *nvMutex_new() {
        return NULL;
    }

    void nvMutex_free(nvMutex *mutex) {
        return;
    }

    nv_bool nvMutex_lock(nvMutex *mutex) {
        return false;
    }

    nv_bool nvMutex_unlock(nvMutex *mutex) {
        return false;
    }


    nvCondition *nvCondition_new() {
        return NULL;
    }

    void nvCondition_free(nvCondition *cond) {
        return;
    }

    void nvCondition_wait(nvCondition *cond, nvMutex *mutex) {
        return;
    }

    void nvCondition_signal(nvCondition *cond) {
        return;
    }


    nvThread *nvThread_create(nvThreadWorker func, void *data) {
        return NULL;
    }

    void nvThread_free(nvThread *thread) {
        return;
    }

    void nvThread_join(nvThread *thread) {
        return;
    }

    void nvThread_join_multiple(nvThread **threads, size_t length) {
        return;
    }

#elif defined(NV_WINDOWS)

    /* Win32 implementation of the API. */

    #include <windows.h>


    nv_uint32 nv_get_cpu_count() {
        SYSTEM_INFO info;
        GetSystemInfo(&info);
        return (nv_uint32)info.dwNumberOfProcessors;
    }


    nvMutex *nvMutex_new() {
        nvMutex *mutex = NV_NEW(nvMutex);
        if (!mutex) return NULL;

        mutex->_handle = CreateMutex(
            NULL,  // Default security attributes
            false, // Not owned by any thread by default
            NULL   // Name of the mutex
        );

        if (!mutex->_handle) {
            free(mutex);
            return NULL;
        }

        return mutex;
    }

    void nvMutex_free(nvMutex *mutex) {
        CloseHandle(mutex->_handle);
        free(mutex);
    }

    nv_bool nvMutex_lock(nvMutex *mutex) {
        NV_TRACY_ZONE_START;

        DWORD result = WaitForSingleObject(mutex->_handle, INFINITE);

        NV_TRACY_ZONE_END;
        return result != WAIT_ABANDONED && result != WAIT_FAILED;
    }

    nv_bool nvMutex_unlock(nvMutex *mutex) {
        NV_TRACY_ZONE_START;

        nv_bool result = ReleaseMutex(mutex->_handle);

        NV_TRACY_ZONE_END;
        return result;
    }


    nvCondition *nvCondition_new() {
        nvCondition *cond = NV_NEW(nvCondition);
        if (!cond) return NULL;

        cond->_handle = CreateEvent(
            NULL,  // Default security attributes
            FALSE, // Manual reset or not
            FALSE, // Initial state of the signal
            NULL   // Name of the event
        );

        if (!cond->_handle) {
            free(cond);
            return NULL;
        }

        return cond;
    }

    void nvCondition_free(nvCondition *cond) {
        CloseHandle(cond->_handle);
        free(cond);
    }

    void nvCondition_wait(nvCondition *cond, nvMutex *mutex) {
        WaitForSingleObject(cond->_handle, INFINITE);
    }

    void nvCondition_signal(nvCondition *cond) {
        SetEvent(cond->_handle);
    }


    nvThread *nvThread_create(nvThreadWorker func, void *data) {
        nvThread *thread = NV_NEW(nvThread);
        if (!thread) return NULL;

        thread->worker_data = NV_NEW(nvThreadWorkerData);
        if (!thread->worker_data) return NULL;
        thread->worker_data->data = data;

        // Thread ID pointer, this will be casted to DWORD pointer
        nv_uint64 *thread_id = &thread->id;

        HANDLE thread_handle = CreateThread( 
            NULL,                         // Default security attributes
            0,                            // 0 -> Use default stack size
            (LPTHREAD_START_ROUTINE)func, // Thread worker function
            thread->worker_data,          // Data passed to thread function 
            0,                            // 0 -> Use default creation flags 
            (DWORD *)thread_id            // Pointer to thread identifier
        );

        if (!thread_handle) {
            free(thread);
            return NULL;
        }

        thread->worker_data->id = thread->id;
        thread->_handle = thread_handle;

        return thread;
    }

    void nvThread_free(nvThread *thread) {
        CloseHandle(thread->_handle);
        free(thread->worker_data);
        free(thread);
    }

    void nvThread_join(nvThread *thread) {
        WaitForSingleObject(thread->_handle, INFINITE);
    }

    void nvThread_join_multiple(nvThread **threads, size_t length) {
        #ifdef NV_COMPILER_MSVC

            // MSVC doesn't like VLAs, so malloc
            HANDLE *handles = malloc(sizeof(HANDLE) * length);

        #else

            HANDLE handles[length];

        #endif

        for (size_t i = 0; i < length; i++) {
            handles[i] = threads[i]->_handle;
        }

        WaitForMultipleObjects(length, handles, true, INFINITE);

        #ifdef NV_COMPILER_MSVC

            free(handles);

        #endif
    }

#else

    /* Posix threads implementation of the API. */

    #include <pthread.h>
    #include <unistd.h>


    nv_uint32 nv_get_cpu_count() {
        long cpu_count = sysconf(_SC_NPROCESSORS_ONLN);
        if (cpu_count == -1) return 1;
        return (nv_uint32)cpu_count;
    }


    nvMutex *nvMutex_new() {
        nvMutex *mutex = NV_NEW(nvMutex);
        if (!mutex) return NULL;

        mutex->_handle = NV_NEW(pthread_mutex_t);
        if (!mutex->_handle) {
            free(mutex);
            return NULL;
        }

        if (
            pthread_mutex_init(
                mutex->_handle, // Pointer to mutex
                NULL            // Default creation attributes
            ) != 0
        ) {
            free(mutex->_handle);
            free(mutex);
            return NULL;
        }

        return mutex;
    }

    void nvMutex_free(nvMutex *mutex) {
        pthread_mutex_destroy(mutex->_handle);
        free(mutex->_handle);
        free(mutex);
    }

    nv_bool nvMutex_lock(nvMutex *mutex) {
        return pthread_mutex_lock(mutex->_handle) != 0;
    }

    nv_bool nvMutex_unlock(nvMutex *mutex) {
        return pthread_mutex_unlock(mutex->_handle) != 0;
    }


    nvCondition *nvCondition_new() {
        nvCondition *cond = NV_NEW(nvCondition);
        if (!cond) return NULL;

        cond->_handle = NV_NEW(pthread_cond_t);
        if (!cond->_handle) {
            free(cond);
            return NULL;
        }

        if (
            pthread_cond_init(
                cond->_handle, // Pointer to condition variable
                NULL           // Default creation attributes
            ) != 0
        ) {
            free(cond->_handle);
            free(cond);
            return NULL;
        }

        return cond;
    }

    void nvCondition_free(nvCondition *cond) {
        pthread_cond_destroy(cond->_handle);
        free(cond->_handle);
        free(cond);
    }

    void nvCondition_wait(nvCondition *cond, nvMutex *mutex) {
        pthread_cond_wait(cond->_handle, mutex->_handle);
    }

    void nvCondition_signal(nvCondition *cond) {
        pthread_cond_signal(cond->_handle);
    }


    nvThread *nvThread_create(nvThreadWorker func, void *data) {
        nvThread *thread = NV_NEW(nvThread);
        if (!thread) return NULL;

        thread->worker_data = NV_NEW(nvThreadWorkerData);
        if (!thread->worker_data) return NULL;
        thread->worker_data->data = data;

        pthread_create(
            &thread->id,              // Pointer to thread identifier
            NULL,                     // Default creation attributes
            (void * (*)(void *))func, // Thread worker function
            thread->worker_data       // Data passed to worker function
        );

        thread->worker_data->id = thread->id;

        return thread;
    }

    void nvThread_free(nvThread *thread) {
        free(thread->worker_data);
        free(thread);
    }

    void nvThread_join(nvThread *thread) {
    }

    void nvThread_join_multiple(nvThread **threads, size_t length) {
        for (size_t i = 0; i < length; i++) {
            pthread_join(threads[i]->id, NULL);
        }
    }

#endif


static int nvTaskExecutor_main(nvThreadWorkerData *worker_data) {
    nvTaskExecutorData *data = worker_data->data;
    data->is_active = true;

    while (data->is_active) {
        
        #ifdef NV_WINDOWS

            nvCondition_wait(data->task_event, data->task_mutex);
            data->is_busy = true;
            data->task_arrived = true;

            if (data->task) {
                data->task->task_func(data->task->data);
                free(data->task);
                data->task = NULL;
            }

            data->is_busy = false;
            nvCondition_signal(data->done_event);

        #else

            nvMutex_lock(data->task_mutex);
            nvCondition_wait(data->task_event, data->task_mutex);
            data->is_busy = true;
            data->task_arrived = true;
            nvMutex_unlock(data->task_mutex);

            nvMutex_lock(data->task_mutex);
            if (data->task) {
                data->task->task_func(data->task->data);
                free(data->task);
                data->task = NULL;
            }
            nvMutex_unlock(data->task_mutex);

            nvMutex_lock(data->task_mutex);
            data->is_busy = false;
            nvCondition_signal(data->done_event);
            nvMutex_unlock(data->task_mutex);

        #endif
    }

    return 0;
}

nvTaskExecutor *nvTaskExecutor_new(size_t size) {
    nvTaskExecutor *task_executor = NV_NEW(nvTaskExecutor);
    if (!task_executor) return NULL;

    task_executor->threads = nvArray_new();
    task_executor->data = nvArray_new();

    for (size_t i = 0; i < size; i++) {
        nvTaskExecutorData *thread_data = NV_NEW(nvTaskExecutorData);
        if (!thread_data) return NULL;

        thread_data->is_active = true;
        thread_data->is_busy = false;
        thread_data->task_arrived = false;
        thread_data->task = NULL;
        thread_data->task_mutex = nvMutex_new();
        thread_data->task_event = nvCondition_new();
        thread_data->done_event = nvCondition_new();
        nvArray_add(task_executor->data, thread_data);

        nvThread *thread = nvThread_create(nvTaskExecutor_main, thread_data);
        nvArray_add(task_executor->threads, thread);
    }

    return task_executor;
}

void nvTaskExecutor_free(nvTaskExecutor *task_executor) {
    nvArray_free(task_executor->threads);
    for (size_t i = 0; i < task_executor->data->size; i++) {
        nvTaskExecutorData *data = task_executor->data->data[i];
        free(data->task);
        nvMutex_free(data->task_mutex);
        nvCondition_free(data->task_event);
        nvCondition_free(data->done_event);
    }
    nvArray_free_each(task_executor->data, free);
    nvArray_free(task_executor->data);
}

void nvTaskExecutor_close(nvTaskExecutor *task_executor) {
    for (size_t i = 0; i < task_executor->data->size; i++) {
        nvTaskExecutorData *data = task_executor->data->data[i];
        
        nvMutex_lock(data->task_mutex);
        data->is_active = false;
        data->task = NULL;
        nvCondition_signal(data->task_event);
        nvMutex_unlock(data->task_mutex);
        
    }

    nvThread_join_multiple(
        (nvThread **)task_executor->threads->data,
        task_executor->threads->size
    );
}

nv_bool nvTaskExecutor_add_task(
    nvTaskExecutor *task_executor,
    nvTaskCallback task_func,
    void *task_data
) {
    for (size_t i = 0; i < task_executor->threads->size; i++) {
        if (
            nvTaskExecutor_add_task_to(
                task_executor,
                task_func,
                task_data,
                i
            )
        )
            return true;
    }

    return false;
}

nv_bool nvTaskExecutor_add_task_to(
    nvTaskExecutor *task_executor,
    nvTaskCallback task_func,
    void *task_data,
    size_t thread_no
) {
    nvTaskExecutorData *data = task_executor->data->data[thread_no];

    if (!data->task) {
        nvTask *task = NV_NEW(nvTask);
        if (!task) return false;

        task->task_func = task_func;
        task->data = task_data;
        
        #ifdef NV_WINDOWS

            data->task = task;
            data->task_arrived = false;

            nvCondition_signal(data->task_event);

        #else

            nvMutex_lock(data->task_mutex);

            data->task = task;
            data->task_arrived = false;

            nvCondition_signal(data->task_event);

            nvMutex_unlock(data->task_mutex);

        #endif

        return true;
    }

    return false;
}

void nvTaskExecutor_wait_tasks(nvTaskExecutor *task_executor) {
    #ifdef NV_WINDOWS

        for (size_t i = 0; i < task_executor->threads->size; i++) {
            nvTaskExecutorData *data = task_executor->data->data[i];
            nvCondition_wait(data->done_event, data->task_mutex);
        }

    #else

        for (size_t i = 0; i < task_executor->threads->size; i++) {
            nvTaskExecutorData *data = task_executor->data->data[i];

            // Busy wait if the task hasn't arrived yet
            while (!data->task_arrived) {}

            while (data->is_busy) {
                nvMutex_lock(data->task_mutex);
                nvCondition_wait(data->done_event, data->task_mutex);
                nvMutex_unlock(data->task_mutex);
            }
        }
    
    #endif
}