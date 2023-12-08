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
 * @brief Boxes benchmark. 5000 steps, 3500 objects minimal movement.
 */


int main(int argc, char *argv[]) {
    // Create benchmark
    Benchmark bench = Benchmark_new(5000);


    // Setup benchmark

    nvSpace *space = nvSpace_new();

    nvBody *ground = nv_Rect_new(
        nv_BodyType_STATIC,
        NV_VEC2(64.0, 72),
        0.0,
        (nvMaterial){1.0, 0.1, 0.7},
        89.0, 5.0
    );

    nvSpace_add(space, ground);

    nvBody *wall_l = nv_Rect_new(
        nv_BodyType_STATIC,
        NV_VEC2(22.0, 36.0),
        0.0,
        (nvMaterial){1.0, 0.1, 0.7},
        5.0, 80.0
    );

    nvSpace_add(space, wall_l);

    nvBody *wall_r = nv_Rect_new(
        nv_BodyType_STATIC,
        NV_VEC2(128.0 - 22.0, 36.0),
        0.0,
        (nvMaterial){1.0, 0.1, 0.7},
        5.0, 80.0
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

            nvBody *box = nv_Rect_new(
                nv_BodyType_DYNAMIC,
                NV_VEC2(
                    1280.0 / 20.0 - (nv_float)cols * s2 + s2 + size * x,
                    starty - size - y * (size + ygap)
                ),
                0.0,
                (nvMaterial){1.0, 0.1, 0.2},
                sizen, sizen
            );

            nvSpace_add(space, box);
        }
    }

    nvSpace_set_SHG(space, space->shg->bounds, 1.9, 1.9);


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
    
    Benchmark_results(&bench, true);


    nvSpace_free(space);
}