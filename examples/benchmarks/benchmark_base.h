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

void print_stats(Stats stats, int n, int iters, int substeps) {
    printf(
        "Benchmark results - %d steps, %d iters, %d substeps\n\n"
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


#include <windows.h>


typedef struct {
    LARGE_INTEGER start;
    LARGE_INTEGER end;
    LARGE_INTEGER frequency;
    double elapsed;
} PrecisionTimer;

static inline void PrecisionTimer_start(PrecisionTimer *timer) {
    QueryPerformanceFrequency(&timer->frequency);

    QueryPerformanceCounter(&timer->start);
}

static inline void PrecisionTimer_stop(PrecisionTimer *timer ) {
    QueryPerformanceCounter(&timer->end);

    timer->elapsed = (double)(timer->end.QuadPart - timer->start.QuadPart) /
                     (double)timer->frequency.QuadPart;
}


#endif