/*

  This file is a part of the Nova Physics Engine
  project and distributed under the MIT license.

  Copyright © Kadir Aksoy
  https://github.com/kadir014/nova-physics

*/

#include <stdio.h>
#include <string.h>
#include "benchmark_base.h"
#include "novaphysics/novaphysics.h"

#include "scenes/scene_pyramid.h"


/**
 * @file benchmarks/main.c
 * 
 * @brief Nova benchmarks entry point.
 */


#define BENCHMARK_ITERS 1000
#define BENCHMARK_HERTZ 60.0
#define BENCHMARK_VELOCITY_ITERATIONS 8


int main(int argc, char *argv[]) {
    const nv_float dt = 1.0f / (nv_float)BENCHMARK_HERTZ;

    nvSpace *space = nvSpace_new();
    space->settings.velocity_iterations = BENCHMARK_VELOCITY_ITERATIONS;
    space->settings.warmstarting = true;

    nv_bool verbose = false;
    for (int i = 0; i < argc; i++) {
        if (strcmp(argv[i], "-v") == 0) {
            verbose = true;
            break;
        }
    }
    printf("Verbose output: %s (use -v to enable)\n\n", verbose ? "on" : "off");

    // Pyramid scene
    {
        nvbBenchmark bench = nvbBenchmark_new(BENCHMARK_ITERS, space);

        Pyramid_setup(space);

        // Warmup to remove outliers caused by OS spikes and such...
        for (size_t i = 0; i < 30; i++) {
            nvSpace_step(space, dt);
        }

        for (size_t i = 0; i < bench.iters; i++) {
            nvbBenchmark_start(&bench);
            nvSpace_step(space, dt);
            nvbBenchmark_update(&bench);
        }
        
        nvbBenchmark_results(&bench, "Pyramid", verbose);

        nvSpace_clear(space, true, true);
    }

    nvSpace_free(space);
}