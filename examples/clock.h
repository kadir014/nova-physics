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

#include "novaphysics/profiler.h"


typedef struct {
    nvPrecisionTimer timer;
 
    double deltatime; /**< Seconds elapsed during the last frame. */
    int target_fps; /**< Target FPS cap. */
    double target_frametime; /**< 1.0 / target_fps */
 
    double fps; /**< Most recently reported FPS value. */
    double fps_accum_time; /**< Seconds accumulated since FPS was last refreshed. */
    int fps_accum_frames; /**< Frames counted since FPS was last refreshed */
    double fps_update_interval; /**< How often the reported fps value refreshes. */
 
    int initialized; /**< Flag for setting if the timer been started yet. */
} nvClock;

/**
 * @brief Initialize a new clock.
 * 
 * @return nvClock 
 */
nvClock nvClock_new();

/**
 * @brief Update the clock.
 * 
 * This should be called once per frame, at the top (or bottom) of your main loop.
 * Blocks just long enough to keep the app at approximately the targeted FPS.
 * 
 * @param clock Clock.
 * @param target_fps FPS cap. 
 */
void nvClock_tick(nvClock *clock, int target_fps);

/**
 * @brief Seconds elapsed during the last frame (after framerate capping).
 * 
 * @param clock Clock.
 * @return Elapsed time in seconds.
 */
double nvClock_get_delta_time(const nvClock *clock);

/**
 * @brief Most recently measured frames-per-second.
 * 
 * @param clock Clock.
 * @return Most recent FPS value.
 */
double nvClock_get_fps(const nvClock *clock);

/**
 * @brief Set what frequency the FPS is updated in seconds. 
 * 
 * @param clock Clock.
 * @param interval FPS update interval in seconds.
 */
void nvClock_set_fps_interval(nvClock *clock, double interval);


#endif