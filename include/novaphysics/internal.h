/*

  This file is a part of the Nova Physics Engine
  project and distributed under the MIT license.

  Copyright © Kadir Aksoy
  https://github.com/kadir014/nova-physics

*/

#ifndef NOVAPHYSICS_INTERNAL_H
#define NOVAPHYSICS_INTERNAL_H

#include <stdlib.h>
#include <string.h>


/**
 * @file internal.h
 * 
 * @brief Nova Physics internal API header.
 */


#if defined(_WIN32) || defined(__WIN32__) || defined(__WINDOWS__)

    #define NV_WINDOWS

#endif

#if defined(__EMSCRIPTEN__) || defined(__wasi__)

    #define NV_WEB

#endif

#if defined(__GNUC__) || defined(__MINGW32__) || defined(__MINGW64__)

    #define NV_COMPILER_GCC

#elif defined(_MSC_VER) || defined(_MSC_FULL_VER) || defined(_MSVC_LANG)

    #define NV_COMPILER_MSVC

#endif


#include "novaphysics/types.h"
#include "novaphysics/constants.h"
#include "novaphysics/core/error.h"


// Align memory as given byte range. Needed for some SIMD functions.

#if defined(NV_COMPILER_GCC)

    #define NV_ALIGNED_AS(x) __attribute__((aligned(x)))

#elif defined(NV_COMPILER_MSVC)

    #define NV_ALIGNED_AS(x) __declspec(align(x))

#else

    #define NV_ALIGNED_AS(x)

#endif


/*
    Profiling macros.
*/
#ifdef NV_ENABLE_PROFILER

    #define NV_PROFILER_START(timer) (nvPrecisionTimer_start(&timer))
    #define NV_PROFILER_STOP(timer, field) (field = nvPrecisionTimer_stop(&timer))

#else

    #define NV_PROFILER_START(timer)
    #define NV_PROFILER_STOP(timer, field)

#endif


// This is forward declared to prevent circular includes
struct nvSpace;


#define NV_MEM_CHECK(object) {                      \
    if (!(object)) {                                \
        nv_set_error("Failed to allocate memory."); \
        return NULL;                                \
    }                                               \
}                                                   \

#define NV_MEM_CHECKI(object) {                     \
    if (!(object)) {                                \
        nv_set_error("Failed to allocate memory."); \
        return 1;                                   \
    }                                               \
}                                                   \


/*
    Internal Tracy Profiler macros.
*/
#ifdef TRACY_ENABLE

    #include "TracyC.h"

    #define NV_TRACY_ZONE_START TracyCZone(_tracy_zone, true)
    #define NV_TRACY_ZONE_END TracyCZoneEnd(_tracy_zone)
    #define NV_TRACY_FRAMEMARK TracyCFrameMark

    static inline void *NV_MALLOC(size_t size) {
        void *ptr = malloc(size);
        TracyCAlloc(ptr, size);
        return ptr;
    }

    static inline void *NV_REALLOC(void *ptr, size_t new_size) {
        if (ptr) {
            TracyCFree(ptr);
        }

        void *new_ptr = realloc(ptr, new_size);
        TracyCAlloc(new_ptr, new_size);

        return new_ptr;
    }

    static inline void NV_FREE(void *ptr) {
        TracyCFree(ptr);
        free(ptr);
    }

#else

    #define NV_TRACY_ZONE_START
    #define NV_TRACY_ZONE_END
    #define NV_TRACY_FRAMEMARK

    #define NV_MALLOC(size) malloc(size)
    #define NV_REALLOC(ptr, new_size) realloc(ptr, new_size)
    #define NV_FREE(ptr) free(ptr)

#endif


#define NV_NEW(type) ((type *)NV_MALLOC(sizeof(type)))


#endif