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
#include <time.h>
#include <string.h>
#include "novaphysics/novaphysics.h"


/**
 * @file benchmark_base.h
 * 
 * @brief This header defines utilities to run and measure benchmarks.
 */


#if defined(_WIN32) || defined(__WIN32__) || defined(__WINDOWS__)

    #define BENCHMARK_PLATFORM_WINDOWS
    #define BENCHMARK_PLATFORM_STR "Windows"

    #include <windows.h>

#elif defined(__linux__)

    #define BENCHMARK_PLATFORM_LINUX
    #define BENCHMARK_PLATFORM_STR "Linux"

#elif defined(__APPLE__) || defined(__MACH__)

    #define BENCHMARK_PLATFORM_MACOS
    #define BENCHMARK_PLATFORM_STR "MacOS"

#else

    #define BENCHMARK_PLATFORM_STR "Unknown"

#endif


#if defined(__GNUC__) || defined(__MINGW32__) || defined(__MINGW64__)

    #define BENCHMARK_COMPILER_GCC
    #define BENCHMARK_COMPILER_STR "GCC"

#elif defined(_MSC_VER) || defined(_MSC_FULL_VER) || defined(_MSVC_LANG)

    #define BENCHMARK_COMPILER_MSVC
    #define BENCHMARK_COMPILER_STR "MSVC"

#else

    #define BENCHMARK_COMPILER_STR "Unknown"

#endif


/**
 * @brief Return random integer in given range.
 * 
 * @param lower Min range
 * @param higher Max range
 * @return int 
 */
int irand(int lower, int higher) {
    return (rand() % (higher - lower + 1)) + lower;
}

/**
 * @brief Return random double in given range.
 * 
 * @param lower Min range
 * @param higher Max range
 * @return double 
 */
double frand(double lower, double higher) {
    double normal = (double)rand() / (double)RAND_MAX;
    return lower + normal * (higher - lower);
}


/**
 * @brief Accumulated statistics for a benchmark run.
 */
typedef struct {
    double min; /**< Absolute minimum of all timings. */
    double max; /**< Absolute maximum of all timings. */
    double p5; /**< 5% lowest of all timings. */
    double p95; /**< 95% highest of all timings. */
    double avg; /**< Arithmetic mean of all timings. */
    double tm; /**< Mean of 5% trimmed of each end (lowest and highest). */
    double stdev; /**< Standard deviation. */
} Stats;

/**
 * @brief Base benchmark struct.
 */
typedef struct {
    void *user_data;

    nvPrecisionTimer *timer;
    nvPrecisionTimer *global_timer;

    size_t iters;

    double *times;

    size_t frame;
} Benchmark;

/**
 * @brief Create new benchmark test.
 */
Benchmark Benchmark_new(size_t iters, void *user_data) {
    Benchmark bench;

    bench.user_data = user_data;

    bench.timer = NV_NEW(nvPrecisionTimer);
    bench.global_timer = NV_NEW(nvPrecisionTimer);

    bench.iters = iters;
    bench.times = NV_MALLOC(sizeof(double) * bench.iters);

    bench.frame = 0;
    nvPrecisionTimer_start(bench.global_timer);

    srand(time(NULL));

    return bench;
}

void Benchmark_free(Benchmark *bench) {
    if (!bench) return;

    NV_FREE(bench->global_timer);
    NV_FREE(bench->timer);
    NV_FREE(bench->times);
}

static inline void Benchmark_start(Benchmark *bench) {
    nvPrecisionTimer_start(bench->timer);
}

static inline void Benchmark_update(Benchmark *bench) {
    nvPrecisionTimer_stop(bench->timer);
    nvPrecisionTimer_stop(bench->global_timer);

    bench->times[bench->frame] = bench->timer->elapsed;

    // Guess remaining time from global elapsed time

    double elapsed = bench->global_timer->elapsed;
    double remaining = (elapsed / (double)bench->frame) * ((double)bench->iters - (double)bench->frame);

    int rem_secs = (int)round(remaining);

    int rem_mins = rem_secs / 60;
    rem_secs %= 60;

    int rem_hours = rem_mins / 60;
    rem_mins %= 60;

    // Update console output every 10th frame
    if (bench->frame % 10 == 0) {
        printf(
            "Frame %llu/%llu (%.2f%%) eta %02d:%02d:%02d                 \n\033[1G\033[1A",
            (unsigned long long)bench->frame,
            (unsigned long long)bench->iters,
            (double)bench->frame / (double)bench->iters * 100.0,
            rem_hours,
            rem_mins,
            rem_secs
        );
    }

    bench->frame++;
}

