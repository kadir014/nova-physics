/*

  This file is a part of the Nova Physics Engine
  project and distributed under the MIT license.

  Copyright © Kadir Aksoy
  https://github.com/kadir014/nova-physics

*/

#include "novaphysics/threading.h"
#include "novaphysics/constants.h"


/**
 * @file threading.c
 * 
 * @brief Cross-platform multi-threading API.
 * 
 * Multi-threading in Nova Physics is highly experimental and all of the API is
 * subject to change in later versions.
 */


#ifdef NV_WINDOWS

    /* Win32 implementation of the API. */

    #include <windows.h>


    nv_uint32 nv_get_cpu_count() {
        SYSTEM_INFO info;
        GetSystemInfo(&info);
        return (nv_uint32)info.dwNumberOfProcessors;
    }


    nv_Mutex *nv_Mutex_new() {
        nv_Mutex *mutex = NV_NEW(nv_Mutex);
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

    void nv_Mutex_free(nv_Mutex *mutex) {
        CloseHandle(mutex->_handle);
        free(mutex);
    }

    bool nv_Mutex_lock(nv_Mutex *mutex) {
        DWORD result = WaitForSingleObject(mutex->_handle, INFINITE);
        return result != WAIT_ABANDONED && result != WAIT_FAILED;
    }

    bool nv_Mutex_unlock(nv_Mutex *mutex) {
        return ReleaseMutex(mutex->_handle);
    }


    nv_Thread *nv_Thread_create(nv_ThreadWorker func, void *data) {
        nv_Thread *thread = NV_NEW(nv_Thread);
        if (!thread) return NULL;

        thread->worker_data = NV_NEW(nv_ThreadWorkerData);
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

    void nv_Thread_free(nv_Thread *thread) {
        CloseHandle(thread->_handle);
        free(thread);
    }

    void nv_Thread_join(nv_Thread *thread) {
        WaitForSingleObject(thread->_handle, INFINITE);
    }

    void nv_Thread_join_multiple(nv_Thread **threads, size_t length) {
        // MSVC doesn't like VLAs, so malloc
        HANDLE *handles = malloc(sizeof(HANDLE) * length);

        for (size_t i = 0; i < length; i++) {
            handles[i] = threads[i]->_handle;
        }

        WaitForMultipleObjects(length, handles, true, INFINITE);

        free(handles);
    }

#else

    // Posix threads implementation will be here.

#endif