/*

  This file is a part of the Nova Physics Engine
  project and distributed under the MIT license.

  Copyright © Kadir Aksoy
  https://github.com/kadir014/nova-physics

*/

#include <stdio.h>
#include "benchmark_base.h"
#include "novaphysics/novaphysics.h"

#include "scenes/scene_pyramid.h"


/**
 * @file benchmarks/main.c
 * 
 * @brief Nova benchmarks entry point.
 */


enum {
    BENCHMARK_ITERS = 1000,
    BENCHMARK_HERTZ = 60,
    BENCHMARK_VELOCITY_ITERATIONS = 10
};


int main(int argc, char *argv[]) {
    nvSpace *space = nvSpace_new();
    space->settings.velocity_iterations = BENCHMARK_VELOCITY_ITERATIONS;

    // Pyramid scene
    {
        Benchmark bench = Benchmark_new(BENCHMARK_ITERS, space);

        Pyramid_setup(space);

        for (size_t i = 0; i < bench.iters; i++) {
            nv_float dt = 1.0 / (nv_float)BENCHMARK_HERTZ;

            Benchmark_start(&bench);
            nvSpace_step(space, dt);
            Benchmark_stop(&bench);
        }
        
        Benchmark_results(&bench);

        nvSpace_clear(space, true);
    }

    nvSpace_free(space);
}