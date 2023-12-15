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
#include "novaphysics/novaphysics.h"


/**
 * @file benchmark_base.h
 * 
 * @brief This header defines utilities to run and measure benchmarks.
 */


#ifdef NV_WINDOWS

    #define BENCHMARK_PLATFORM_STR "Windows"

#elif defined(__linux__)

    #define BENCHMARK_PLATFORM_STR "Linux"

#elif defined(__APPLE__) || defined(__MACH__)

    #define BENCHMARK_PLATFORM_STR "OSX"

#else

    #define BENCHMARK_PLATFORM_STR "Unknown"

#endif


#ifdef NV_COMPILER_GCC

    #define BENCHMARK_COMPILER_STR "GCC"

#elif defined(NV_COMPILER_MSVC)

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
    double normal = rand() / (double)RAND_MAX;
    return lower + normal * (higher - lower);
}


typedef struct {
    double min;
    double max;
    double avg;
    double stdev;
} Stats;

/**
 * @brief Calculate minimum, maximum, mean and standard deviation values.
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
 * @brief Pretty print stats.
 */
void print_stats(Stats stats) {
    printf(
        "       μs       ms\n"
        "min:   %-8.1f %-8.3f\n"
        "max:   %-8.1f %-8.3f\n"
        "avg:   %-8.1f %-8.3f\n",
        //"stdev: %-8.1f %-8.3f %-8f\n\n",
        stats.min * 1e6, stats.min * 1e3,
        stats.max * 1e6, stats.max * 1e3,
        stats.avg * 1e6, stats.avg * 1e3
        //stats.stdev * 1e6, stats.stdev * 1e3, stats.stdev
    );
}


/**
 * @brief Base benchmark struct.
 */
typedef struct {
    nvPrecisionTimer *timer;
    nvPrecisionTimer *global_timer;
    size_t iters;
    double *times;
    double *integrate_accelerations;
    double *broadphase;
    double *narrowphase;
    double *presolve_collisions;
    double *solve_positions;
    double *solve_velocities;
    double *integrate_velocities;
    size_t _index;
    FILE *output;
} Benchmark;

/**
 * @brief Create new benchmark test.
 */
Benchmark Benchmark_new(size_t iters) {
    Benchmark bench;

    bench.timer = (nvPrecisionTimer *)malloc(sizeof(nvPrecisionTimer));
    bench.global_timer = (nvPrecisionTimer *)malloc(sizeof(nvPrecisionTimer));
    nvPrecisionTimer_start(bench.global_timer);
    bench.iters = iters;
    bench.times = (double *)malloc(sizeof(double) * bench.iters);
    bench.integrate_accelerations = (double *)malloc(sizeof(double) * bench.iters);
    bench.broadphase = (double *)malloc(sizeof(double) * bench.iters);
    bench.narrowphase = (double *)malloc(sizeof(double) * bench.iters);
    bench.presolve_collisions = (double *)malloc(sizeof(double) * bench.iters);
    bench.solve_positions = (double *)malloc(sizeof(double) * bench.iters);
    bench.solve_velocities = (double *)malloc(sizeof(double) * bench.iters);
    bench.integrate_velocities = (double *)malloc(sizeof(double) * bench.iters);
    bench._index = 0;

    srand(time(NULL));

    #ifdef NV_WINDOWS
    nv_set_windows_timer_resolution();
    #endif

    bench.output = fopen("bench_out.txt", "a");

    return bench;
}

static inline void Benchmark_start(Benchmark *bench) {
    nvPrecisionTimer_start(bench->timer);
}

