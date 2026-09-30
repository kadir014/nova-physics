/*

  This file is a part of the Nova Physics Engine
  project and distributed under the MIT license.

  Copyright © Kadir Aksoy
  https://github.com/kadir014/nova-physics

*/

#include <SDL.h>
#include "clock.h"


/*
    How often (in seconds) the value returned by nvClock_get_fps() refreshes.
    Averaging over a short window instead of reporting 1/dt every
    single frame gives a much more readable, less jittery number.
*/
#define NV_CLOCK_DEFAULT_FPS_UPDATE_INTERVAL 0.5

 
nvClock nvClock_new() {
    return (nvClock){
        .deltatime = 0.0,
        .target_fps = 0,
        .target_frametime = 0.0,
        .fps = 0.0,
        .fps_accum_time = 0.0,
        .fps_accum_frames = 0,
        .fps_update_interval = NV_CLOCK_DEFAULT_FPS_UPDATE_INTERVAL,
        .initialized = 0
    };
}
 
void nvClock_tick(nvClock *clock, int target_fps) {
    clock->target_fps = target_fps;
    clock->target_frametime = (target_fps > 0) ? (1.0 / (double)target_fps) : 0.0;
 
    // First call ever, just initialize the timer
    if (!clock->initialized) {
        nvPrecisionTimer_start(&clock->timer);
        clock->initialized = 1;
        clock->deltatime = (clock->target_frametime > 0.0) ? clock->target_frametime : 0.0;
        return;
    }
 
    double elapsed = nvPrecisionTimer_stop(&clock->timer);
 
    if (clock->target_frametime > 0.0 && elapsed < clock->target_frametime) {
        double remaining = clock->target_frametime - elapsed;
        
        /*
            The wait consists of two phases:

            1. Coarse & cheap wait
                Since SDL_Delay only quarantees ~1ms granularity and can oversleep
                (https://wiki.libsdl.org/SDL2/SDL_Delay), we sleep most of the
                remaining time with SDL_Delay and leave a very small safety margin
                for the second phase.

            2. Precise & expensive wait
                Burn the last sub-millisecond of the wait by busy-looping with
                the high precision timer.
                TODO: provide a parameter for the second phase.
        */

        double sleep_ms = (remaining * 1000.0) - 1.0;
        if (sleep_ms > 0.0) {
            SDL_Delay((Uint32)sleep_ms);
        }
 
        do {
            elapsed = nvPrecisionTimer_stop(&clock->timer);
        } while (elapsed < clock->target_frametime);
    }
 
    clock->deltatime = elapsed;
 
    // Rolling fps counter
    clock->fps_accum_frames++;
    clock->fps_accum_time += elapsed;
    if (clock->fps_accum_time >= clock->fps_update_interval) {
        clock->fps = (double)clock->fps_accum_frames / clock->fps_accum_time;
        clock->fps_accum_frames = 0;
        clock->fps_accum_time = 0.0;
    }
 
    nvPrecisionTimer_start(&clock->timer);
}
 
double nvClock_get_delta_time(const nvClock *clock) {
    return clock->deltatime;
}
 
double nvClock_get_fps(const nvClock *clock) {
    return clock->fps;
}

void nvClock_set_fps_interval(nvClock *clock, double interval) {
    clock->fps_update_interval = interval;
}