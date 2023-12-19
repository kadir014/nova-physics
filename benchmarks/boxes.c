/*

  This file is a part of the Nova Physics Engine
  project and distributed under the MIT license.

  Copyright © Kadir Aksoy
  https://github.com/kadir014/nova-physics

*/

#include <stdio.h>
#include "benchmark_base.h"
#include "novaphysics/novaphysics.h"


/**
 * @file boxes.c
 * 
 * @brief Boxes benchmark. 3500 objects with minimal movement.
 */


enum {
    BENCHMARK_ITERS = 5000,
    BENCHMARK_HERTZ = 60,
    BENCHMARK_VELOCITY_ITERATIONS = 10,
    BENCHMARK_POSITION_ITERATIONS = 10,
    BENCHMARK_CONSTRAINT_ITERATIONS = 5
};


int main(int argc, char *argv[]) {
    // Setup benchmark

    nvSpace *space = nvSpace_new();

    Benchmark bench = Benchmark_new(BENCHMARK_ITERS, space);

    nvBody *ground = nvBody_new(
        nvBodyType_STATIC,
        nvRectShape_new(89.0, 5.0),
        NV_VEC2(64.0, 72),
        0.0,
        (nvMaterial){1.0, 0.1, 0.7}
    );

    nvSpace_add(space, ground);

    nvBody *wall_l = nvBody_new(
        nvBodyType_STATIC,
        nvRectShape_new(5.0, 80.0),
        NV_VEC2(22.0, 36.0),
        0.0,
        (nvMaterial){1.0, 0.1, 0.7}
    );

    nvSpace_add(space, wall_l);

    nvBody *wall_r = nvBody_new(
        nvBodyType_STATIC,
        nvRectShape_new(5.0, 80.0),
        NV_VEC2(128.0 - 22.0, 36.0),
        0.0,
        (nvMaterial){1.0, 0.1, 0.7}
    );

    nvSpace_add(space, wall_r);

    // Create stacked boxes

    int cols = 70; // Columns of the stack
    int rows = 50; // Rows of the stack
    nv_float size = 1.0; // Size of the boxes
    nv_float s2 = size / 2.0;
    nv_float ygap = 0.0;
    nv_float starty = 67.0;

    for (size_t y = 0; y < rows; y++) {
        for (size_t x = 0; x < cols; x++) {

            nv_float sizen = frand(3.75 / 10.0, 18.75 / 10.0);

            nvBody *box = nvBody_new(
                nvBodyType_DYNAMIC,
                nvRectShape_new(sizen, sizen),
                NV_VEC2(
                    1280.0 / 20.0 - (nv_float)cols * s2 + s2 + size * x,
                    starty - size - y * (size + ygap)
                ),
                0.0,
                (nvMaterial){1.0, 0.1, 0.2}
            );

            nvSpace_add(space, box);
        }
    }

    if (space->broadphase_algorithm == nvBroadPhaseAlg_SPATIAL_HASH_GRID)
        nvSpace_set_SHG(space, space->shg->bounds, 1.9, 1.9);

    // Run benchmark
    for (size_t i = 0; i < bench.iters; i++) {
        nv_float dt = 1.0 / (nv_float)BENCHMARK_HERTZ;

        Benchmark_start(&bench);

        nvSpace_step(
            space,
            dt,
            BENCHMARK_VELOCITY_ITERATIONS,
            BENCHMARK_POSITION_ITERATIONS,
            BENCHMARK_CONSTRAINT_ITERATIONS,
            1
        );

        Benchmark_stop(&bench);
    }
    
    Benchmark_results(&bench);


    nvSpace_free(space);
}