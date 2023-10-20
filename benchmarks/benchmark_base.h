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


#if defined(_WIN32) || defined(__WIN32__) || defined(__WINDOWS__)

    #define WINDOWS

#endif


/**
 * Expose PrecisionTimer API for Windows and Linux
*/

#ifdef WINDOWS

    #include <windows.h>

    typedef struct {
        double elapsed;
        LARGE_INTEGER _start;
        LARGE_INTEGER _end;
    } PrecisionTimer;

    static inline void PrecisionTimer_start(PrecisionTimer *timer) {
        QueryPerformanceCounter(&timer->_start);
    }

    static inline void PrecisionTimer_stop(PrecisionTimer *timer ) {
        QueryPerformanceCounter(&timer->_end);

        LARGE_INTEGER frequency;
        QueryPerformanceFrequency(&frequency);

        timer->elapsed = (double)(timer->_end.QuadPart - timer->_start.QuadPart) / (double)frequency.QuadPart;
    }

    /**
     * Set timer resolution to minimum for higher precision
    */
    void set_windows_timer_resolution() {
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
        struct timespec _start;
        struct timespec _end;
        struct timespec _delta;
        double elapsed;
    } PrecisionTimer;

    static inline void PrecisionTimer_start(PrecisionTimer *timer) {
        clock_gettime(CLOCK_REALTIME, &timer->_start);
    }

    static inline void PrecisionTimer_stop(PrecisionTimer *timer) {
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
    }

#endif


/**
 * Base benchmark struct
*/
typedef struct {
    PrecisionTimer *timer;
    size_t iters;
    double *times;
    size_t _index;
} Benchmark;

/**
 * Create new benchmark test
*/
Benchmark Benchmark_new(size_t iters) {
    Benchmark bench;

    bench.timer = (PrecisionTimer *)malloc(sizeof(PrecisionTimer));
    bench.iters = iters;
    bench.times = (double *)malloc(sizeof(double) * bench.iters);
    bench._index = 0;

    srand(time(NULL));

    #ifdef WINDOWS
    set_windows_timer_resolution();
    #endif

    return bench;
}

static inline void Benchmark_start(Benchmark *bench) {
    PrecisionTimer_start(bench->timer);
}

static inline void Benchmark_stop(Benchmark *bench) {
    PrecisionTimer_stop(bench->timer);

    bench->times[bench->_index] = bench->timer->elapsed;

    if (bench->_index % (bench->iters / 10) == 0) {
        printf("%ld%% ", (long)((double)bench->_index / (double)bench->iters * 100.0));
    }

    bench->_index++;
}

void Benchmark_results(Benchmark *bench) {
    printf("100%%\n");

    Stats stats1;
    calculate_stats(&stats1, bench->times, bench->iters);
    print_stats(stats1);

    free(bench->timer);
    free(bench->times);
}


#endif