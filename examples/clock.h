/*

  This file is a part of the Nova Physics Engine
  project and distributed under the MIT license.

  Copyright © Kadir Aksoy
  https://github.com/kadir014/nova-physics

*/

#ifndef NOVAPHYSICS_EXAMPLE_CLOCK_H
#define NOVAPHYSICS_EXAMPLE_CLOCK_H


#include <stdlib.h>
#include <stdint.h>

#include "SDL.h"


typedef struct {
    double frequency;
    double accumulated_fps;
    double frame_time_full;
    double fps;
    double dt;
    double start;
    double time;
    uint64_t timer_start;
    uint64_t timer_end;
    uint64_t timer_full_end;
    uint32_t fps_counter;
} Clock;


Clock *Clock_new() {
    Clock *clock = NV_MALLOC(sizeof(Clock));

    clock->frequency = (double)SDL_GetPerformanceFrequency();
    clock->accumulated_fps = 0.0;
    clock->frame_time_full = 1.0;
    clock->fps = 0.0;
    clock->dt = 0.0;
    clock->start = (double)SDL_GetPerformanceCounter() / clock->frequency;
    clock->time = 0.0;
    clock->timer_start = 0;
    clock->timer_end = 0;
    clock->timer_full_end = 0;
    clock->fps_counter = 0;

    return clock;
}

void Clock_free(Clock *clock) {
    if (!clock) return;

    NV_FREE(clock);
}

void Clock_tick(Clock *clock, double target_fps) {
    double start = (double)clock->timer_start / clock->frequency;

    clock->timer_end = SDL_GetPerformanceCounter();

    double frame_time = ((double)clock->timer_end / clock->frequency - start) * 1000.0;

    clock->fps_counter++;
    clock->accumulated_fps += 1000.0 / clock->frame_time_full;
    if (clock->fps_counter >= 10) {
        clock->fps = clock->accumulated_fps / (double)10;

        clock->fps_counter = 0;
        clock->accumulated_fps = 0.0;
    }

    double target_wait_time = 1000.0 / target_fps;
    if (frame_time < target_wait_time) {
        SDL_Delay((uint32_t)(target_wait_time - frame_time));
    }

    clock->timer_full_end = SDL_GetPerformanceCounter();
    clock->frame_time_full = ((double)clock->timer_full_end / clock->frequency - start) * 1000.0;
    clock->dt = clock->frame_time_full / 1000.0;

    clock->timer_start = SDL_GetPerformanceCounter();

    clock->time = (double)SDL_GetPerformanceCounter() / clock->frequency - clock->start;
}


#endif