static inline void Benchmark_stop(Benchmark *bench, nvSpace *space) {
    nvPrecisionTimer_stop(bench->timer);
    nvPrecisionTimer_stop(bench->global_timer);

    bench->times[bench->_index] = bench->timer->elapsed;

    if (space) {
        bench->integrate_accelerations[bench->_index] = space->profiler.integrate_accelerations;
        bench->broadphase[bench->_index] = space->profiler.broadphase;
        bench->narrowphase[bench->_index] = space->profiler.narrowphase;
        bench->presolve_collisions[bench->_index] = space->profiler.presolve_collisions;
        bench->solve_positions[bench->_index] = space->profiler.solve_positions;
        bench->solve_velocities[bench->_index] = space->profiler.solve_velocities;
        bench->integrate_velocities[bench->_index] = space->profiler.integrate_velocities;
    }

    // Append frame stats to output file
    fprintf(
        bench->output,
        "%f:%f:%f:%f:%f:%f:%f:%f\n",
        bench->timer->elapsed * 1000,
        space->profiler.integrate_accelerations * 1000,
        space->profiler.broadphase * 1000,
        space->profiler.narrowphase * 1000,
        space->profiler.presolve_collisions * 1000,
        space->profiler.solve_positions * 1000,
        space->profiler.solve_velocities * 1000,
        space->profiler.integrate_velocities * 1000
    );

    double elapsed = bench->global_timer->elapsed;

    double remaining = (elapsed / (double)bench->_index) * ((double)bench->iters - (double)bench->_index);

    int rem_secs = (int)round(remaining);

    int rem_mins = rem_secs / 60;
    rem_secs %= 60;

    int rem_hours = rem_mins / 60;
    rem_mins %= 60;

    if (bench->_index % 10 == 0) {
        printf(
            "Frame %llu/%llu (%.2f%%) eta %02d:%02d:%02d                 \n\033[1G\033[1A",
            (unsigned long long)bench->_index,
            (unsigned long long)bench->iters,
            (double)bench->_index / (double)bench->iters * 100.0,
            rem_hours,
            rem_mins,
            rem_secs
        );
    }

    bench->_index++;
}

void Benchmark_results(Benchmark *bench) {
    nvPrecisionTimer_stop(bench->global_timer);

    int ela_secs = (int)round(bench->global_timer->elapsed);

    int ela_mins = ela_secs / 60;
    ela_secs %= 60;

    int ela_hours = ela_mins / 60;
    ela_mins %= 60;

    // Overwrite progress bbar
    printf("                                            \n\033[1G\033[1A\n");

    printf(
        "Nova Physics benchmark finished successfully.\n"
        "=============================================\n"
        "Benchmark took %02d:%02d:%02d\n"
        "Nova version: %d.%d.%d\n"
        "Compiled with %s\n"
        "Platform: %s\n",
        ela_hours, ela_mins, ela_secs,
        NV_VERSION_MAJOR, NV_VERSION_MINOR, NV_VERSION_PATCH,
        BENCHMARK_COMPILER_STR,
        BENCHMARK_PLATFORM_STR
    );

    Stats stats0;
    calculate_stats(&stats0, bench->times, bench->iters);
    printf("\nPhysics time (one frame):\n---------------------\n");
    print_stats(stats0);

    Stats stats1;
    calculate_stats(&stats1, bench->integrate_accelerations, bench->iters);
    printf("\nIntegrate accelerations:\n---------------------\n");
    print_stats(stats1);

    Stats stats2;
    calculate_stats(&stats2, bench->broadphase, bench->iters);
    printf("\nBroad-phase:\n---------------------\n");
    print_stats(stats2);

    Stats stats7;
    calculate_stats(&stats7, bench->narrowphase, bench->iters);
    printf("\nNarrow-phase:\n---------------------\n");
    print_stats(stats7);

    Stats stats3;
    calculate_stats(&stats3, bench->presolve_collisions, bench->iters);
    printf("\nPresolve collisions:\n---------------------\n");
    print_stats(stats3);

    Stats stats4;
    calculate_stats(&stats4, bench->solve_positions, bench->iters);
    printf("\nSolve positions:\n---------------------\n");
    print_stats(stats4);

    Stats stats5;
    calculate_stats(&stats5, bench->solve_velocities, bench->iters);
    printf("\nSolve velocities:\n---------------------\n");
    print_stats(stats5);

    Stats stats6;
    calculate_stats(&stats6, bench->integrate_velocities, bench->iters);
    printf("\nIntegrate velocities:\n---------------------\n");
    print_stats(stats6);

    free(bench->timer);
    free(bench->times);
    free(bench->integrate_accelerations);
    free(bench->broadphase);
    free(bench->presolve_collisions);
    free(bench->solve_positions);
    free(bench->solve_velocities);
    free(bench->integrate_velocities);
    fclose(bench->output);
}


#endif