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
 * @brief Return random integer in given range
 * 
 * @param lower Min range
 * @param higher Max range
 * @return int 
 */
int irand(int lower, int higher) {
    return (rand() % (higher - lower + 1)) + lower;
}

/**
 * @brief Return random double in given range
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
void print_stats(Stats stats) {
    printf(
        "       μs       ms       s\n"
        "       -------- -------- --------\n"
        "min:   %-8.1f %-8.3f %-8f\n"
        "max:   %-8.1f %-8.3f %-8f\n"
        "avg:   %-8.1f %-8.3f %-8f\n"
        "stdev: %-8.1f %-8.3f %-8f\n\n",
        stats.min * 1e6, stats.min * 1e3, stats.min,
        stats.max * 1e6, stats.max * 1e3, stats.max,
        stats.avg * 1e6, stats.avg * 1e3, stats.avg,
        stats.stdev * 1e6, stats.stdev * 1e3, stats.stdev
    );
}


/**
 * Base benchmark struct
*/
typedef struct {
    nv_PrecisionTimer *timer;
    nv_PrecisionTimer *global_timer;
    size_t iters;
    double *times;
    double *integrate_accelerations;
    double *broadphase;
    double *presolve_collisions;
    double *solve_positions;
    double *solve_velocities;
    double *integrate_velocities;
    size_t _index;
} Benchmark;

/**
 * Create new benchmark test
*/
Benchmark Benchmark_new(size_t iters) {
    Benchmark bench;

    bench.timer = (nv_PrecisionTimer *)malloc(sizeof(nv_PrecisionTimer));
    bench.global_timer = (nv_PrecisionTimer *)malloc(sizeof(nv_PrecisionTimer));
    nv_PrecisionTimer_start(bench.global_timer);
    bench.iters = iters;
    bench.times = (double *)malloc(sizeof(double) * bench.iters);
    bench.integrate_accelerations = (double *)malloc(sizeof(double) * bench.iters);
    bench.broadphase = (double *)malloc(sizeof(double) * bench.iters);
    bench.presolve_collisions = (double *)malloc(sizeof(double) * bench.iters);
    bench.solve_positions = (double *)malloc(sizeof(double) * bench.iters);
    bench.solve_velocities = (double *)malloc(sizeof(double) * bench.iters);
    bench.integrate_velocities = (double *)malloc(sizeof(double) * bench.iters);
    bench._index = 0;

    srand(time(NULL));

    #ifdef NV_WINDOWS
    nv_set_windows_timer_resolution();
    #endif

    return bench;
}

static inline void Benchmark_start(Benchmark *bench) {
    nv_PrecisionTimer_start(bench->timer);
}

static inline void Benchmark_stop(Benchmark *bench, nv_Space *space) {
    nv_PrecisionTimer_stop(bench->timer);
    nv_PrecisionTimer_stop(bench->global_timer);

    bench->times[bench->_index] = bench->timer->elapsed;

    if (space) {
        bench->integrate_accelerations[bench->_index] = space->profiler.integrate_accelerations;
        bench->broadphase[bench->_index] = space->profiler.broadphase;
        bench->presolve_collisions[bench->_index] = space->profiler.presolve_collisions;
        bench->solve_positions[bench->_index] = space->profiler.solve_positions;
        bench->solve_velocities[bench->_index] = space->profiler.solve_velocities;
        bench->integrate_velocities[bench->_index] = space->profiler.integrate_velocities;
    }

    double elapsed = bench->global_timer->elapsed;

    double remaining = (elapsed / (double)bench->_index) * ((double)bench->iters - (double)bench->_index);

    int rem_secs = (int)round(remaining);

    int rem_mins = rem_secs / 60;
    rem_secs %= 60;

    int rem_hours = rem_mins / 60;
    rem_mins %= 60;

    if (bench->_index % 10 == 0) {
        printf(
            "Frame %ld/%ld (%.2f%%) eta %02d:%02d:%02d                 \n\033[1G\033[1A",
            bench->_index,
            bench->iters,
            (double)bench->_index / (double)bench->iters * 100.0,
            rem_hours,
            rem_mins,
            rem_secs
        );
    }

    bench->_index++;
}

void Benchmark_results(Benchmark *bench, bool print_profiler) {
    nv_PrecisionTimer_stop(bench->global_timer);

    int rem_secs = (int)round(bench->global_timer->elapsed);

    int rem_mins = rem_secs / 60;
    rem_secs %= 60;

    int rem_hours = rem_mins / 60;
    rem_mins %= 60;

    printf(
        "Benchmark completed in %02d:%02d:%02d                      \n\033[1G\033[1A\n\n",
        rem_hours,
        rem_mins,
        rem_secs    
    );

    Stats stats0;
    calculate_stats(&stats0, bench->times, bench->iters);
    print_stats(stats0);

    if (print_profiler) {
        Stats stats1;
        calculate_stats(&stats1, bench->integrate_accelerations, bench->iters);
        printf("INTEGRATE ACCELERATIONS\n");
        print_stats(stats1);

        Stats stats2;
        calculate_stats(&stats2, bench->broadphase, bench->iters);
        printf("BROADPHASE\n");
        print_stats(stats2);

        Stats stats3;
        calculate_stats(&stats3, bench->presolve_collisions, bench->iters);
        printf("PRESOLVE\n");
        print_stats(stats3);

        Stats stats4;
        calculate_stats(&stats4, bench->solve_positions, bench->iters);
        printf("SOLVE POSITIONS\n");
        print_stats(stats4);

        Stats stats5;
        calculate_stats(&stats5, bench->solve_velocities, bench->iters);
        printf("SOLVE VELOCITIES\n");
        print_stats(stats5);

        Stats stats6;
        calculate_stats(&stats6, bench->integrate_velocities, bench->iters);
        printf("INTEGRATE VELOCITIES\n");
        print_stats(stats6);
    }

    free(bench->timer);
    free(bench->times);
    free(bench->integrate_accelerations);
    free(bench->broadphase);
    free(bench->presolve_collisions);
    free(bench->solve_positions);
    free(bench->solve_velocities);
    free(bench->integrate_velocities);
}


#endif