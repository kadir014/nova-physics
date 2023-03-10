#include <stdio.h>
#include "benchmark_base.h"
#include "novaphysics/novaphysics.h"
#include <unistd.h>


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


    // Create stacked mixed shapes

    int cols = 30; // Columns of the stack
    int rows = 50; // Rows of the stack
    nv_float size = 1.33; // Size of the shapes
    nv_float s2 = size * 2.0;

    for (size_t y = 0; y < rows; y++) {
        for (size_t x = 0; x < cols; x++) {

            int r = (x + y) % 4;

            nv_Body *body;

            // Circle
            if (r == 0) {
                body = nv_Circle_new(
                    nv_BodyType_DYNAMIC,
                    NV_VEC2(
                        1280.0 / 20.0 - 2.3 - ((nv_float)cols * size) / 2.0 + s2 + size * x,
                        62.5 - 2.5 - s2 - y * size
                    ),
                    0.0,
                    nv_Material_BASIC,
                    size / 2.0
                );
            }

            // Box
            else if (r == 1) {
                body = nv_Rect_new(
                    nv_BodyType_DYNAMIC,
                    NV_VEC2(
                        1280.0 / 20.0 - 2.3 - ((nv_float)cols * size) / 2.0 + s2 + size * x,
                        62.5 - 2.5 - s2 - y * size
                    ),
                    0.0,
                    nv_Material_BASIC,
                    size, size
                );
            }

            // Pentagon
            else if (r == 2) {
                nv_Array *vertices = nv_Array_new();

                nv_Vector2 radius = {size / 2.0, 0.0};

                for (size_t i = 0; i < 5; i++) {
                    nv_Array_add(vertices, NV_VEC2_NEW(radius.x, radius.y));
                    radius = nv_Vector2_rotate(radius, 2.0 * NV_PI / 5.0); // 2pi/5 = 72 degrees
                }

                body = nv_Polygon_new(
                    nv_BodyType_DYNAMIC,
                    NV_VEC2(
                        1280.0 / 20.0 - 2.3 - ((nv_float)cols * size) / 2.0 + s2 + size * x,
                        62.5 - 2.5 - s2 - y * size
                    ),
                    0.0,
                    nv_Material_BASIC,
                    vertices
                );
            }

            // Triangle
            else if (r == 3) {
                nv_Vector2 radius = {size / 2.0, 0.0};
                nv_Array *vertices = nv_Array_new();
                for (size_t i = 0; i < 3; i++) {
                    nv_Array_add(vertices, NV_VEC2_NEW(radius.x, radius.y));
                    radius = nv_Vector2_rotate(radius, 2.0 * NV_PI / 3.0); // 2pi/3 = 120 degrees
                }

                body = nv_Polygon_new(
                    nv_BodyType_DYNAMIC,
                    NV_VEC2(
                        1280.0 / 20.0 - 2.3 - ((nv_float)cols * size) / 2.0 + s2 + size * x,
                        62.5 - 2.5 - s2 - y * size
                    ),
                    0.0,
                    nv_Material_BASIC,
                    vertices
                );
            }

            nv_Space_add(space, body);
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