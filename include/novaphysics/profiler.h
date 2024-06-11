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
 * @brief Profiler.
 */


typedef struct {
    double step;
    double broadphase;
    double narrowphase;
    double integrate_accelerations;
    double presolve;
    double warmstart;
    double solve_velocities;
    double solve_positions;
    double integrate_velocities;
} nvProfiler;


static inline void nvProfiler_reset(nvProfiler *profiler) {
    profiler->step = 0.0;
    profiler->broadphase = 0.0;
    profiler->narrowphase = 0.0;
    profiler->integrate_accelerations = 0.0;
    profiler->presolve = 0.0;
    profiler->warmstart = 0.0;
    profiler->solve_velocities = 0.0;
    profiler->solve_positions = 0.0;
    profiler->integrate_velocities = 0.0;
}


#ifdef NV_WINDOWS

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