/*

  This file is a part of the Nova Physics Engine
  project and distributed under the MIT license.

  Copyright © Kadir Aksoy
  https://github.com/kadir014/nova-physics

*/

#ifndef NV_BENCHMARK_BASE_H
#define NV_BENCHMARK_BASE_H

#include <math.h>
#include <stdlib.h>
#include <stdio.h>


typedef struct {
    double min;
    double max;
    double avg;
    double stdev;
} Stats;

/**
 * Calculate minimum, maximum, mean and standard deviation values
*/
void calculate_stats(Stats *stats, double *times, size_t n) {
    double sum = 0.0;
    stats->min = INFINITY;
    stats->max = -INFINITY;

    for (size_t i = 0; i < n; i++) {
        sum += times[i];
        if (times[i] < stats->min) stats->min = times[i];
        if (times[i] > stats->max) stats->max = times[i];
    }

    stats->avg = sum / (double)n;

    stats->stdev = 0.0;

    for (size_t i = 0; i < n; i++) {
        double dist = (times[i] - stats->avg) * (times[i] - stats->avg);
        stats->stdev += dist;
    }

    stats->stdev = sqrt(stats->stdev / n);
}

/**
 * Pretty print stats
*/
void print_stats(Stats stats, int n, int iters, int substeps) {
    printf(
        "\nBenchmark results - %d steps, %d iters, %d substeps\n\n"
        "       μs       ms       s\n"
        "       -------- -------- --------\n"
        "min:   %-8.1f %-8.3f %-8f\n"
        "max:   %-8.1f %-8.3f %-8f\n"
        "avg:   %-8.1f %-8.3f %-8f\n"
        "stdev: %-8.1f %-8.3f %-8f\n",
        n, iters, substeps,
        stats.min * 1e6, stats.min * 1e3, stats.min,
        stats.max * 1e6, stats.max * 1e3, stats.max,
        stats.avg * 1e6, stats.avg * 1e3, stats.avg,
        stats.stdev * 1e6, stats.stdev * 1e3, stats.stdev
    );
}


/**
 * Expose PrecisionTimer API for Windows and Linux
*/

#if defined(_WIN32) || defined(__WIN32__) || defined(__WINDOWS__)

    #include <windows.h>

    typedef struct {
        LARGE_INTEGER _start;
        LARGE_INTEGER _end;
        LARGE_INTEGER _frequency;
        double elapsed;
    } PrecisionTimer;

    static inline void PrecisionTimer_start(PrecisionTimer *timer) {
        QueryPerformanceFrequency(&timer->_frequency);

        QueryPerformanceCounter(&timer->_start);
    }

    static inline void PrecisionTimer_stop(PrecisionTimer *timer ) {
        QueryPerformanceCounter(&timer->_end);

        timer->elapsed = (double)(timer->_end.QuadPart - timer->_start.QuadPart) /
                        (double)timer->_frequency.QuadPart;
    }

#else

    #include <time.h>
    #include <unistd.h>

    #define NS_PER_SECOND 1000000000

    typedef struct {
        struct timespec _start;
        struct timespec _end;
        struct timespec _delta;
        double elapsed;
    } PrecisionTimer;

    PrecisionTimer_start(PrecisionTimer *timer) {
        clock_gettime(CLOCK_REALTIME, &timer->start);
    }

    PrecisionTimer_stop(PrecisionTimer *timer) {
        clock_gettime(CLOCK_REALTIME, &timer->end);

        timer->delta.tv_nsec = timer->end.tv_nsec - timer->start.tv_nsec;
        timer->delta.tv_sec = timer->end.tv_sec - timer->start.tv_sec;

        if (timer->delta.tv_sec > 0 && timer->delta.tv_nsec < 0) {
            timmer->delta.tv_nsec += NS_PER_SECOND;
            timer->delta.tv_sec--;
        }
        else if (ttimer->delta.tv_sec < 0 && timer->delta.tv_nsec > 0) {
            timer->delta.tv_nsec -= NS_PER_SECOND;
            timer->delta.tv_sec++;
        }

        timer->elapsed = timer->delta.tv_nsec;
    }

#endif


#endif