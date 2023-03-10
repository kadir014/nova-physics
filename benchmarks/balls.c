#include <stdio.h>
#include "benchmark_base.h"
#include "novaphysics/novaphysics.h"


int main(int argc, char *argv[]) {
    // Create benchmark
    Benchmark bench = Benchmark_new(1000);


    // Setup benchmark

    nv_Space *space = nv_Space_new();

    // Create ground & walls
    nv_Body *ground = nv_Rect_new(
        nv_BodyType_STATIC,
        (nv_Vector2){64.0, 70.0},
        0.0,
        nv_Material_CONCRETE,
        52.5, 5.0
    );

    nv_Space_add(space, ground);

    nv_Body *wall_l = nv_Rect_new(
        nv_BodyType_STATIC,
        (nv_Vector2){40.0, 36.0},
        0.0,
        nv_Material_CONCRETE,
        5.0, 72.0
    );

    nv_Space_add(space, wall_l);

    nv_Body *wall_r = nv_Rect_new(
        nv_BodyType_STATIC,
        (nv_Vector2){128.0 - 40.0, 36.0},
        0.0,
        nv_Material_CONCRETE,
        5.0, 72.0
    );

    nv_Space_add(space, wall_r);

    // Create stacked circles

    int cols = 30; // Columns of the stack
    int rows = 50; // Rows of the stack
    nv_float size = 0.7; // Radius of the circles
    nv_float s2 = size * 2.0;

    for (size_t y = 0; y < rows; y++) {

        for (size_t x = 0; x < cols; x ++) {

            nv_float offset = frand(-0.2, 0.2);

            nv_Body *ball = nv_Circle_new(
                nv_BodyType_DYNAMIC,
                NV_VEC2(
                    1280.0 / 20.0 - ((double)cols * s2) / 2.0 + size + s2 * x + offset,
                    62.5 - 2.5 - size - y * s2
                ),
                0.0,
                nv_Material_BASIC,
                size
            );

            nv_Space_add(space, ball);
        }
    }


    // Space step settings
    nv_float dt = 1.0 / 60.0;
    unsigned int v_iters = 8;
    unsigned int p_iters = 3;
    unsigned int c_iters = 1;
    unsigned int substeps = 1;

    // Run benchmark
    for (size_t i = 0; i < bench.iters; i++) {
        Benchmark_start(&bench);

        nv_Space_step(space, dt, v_iters, p_iters, c_iters, substeps);

        Benchmark_stop(&bench);
    }
    
    Benchmark_results(&bench);


    nv_Space_free(space);
}