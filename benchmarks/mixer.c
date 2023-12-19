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
 * @file mixer.c
 * 
 * @brief Mixer benchmark. 1500 objects constantly moving.
 */


enum {
    BENCHMARK_ITERS = 6000,
    BENCHMARK_HERTZ = 60,
    BENCHMARK_VELOCITY_ITERATIONS = 10,
    BENCHMARK_POSITION_ITERATIONS = 10,
    BENCHMARK_CONSTRAINT_ITERATIONS = 5
};


void update(nvSpace *space, int counter) {
    if (counter == 0) return;

    nvBody *mixer = space->bodies->data[5];

    nv_float angle = ((nv_float)counter) / 25.0;

    nvVector2 next_pos = NV_VEC2(
        nv_cos(angle) * 17.0 + 64.0,
        nv_sin(angle) * 17.0 + (72.0 - 25.0)
    );

    nvVector2 delta = nvVector2_sub(next_pos, mixer->position);

    mixer->linear_velocity = nvVector2_add(mixer->linear_velocity, delta);
}


int main(int argc, char *argv[]) {
    // Setup benchmark

    nvSpace *space = nvSpace_new();

    Benchmark bench = Benchmark_new(BENCHMARK_ITERS, space);

    nvMaterial ground_mat = (nvMaterial){1.0, 0.1, 0.6};
    nvMaterial mixer_mat = (nvMaterial){5.0, 0.03, 0.1};
    nvMaterial basic_mat = (nvMaterial){1.0, 0.0, 0.25};

    nvBody *ground = nvBody_new(
        nvBodyType_STATIC,
        nvRectShape_new(80.0, 5.0),
        NV_VEC2(64.0, 72.0 - 2.5),
        0.0,
        ground_mat
    );

    nvSpace_add(space, ground);

    nvBody *ceiling = nvBody_new(
        nvBodyType_STATIC,
        nvRectShape_new(80.0, 5.0),
        NV_VEC2(64.0, 2.5),
        0.0,
        ground_mat
    );

    nvSpace_add(space, ceiling);

    nvBody *wall_l = nvBody_new(
        nvBodyType_STATIC,
        nvRectShape_new(5.0, 75.0),
        NV_VEC2(64.0 - 40.0 + 2.5, 36.0),
        0.0,
        ground_mat
    );

    nvSpace_add(space, wall_l);

    nvBody *wall_r = nvBody_new(
        nvBodyType_STATIC,
        nvRectShape_new(5.0, 75.0),
        NV_VEC2(64.0 + 40.0 - 2.5, 36.0),
        0.0,
        ground_mat
    );

    nvSpace_add(space, wall_r);

    nvBody *mixer = nvBody_new(
        nvBodyType_DYNAMIC,
        nvCircleShape_new(4.0),
        NV_VEC2(94.0, 72.0 - 25.0),
        0.0,
        mixer_mat
    );

    nvSpace_add(space, mixer);

    // Create stacked mixed shapes

    int cols = 50; // Columns of the stack
    int rows = 30; // Rows of the stack
    nv_float size = 1.33; // Size of the shapes
    nv_float s2 = size * 2.0;

    for (size_t y = 0; y < rows; y++) {

        for (size_t x = 0; x < cols; x++) {

            int r = (x + y) % 4;

            nvBody *body;

            // Circle
            if (r == 0) {
                body = nvBody_new(
                    nvBodyType_DYNAMIC,
                    nvCircleShape_new(size / 2.0),
                    NV_VEC2(
                        64.0 - 2.3 - ((nv_float)cols * size) / 2.0 + s2 + size * x,
                        62.5 - 2.5 - s2 - y * size
                    ),
                    0.0,
                    basic_mat
                );
            }

            // Box
            else if (r == 1) {
                body = nvBody_new(
                    nvBodyType_DYNAMIC,
                    nvRectShape_new(size, size),
                    NV_VEC2(
                        64.0 - 2.3 - ((nv_float)cols * size) / 2.0 + s2 + size * x,
                        62.5 - 2.5 - s2 - y * size
                    ),
                    0.0,
                    basic_mat
                );
            }

            // Pentagon
            else if (r == 2) {
                body = nvBody_new(
                    nvBodyType_DYNAMIC,
                    nvNGonShape_new(5, size),
                    NV_VEC2(
                        64.0 - 2.3 - ((nv_float)cols * size) / 2.0 + s2 + size * x,
                        62.5 - 2.5 - s2 - y * size
                    ),
                    0.0,
                    basic_mat
                );
            }

            // Triangle
            else if (r == 3) {
                body = nvBody_new(
                    nvBodyType_DYNAMIC,
                    nvNGonShape_new(3, size),
                    NV_VEC2(
                        64.0 - 2.3 - ((nv_float)cols * size) / 2.0 + s2 + size * x,
                        62.5 - 2.5 - s2 - y * size
                    ),
                    0.0,
                    basic_mat
                );
            }

            nvSpace_add(space, body);
        }
    }

    if (space->broadphase_algorithm == nvBroadPhaseAlg_SPATIAL_HASH_GRID)
        nvSpace_set_SHG(space, space->shg->bounds, size + size * 0.2, size + size * 0.2);


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