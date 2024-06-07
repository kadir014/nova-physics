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
 * @file ball_pool.c
 * 
 * @brief Ball pool benchmark. 9000 objects with average movement.
 */


enum {
    BENCHMARK_ITERS = 4000,
    BENCHMARK_HERTZ = 60,
    BENCHMARK_VELOCITY_ITERATIONS = 10,
    BENCHMARK_POSITION_ITERATIONS = 10,
    BENCHMARK_CONSTRAINT_ITERATIONS = 5
};


int main(int argc, char *argv[]) {
    // Setup benchmark

    nvSpace *space = nvSpace_new();

    Benchmark bench = Benchmark_new(BENCHMARK_ITERS, space);

    nvMaterial ground_mat = (nvMaterial){1.0, 0.0, 0.7};

    nvRigidBody *ground = nvRigidBody_new(
        nvRigidBodyType_STATIC,
        nvRectShape_new(128.0, 5.0),
        NV_VECTOR2(64.0, 74.0),
        0.0,
        ground_mat
    );

    nvSpace_add(space, ground);

    nvRigidBody *ceiling = nvRigidBody_new(
        nvRigidBodyType_STATIC,
        nvRectShape_new(128.0, 5.0),
        NV_VECTOR2(64.0, -2.0),
        0.0,
        ground_mat
    );

    nvSpace_add(space, ceiling);

    nvRigidBody *wall_left = nvRigidBody_new(
        nvRigidBodyType_STATIC,
        nvRectShape_new(5.0, 100.0),
        NV_VECTOR2(64.0 - 50.0, 36.0),
        0.0,
        ground_mat
    );

    nvSpace_add(space, wall_left);

    nvRigidBody *wall_right = nvRigidBody_new(
        nvRigidBodyType_STATIC,
        nvRectShape_new(5.0, 100.0),
        NV_VECTOR2(64.0 + 50.0, 36.0),
        0.0,
        ground_mat
    );

    nvSpace_add(space, wall_right);

    size_t rows = 90;
    size_t cols = 100;
    nv_float size = 0.75;

    for (size_t y = 0; y < rows; y++) {
        for (size_t x = 0; x < cols; x++) {
            nvRigidBody *ball = nvRigidBody_new(
                nvRigidBodyType_DYNAMIC,
                nvCircleShape_new(size / 2.0),
                NV_VECTOR2(
                    64.0-50.0 + size*4.0 + ((nv_float)x) * size + (nv_float)((x*x + y*y) % 10) / 10.0,
                    70.0 - ((nv_float)y) * size
                ),
                0.0,
                (nvMaterial){1.0, 0.0, 0.0}
            );

            nvSpace_add(space, ball);
        }
    }

    if (space->broadphase_algorithm == nvBroadPhaseAlg_SHG) {
        nvSpace_set_SHG(space, space->shg->bounds, 0.75, 0.75);
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