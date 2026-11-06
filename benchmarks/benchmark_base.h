/*

  This file is a part of the Nova Physics Engine
  project and distributed under the MIT license.

  Copyright © Kadir Aksoy
  https://github.com/kadir014/nova-physics

*/

#ifndef NOVAPHYSICS_BENCHMARK_BASE_H
#define NOVAPHYSICS_BENCHMARK_BASE_H

#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <string.h>
#include <stdbool.h>


/**
 * @file benchmark_base.h
 * 
 * @brief This header defines utilities to run and measure benchmarks.
 * 
 * It does not depend on novaphysics headers, you can copy this header for your
 * own benchmarks.
 */


#if defined(_WIN32) || defined(__WIN32__) || defined(__WINDOWS__)

    #define NVB_PLATFORM_WINDOWS
    #define NVB_PLATFORM_STR "Windows"

    #include <windows.h>

#elif defined(__linux__)

    #define NVB_PLATFORM_LINUX
    #define NVB_PLATFORM_STR "Linux"

#elif defined(__APPLE__) || defined(__MACH__)

    #define NVB_PLATFORM_MACOS
    #define NVB_PLATFORM_STR "MacOS"

#else

    #define NVB_PLATFORM_STR "Unknown"

#endif


#if defined(__GNUC__) || defined(__MINGW32__) || defined(__MINGW64__)

    #define NVB_COMPILER_GCC
    #define NVB_COMPILER_STR "GCC"

#elif defined(_MSC_VER) || defined(_MSC_FULL_VER) || defined(_MSVC_LANG)

    #define NVB_COMPILER_MSVC
    #define NVB_COMPILER_STR "MSVC"

#else

    #define NVB_COMPILER_STR "Unknown"

#endif


/*
    Direct copy of novaphysics/profiler.h in order to keep this header
    self-sufficient and dependency free.
*/
#if defined(NVB_PLATFORM_WINDOWS)

    #include <windows.h>

    typedef struct _nvbPrecisionTimer {
        double elapsed;
        LARGE_INTEGER _start;
        LARGE_INTEGER _end;
    } nvbPrecisionTimer;

    static inline void nvbPrecisionTimer_start(nvbPrecisionTimer *timer) {
        QueryPerformanceCounter(&timer->_start);
    }

    static inline double nvbPrecisionTimer_stop(nvbPrecisionTimer *timer ) {
        QueryPerformanceCounter(&timer->_end);

        LARGE_INTEGER frequency;
        QueryPerformanceFrequency(&frequency);

        timer->elapsed = (double)(timer->_end.QuadPart - timer->_start.QuadPart) / (double)frequency.QuadPart;
        return timer->elapsed;
    }

#else

    #include <time.h>
    #include <unistd.h>

    // TODO: On OSX, frequency can be milliseconds instead of nanoseconds? Needs researching
    #define NVB_PRECISION_TIMER_NS_PER_SECOND 1e9

    typedef struct _nvbPrecisionTimer {
        double elapsed;
        struct timespec _start;
        struct timespec _end;
        struct timespec _delta;
    } nvbPrecisionTimer;

    static inline void nvbPrecisionTimer_start(nvbPrecisionTimer *timer) {
        clock_gettime(CLOCK_REALTIME, &timer->_start);
    }

    static inline double nvbPrecisionTimer_stop(nvbPrecisionTimer *timer) {
        clock_gettime(CLOCK_REALTIME, &timer->_end);

        timer->_delta.tv_nsec = timer->_end.tv_nsec - timer->_start.tv_nsec;
        timer->_delta.tv_sec = timer->_end.tv_sec - timer->_start.tv_sec;

        if (timer->_delta.tv_sec > 0 && timer->_delta.tv_nsec < 0) {
            timer->_delta.tv_nsec += NVB_PRECISION_TIMER_NS_PER_SECOND;
            timer->_delta.tv_sec--;
        }
        else if (timer->_delta.tv_sec < 0 && timer->_delta.tv_nsec > 0) {
            timer->_delta.tv_nsec -= NVB_PRECISION_TIMER_NS_PER_SECOND;
            timer->_delta.tv_sec++;
        }

        timer->elapsed = (double)timer->_delta.tv_sec + (double)timer->_delta.tv_nsec / NVB_PRECISION_TIMER_NS_PER_SECOND;
        return timer->elapsed;
    }

#endif


/**
 * @brief Return random integer in given range.
 * 
 * @param lower Min range
 * @param higher Max range
 * @return int 
 */
static inline int nvb_irand(int lower, int higher) {
    return (rand() % (higher - lower + 1)) + lower;
}

/**
 * @brief Return random double in given range.
 * 
 * @param lower Min range
 * @param higher Max range
 * @return double 
 */
static inline double nvb_frand(double lower, double higher) {
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
} nvbStats;

/**
 * @brief Base benchmark struct.
 */
typedef struct {
    void *user_data;

    nvbPrecisionTimer *timer;
    nvbPrecisionTimer *global_timer;

    size_t iters;

    double *times;

    size_t frame;
} nvbBenchmark;

/**
 * @brief Create new benchmark test.
 */
nvbBenchmark nvbBenchmark_new(size_t iters, void *user_data) {
    nvbBenchmark bench;

    bench.user_data = user_data;

    bench.timer = malloc(sizeof(nvbPrecisionTimer));
    bench.global_timer = malloc(sizeof(nvbPrecisionTimer));

    bench.iters = iters;
    bench.times = malloc(sizeof(double) * bench.iters);

    bench.frame = 0;
    nvbPrecisionTimer_start(bench.global_timer);

    srand(time(NULL));

    return bench;
}

void nvbBenchmark_free(nvbBenchmark *bench) {
    if (!bench) return;

    free(bench->global_timer);
    free(bench->timer);
    free(bench->times);
}

static inline void nvbBenchmark_start(nvbBenchmark *bench) {
    nvbPrecisionTimer_start(bench->timer);
}

static inline void nvbBenchmark_update(nvbBenchmark *bench) {
    nvbPrecisionTimer_stop(bench->timer);
    nvbPrecisionTimer_stop(bench->global_timer);

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

static inline int _nvb_compare_doubles(const void *a, const void *b) {
    double x = *(const double *)a;
    double y = *(const double *)b;

    return (x > y) - (x < y);
}

/**
 * @brief Calculate benchmark statistics.
 */
nvbStats nvbBenchmark_calculate_stats(nvbBenchmark *bench) {
    nvbStats stats = {0};

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

    qsort(sorted, n, sizeof(double), _nvb_compare_doubles);

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
void nvbStats_pretty_print(nvbStats *stats) {
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

void nvbBenchmark_results(nvbBenchmark *bench, const char *scene, bool verbose) {
    nvbPrecisionTimer_stop(bench->global_timer);

    #ifdef NVB_PLATFORM_WINDOWS
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
        "nvbBenchmark took %02d:%02d:%02d\n"
        "Compiled with %s\n"
        "Platform: %s\n",
        scene,
        ela_hours, ela_mins, ela_secs,
        NVB_COMPILER_STR,
        NVB_PLATFORM_STR
    );

    nvbStats stats = nvbBenchmark_calculate_stats(bench);
    printf("\nPhysics frame:\n--------------------------------------\n");
    nvbStats_pretty_print(&stats);
}


#endif // NOVAPHYSICS_BENCHMARK_BASE_H