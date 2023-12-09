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
 * @brief Ball pool benchmark. 4000 steps, 9000 objects average movement.
 */


int main(int argc, char *argv[]) {
    // Create benchmark
    Benchmark bench = Benchmark_new(4000);


    // Setup benchmark

    nvSpace *space = nvSpace_new();

    nvMaterial ground_mat = (nvMaterial){1.0, 0.0, 0.7};

    nvBody *ground = nv_Rect_new(
        nvBodyType_STATIC,
        NV_VEC2(64.0, 74.0),
        0.0,
        ground_mat,
        128.0, 5.0
    );

    nvSpace_add(space, ground);

    nvBody *ceiling = nv_Rect_new(
        nvBodyType_STATIC,
        NV_VEC2(64.0, -2.0),
        0.0,
        ground_mat,
        128.0, 5.0
    );

    nvSpace_add(space, ceiling);

    nvBody *wall_left = nv_Rect_new(
        nvBodyType_STATIC,
        NV_VEC2(64.0 - 50.0, 36.0),
        0.0,
        ground_mat,
        5.0, 100.0
    );

    nvSpace_add(space, wall_left);

    nvBody *wall_right = nv_Rect_new(
        nvBodyType_STATIC,
        NV_VEC2(64.0 + 50.0, 36.0),
        0.0,
        ground_mat,
        5.0, 100.0
    );

    nvSpace_add(space, wall_right);

    size_t rows = 90;
    size_t cols = 100;
    nv_float size = 0.75;

    for (size_t y = 0; y < rows; y++) {
        for (size_t x = 0; x < cols; x++) {
            nvBody *ball = nv_Circle_new(
                nvBodyType_DYNAMIC,
                NV_VEC2(
                    64.0-50.0 + size*4.0 + ((nv_float)x) * size + (nv_float)((x*x + y*y) % 10) / 10.0,
                    70.0 - ((nv_float)y) * size
                ),
                0.0,
                (nvMaterial){1.0, 0.0, 0.0},
                size / 2.0
            );

            nvSpace_add(space, ball);
        }
    }

    nvSpace_set_SHG(space, space->shg->bounds, 0.75, 0.75);


    // Space step settings
    nv_float dt = 1.0 / 60.0;
    unsigned int v_iters = 10;
    unsigned int p_iters = 10;
    unsigned int c_iters = 1;
    unsigned int substeps = 1;

    // Run benchmark
    for (size_t i = 0; i < bench.iters; i++) {
        Benchmark_start(&bench);

        nvSpace_step(space, dt, v_iters, p_iters, c_iters, substeps);

        Benchmark_stop(&bench, space);
    }
    
    Benchmark_results(&bench, false);


    nvSpace_free(space);
}