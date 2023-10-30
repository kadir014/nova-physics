#include <stdio.h>
#include "benchmark_base.h"
#include "novaphysics/novaphysics.h"
#include <unistd.h>


int main(int argc, char *argv[]) {
    // Create benchmark
    Benchmark bench = Benchmark_new(4000);


    // Setup benchmark

    nv_Space *space = nv_Space_new();

    nv_Material ground_mat = (nv_Material){1.0, 0.0, 0.7};

    nv_Body *ground = nv_Rect_new(
        nv_BodyType_STATIC,
        NV_VEC2(64.0, 74.0),
        0.0,
        ground_mat,
        128.0, 5.0
    );

    nv_Space_add(space, ground);

    nv_Body *ceiling = nv_Rect_new(
        nv_BodyType_STATIC,
        NV_VEC2(64.0, -2.0),
        0.0,
        ground_mat,
        128.0, 5.0
    );

    nv_Space_add(space, ceiling);

    nv_Body *wall_left = nv_Rect_new(
        nv_BodyType_STATIC,
        NV_VEC2(64.0 - 50.0, 36.0),
        0.0,
        ground_mat,
        5.0, 100.0
    );

    nv_Space_add(space, wall_left);

    nv_Body *wall_right = nv_Rect_new(
        nv_BodyType_STATIC,
        NV_VEC2(64.0 + 50.0, 36.0),
        0.0,
        ground_mat,
        5.0, 100.0
    );

    nv_Space_add(space, wall_right);

    size_t rows = 90;
    size_t cols = 100;
    nv_float size = 0.75;

    for (size_t y = 0; y < rows; y++) {
        for (size_t x = 0; x < cols; x++) {
            nv_Body *ball = nv_Circle_new(
                nv_BodyType_DYNAMIC,
                NV_VEC2(
                    64.0-50.0 + size*4.0 + ((nv_float)x) * size + (nv_float)((x*x + y*y) % 10) / 10.0,
                    70.0 - ((nv_float)y) * size
                ),
                0.0,
                (nv_Material){1.0, 0.0, 0.0},
                size / 2.0
            );

            nv_Space_add(space, ball);
        }
    }

    nv_Space_set_SHG(space, space->shg->bounds, 0.75, 0.75);


    // Space step settings
    nv_float dt = 1.0 / 60.0;
    unsigned int v_iters = 10;
    unsigned int p_iters = 10;
    unsigned int c_iters = 1;
    unsigned int substeps = 1;

    // Run benchmark
    for (size_t i = 0; i < bench.iters; i++) {
        Benchmark_start(&bench);

        nv_Space_step(space, dt, v_iters, p_iters, c_iters, substeps);

        Benchmark_stop(&bench, space);
    }
    
    Benchmark_results(&bench, false);


    nv_Space_free(space);
}