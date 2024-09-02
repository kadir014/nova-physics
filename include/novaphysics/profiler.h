/*

  This file is a part of the Nova Physics Engine
  project and distributed under the MIT license.

  Copyright © Kadir Aksoy
  https://github.com/kadir014/nova-physics

*/

#ifndef NOVAPHYSICS_PROFILER_H
#define NOVAPHYSICS_PROFILER_H

#include "novaphysics/internal.h"


/**
 * @file profiler.h
 * 
 * @brief Built-in performance profiler.
 */


/**
 * @brief Timings for parts of one space step in seconds.
 */
typedef struct {
    double step; /**< Time spent in one simulation step. */
    double broadphase; /**< Time spent for broadphase. */
    double broadphase_finalize; /**< Time spent finalizing broadphase. */
    double bvh_free; /**< Time spent destroying BVH-tree. */
    double bvh_build; /**< Time spent building BVH-tree. */
    double bvh_traverse; /**< Time spent traversing BVH-tree. */
    double narrowphase; /**< Time spent for narrowphase. */
    double integrate_accelerations; /**< Time spent integrating accelerations. */
    double presolve; /**< Time spent preparing constraints for solving. */
    double warmstart; /**< Time spent warmstarting constraints. */
    double solve_velocities; /**< Time spent solving velocity constraints. */
    double solve_positions; /**< Time spent for NGS. */
    double integrate_velocities; /**< Time spent integrating velocities. */
} nvProfiler;


static inline void nvProfiler_reset(nvProfiler *profiler) {
    profiler->step = 0.0;
    profiler->broadphase = 0.0;
    profiler->broadphase_finalize = 0.0;
    profiler->bvh_free = 0.0;
    profiler->bvh_build = 0.0;
    profiler->bvh_traverse = 0.0;
    profiler->narrowphase = 0.0;
    profiler->integrate_accelerations = 0.0;
    profiler->presolve = 0.0;
    profiler->warmstart = 0.0;
    profiler->solve_velocities = 0.0;
    profiler->solve_positions = 0.0;
    profiler->integrate_velocities = 0.0;
}


#ifndef NV_ENABLE_PROFILER

    typedef struct {
        double elapsed;
    } nvPrecisionTimer;

    static inline void nvPrecisionTimer_start(nvPrecisionTimer *timer) {}

    static inline double nvPrecisionTimer_stop(nvPrecisionTimer *timer ) {}

#elif defined(NV_WINDOWS)

    #include <windows.h>

    typedef struct {
        double elapsed;
        LARGE_INTEGER _start;
        LARGE_INTEGER _end;
    } nvPrecisionTimer;

    static inline void nvPrecisionTimer_start(nvPrecisionTimer *timer) {
        QueryPerformanceCounter(&timer->_start);
    }

    static inline double nvPrecisionTimer_stop(nvPrecisionTimer *timer ) {
        QueryPerformanceCounter(&timer->_end);

        LARGE_INTEGER frequency;
        QueryPerformanceFrequency(&frequency);

        timer->elapsed = (double)(timer->_end.QuadPart - timer->_start.QuadPart) / (double)frequency.QuadPart;
        return timer->elapsed;
    }

#else

    #include <time.h>
    #include <unistd.h>

    // TODO: On OSX, frequency can be milliseconds instead of nanoseconds
    #define NS_PER_SECOND 1e9

    typedef struct {
        double elapsed;
        struct timespec _start;
        struct timespec _end;
        struct timespec _delta;
    } nvPrecisionTimer;

    static inline void nvPrecisionTimer_start(nvPrecisionTimer *timer) {
        clock_gettime(CLOCK_REALTIME, &timer->_start);
    }

    static inline double nvPrecisionTimer_stop(nvPrecisionTimer *timer) {
        clock_gettime(CLOCK_REALTIME, &timer->_end);

        timer->_delta.tv_nsec = timer->_end.tv_nsec - timer->_start.tv_nsec;
        timer->_delta.tv_sec = timer->_end.tv_sec - timer->_start.tv_sec;

        if (timer->_delta.tv_sec > 0 && timer->_delta.tv_nsec < 0) {
            timer->_delta.tv_nsec += NS_PER_SECOND;
            timer->_delta.tv_sec--;
        }
        else if (timer->_delta.tv_sec < 0 && timer->_delta.tv_nsec > 0) {
            timer->_delta.tv_nsec -= NS_PER_SECOND;
            timer->_delta.tv_sec++;
        }

        timer->elapsed = (double)timer->_delta.tv_nsec / NS_PER_SECOND;
        return timer->elapsed;
    }

#endif


#endif