static inline int _compare_doubles(const void *a, const void *b) {
    double x = *(const double *)a;
    double y = *(const double *)b;

    return (x > y) - (x < y);
}

/**
 * @brief Calculate benchmark statistics.
 */
Stats Benchmark_calculate_stats(Benchmark *bench) {
    Stats stats = {0};

    size_t n = bench->iters;
    double *times = bench->times;

    if (n == 0) {
        return stats;
    }

    // Calculate minimum, maximum and arithmetic mean.
    double sum = 0.0;
    stats.min = INFINITY;
    stats.max = -INFINITY;

    for (size_t i = 0; i < n; i++) {
        double t = times[i];

        sum += t;

        if (t < stats.min) stats.min = t;
        if (t > stats.max) stats.max = t;
    }

    stats.avg = sum / (double)n;

    // Calculate population standard deviation.
    double variance = 0.0;

    for (size_t i = 0; i < n; i++) {
        double diff = times[i] - stats.avg;
        variance += diff * diff;
    }

    stats.stdev = sqrt(variance / (double)n);

    // Create a sorted copy for percentile and trimmed mean calculations.
    double *sorted = malloc(n * sizeof(double));
    if (!sorted) {
        return stats;
    }

    memcpy(sorted, times, n * sizeof(double));

    qsort(sorted, n, sizeof(double), _compare_doubles);

    // Calculate percentiles (nearest-rank method).
    size_t p5frame = (size_t)ceil(0.05 * n);
    size_t p95frame = (size_t)ceil(0.95 * n);

    if (p5frame > 0) p5frame--;
    if (p95frame > 0) p95frame--;

    stats.p5 = sorted[p5frame];
    stats.p95 = sorted[p95frame];

    // Calculate 5% trimmed mean.
    const double trim_perc = 0.05;
    size_t trim = (size_t)(n * trim_perc);
    size_t start = trim;
    size_t end = n - trim;

    double trimmed_sum = 0.0;

    for (size_t i = start; i < end; i++) {
        trimmed_sum += sorted[i];
    }

    stats.tm = trimmed_sum / (double)(end - start);

    free(sorted);

    return stats;
}

/**
 * @brief Pretty print stats.
 */
void Stats_pretty_print(Stats *stats) {
    printf(
        "                   ms        μs\n"
        "Minimum:           %-8.3f  %-8.3f\n"
        "Maximum:           %-8.3f  %-8.3f\n"
        "5th Percentile:    %-8.3f  %-8.3f\n"
        "95th Percentile:   %-8.3f  %-8.3f\n"
        "Arithmetic Mean:   %-8.3f  %-8.3f\n"
        "Trimmed Mean (5%%): %-8.3f  %-8.3f\n"
        "Std. Dev.:         %-8.3f  %-8.3f\n",
        stats->min * 1e3, stats->min * 1e6,
        stats->max * 1e3, stats->max * 1e6,
        stats->p5 * 1e3, stats->p5 * 1e6,
        stats->p95 * 1e3, stats->p95 * 1e6,
        stats->avg * 1e3, stats->avg * 1e6,
        stats->tm * 1e3, stats->tm * 1e6,
        stats->stdev * 1e3, stats->stdev * 1e6
    );
}

void Benchmark_results(Benchmark *bench, const char *scene, nv_bool verbose) {
    nvPrecisionTimer_stop(bench->global_timer);

    #ifdef BENCHMARK_PLATFORM_WINDOWS
        SetConsoleOutputCP(CP_UTF8);
    #endif

    int ela_secs = (int)round(bench->global_timer->elapsed);

    int ela_mins = ela_secs / 60;
    ela_secs %= 60;

    int ela_hours = ela_mins / 60;
    ela_mins %= 60;

    // Overwrite progress bbar
    printf("                                            \n\033[1G\033[1A\n");

    printf(
        "Benchmark finished successfully.\n"
        "Scene: %s\n"
        "=============================================\n"
        "Benchmark took %02d:%02d:%02d\n"
        "Compiled with %s\n"
        "Platform: %s\n",
        scene,
        ela_hours, ela_mins, ela_secs,
        BENCHMARK_COMPILER_STR,
        BENCHMARK_PLATFORM_STR
    );

    Stats stats = Benchmark_calculate_stats(bench);
    printf("\nPhysics frame:\n--------------------------------------\n");
    Stats_pretty_print(&stats);
}


#endif