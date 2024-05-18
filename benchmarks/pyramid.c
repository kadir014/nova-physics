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

    nvRigidBody *ground = nvRigidBody_new(
        nvRigidBodyType_STATIC,
        nvRectShape_new(200.0, 5.0),
        NV_VEC2(64.0, 62.5),
        0.0,
        (nvMaterial){1.0, 0.1, 0.6}
    );

    nvSpace_add(space, ground);

    nvRigidBody *wall_l = nvRigidBody_new(
        nvRigidBodyType_STATIC,
        nvRectShape_new(5.0, 80.0),
        NV_VEC2(22.0, 36.0),
        0.0,
        (nvMaterial){1.0, 0.1, 0.7}
    );

    nvSpace_add(space, wall_l);

    nvRigidBody *wall_r = nvRigidBody_new(
        nvRigidBodyType_STATIC,
        nvRectShape_new(5.0, 80.0),
        NV_VEC2(128.0 - 22.0, 36.0),
        0.0,
        (nvMaterial){1.0, 0.1, 0.7}
    );

    nvSpace_add(space, wall_r);

    // Create stacked boxes

    int base = 100;
    nv_float size = 0.5;
    nv_float s2 = size / 2.0;
    nv_float y_gap = 0.0;

    for (size_t y = 0; y < base; y++) {
        for (size_t x = 0; x < base - y; x++) {
            nvRigidBody *box = nvRigidBody_new(
                nvRigidBodyType_DYNAMIC,
                nvRectShape_new(size, size),
                NV_VEC2(
                    128.0 / 2.0 - (base * s2 - s2) + x * size + y * s2,
                    62.5 - 2.5 - s2 - y * (size + y_gap)
                ),
                0.0,
                (nvMaterial){1.0, 0.1, 0.5}
            );

            nvSpace_add(space, box);
        }
    }

    if (space->broadphase_algorithm == nvBroadPhaseAlg_SHG) {
        nvSpace_set_SHG(space, space->shg->bounds, 0.6, 0.6);
        nvSpace_enable_multithreading(space, 0);
    }

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