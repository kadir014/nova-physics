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
 * @brief Mixer benchmark. 6000 steps, 1500 objects constantly moving.
 */


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
    // Create benchmark
    Benchmark bench = Benchmark_new(6000);


    // Setup benchmark

    nvSpace *space = nvSpace_new();

    nvMaterial ground_mat = (nvMaterial){1.0, 0.1, 0.6};
    nvMaterial mixer_mat = (nvMaterial){5.0, 0.03, 0.1};
    nvMaterial basic_mat = (nvMaterial){1.0, 0.0, 0.25};

    nvBody *ground = nv_Rect_new(
        nv_BodyType_STATIC,
        NV_VEC2(64.0, 72.0 - 2.5),
        0.0,
        ground_mat,
        80, 5.0
    );

    nvSpace_add(space, ground);

    nvBody *ceiling = nv_Rect_new(
        nv_BodyType_STATIC,
        NV_VEC2(64.0, 2.5),
        0.0,
        ground_mat,
        80, 5.0
    );

    nvSpace_add(space, ceiling);

    nvBody *wall_l = nv_Rect_new(
        nv_BodyType_STATIC,
        NV_VEC2(64.0 - 40.0 + 2.5, 36.0),
        0.0,
        ground_mat,
        5.0, 75.0
    );

    nvSpace_add(space, wall_l);

    nvBody *wall_r = nv_Rect_new(
        nv_BodyType_STATIC,
        NV_VEC2(64.0 + 40.0 - 2.5, 36.0),
        0.0,
        ground_mat,
        5.0, 75.0
    );

    nvSpace_add(space, wall_r);

    nvBody *mixer = nv_Circle_new(
        nv_BodyType_DYNAMIC,
        NV_VEC2(94.0, 72.0 - 25.0),
        0.0,
        mixer_mat,
        4.0
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
                body = nv_Circle_new(
                    nv_BodyType_DYNAMIC,
                    NV_VEC2(
                        64.0 - 2.3 - ((nv_float)cols * size) / 2.0 + s2 + size * x,
                        62.5 - 2.5 - s2 - y * size
                    ),
                    0.0,
                    basic_mat,
                    size / 2.0
                );
            }

            // Box
            else if (r == 1) {
                body = nv_Rect_new(
                    nv_BodyType_DYNAMIC,
                    NV_VEC2(
                        64.0 - 2.3 - ((nv_float)cols * size) / 2.0 + s2 + size * x,
                        62.5 - 2.5 - s2 - y * size
                    ),
                    0.0,
                    basic_mat,
                    size, size
                );
            }

            // Pentagon
            else if (r == 2) {
                nvArray *vertices = nvArray_new();

                nvVector2 radius = {size / 2.0, 0.0};

                for (size_t i = 0; i < 5; i++) {
                    nvArray_add(vertices, NV_VEC2_NEW(radius.x, radius.y));
                    radius = nvVector2_rotate(radius, 2.0 * NV_PI / 5.0); // 2pi/5 = 72 degrees
                }

                body = nv_Polygon_new(
                    nv_BodyType_DYNAMIC,
                    NV_VEC2(
                        64.0 - 2.3 - ((nv_float)cols * size) / 2.0 + s2 + size * x,
                        62.5 - 2.5 - s2 - y * size
                    ),
                    0.0,
                    basic_mat,
                    vertices
                );
            }

            // Triangle
            else if (r == 3) {
                nvVector2 radius = {size / 2.0, 0.0};
                nvArray *vertices = nvArray_new();
                for (size_t i = 0; i < 3; i++) {
                    nvArray_add(vertices, NV_VEC2_NEW(radius.x, radius.y));
                    radius = nvVector2_rotate(radius, 2.0 * NV_PI / 3.0); // 2pi/3 = 120 degrees
                }

                body = nv_Polygon_new(
                    nv_BodyType_DYNAMIC,
                    NV_VEC2(
                        64.0 - 2.3 - ((nv_float)cols * size) / 2.0 + s2 + size * x,
                        62.5 - 2.5 - s2 - y * size
                    ),
                    0.0,
                    basic_mat,
                    vertices
                );
            }

            nvSpace_add(space, body);
        }
    }

    nvSpace_set_SHG(space, space->shg->bounds, 1.5, 1.5);


    // Space step settings
    nv_float dt = 1.0 / 60.0;
    unsigned int v_iters = 10;
    unsigned int p_iters = 10;
    unsigned int c_iters = 1;
    unsigned int substeps = 1;

    // Run benchmark
    for (size_t i = 0; i < bench.iters; i++) {
        update(space, i);

        Benchmark_start(&bench);

        nvSpace_step(space, dt, v_iters, p_iters, c_iters, substeps);

        Benchmark_stop(&bench, space);
    }
    
    Benchmark_results(&bench, false);


    nvSpace_free(space);
}