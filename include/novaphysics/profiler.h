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
    nv_float step;
    nv_float integrate_accelerations;
    nv_float broadphase;
    nv_float presolve_collisions;
    nv_float solve_positions;
    nv_float solve_velocities;
    nv_float presolve_constraints;
    nv_float solve_constraints;
    nv_float integrate_velocities;
    nv_float remove_bodies;
} nv_Profiler;


static inline void nv_Profiler_reset(nv_Profiler *profiler) {
    profiler->step = 0.0;
    profiler->integrate_accelerations = 0.0;
    profiler->broadphase = 0.0;
    profiler->presolve_collisions = 0.0;
    profiler->solve_positions = 0.0;
    profiler->solve_velocities = 0.0;
    profiler->presolve_constraints = 0.0;
    profiler->solve_constraints = 0.0;
    profiler->integrate_accelerations = 0.0;
    profiler->remove_bodies = 0.0;
}


#ifdef NV_WINDOWS

    #include <windows.h>

    typedef struct {
        double elapsed;
        LARGE_INTEGER _start;
        LARGE_INTEGER _end;
    } nv_PrecisionTimer;

    static inline void nv_PrecisionTimer_start(nv_PrecisionTimer *timer) {
        QueryPerformanceCounter(&timer->_start);
    }

    static inline double nv_PrecisionTimer_stop(nv_PrecisionTimer *timer ) {
        QueryPerformanceCounter(&timer->_end);

        LARGE_INTEGER frequency;
        QueryPerformanceFrequency(&frequency);

        timer->elapsed = (double)(timer->_end.QuadPart - timer->_start.QuadPart) / (double)frequency.QuadPart;
        return timer->elapsed;
    }

    /**
     * Set timer resolution to minimum for higher precision.
    */
    static inline void nv_set_windows_timer_resolution() {
        TIMECAPS tc;
        if (timeGetDevCaps(&tc, sizeof(tc)) == MMSYSERR_NOERROR) {
            timeBeginPeriod(tc.wPeriodMin);
        }
        else {
            // TODO handle error
        }
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
    } nv_PrecisionTimer;

    static inline void nv_PrecisionTimer_start(nv_PrecisionTimer *timer) {
        clock_gettime(CLOCK_REALTIME, &timer->_start);
    }

    static inline double nv_PrecisionTimer_stop(nv_PrecisionTimer *timer) {
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