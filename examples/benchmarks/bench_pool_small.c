/*

  This file is a part of the Nova Physics Engine
  project and distributed under the MIT license.

  Copyright © Kadir Aksoy
  https://github.com/kadir014/nova-physics

*/

#include "novaphysics/novaphysics.h"
#include "benchmark_base.h"


/*
    ┌┐       ┌┐ Small Pool benchmark
    ││ooooooo││ 324 (27x12) balls
    ││ooooooo││
    │└───────┘│
    └─────────┘
*/


int main(int argc, char *argv[]) {
    nv_Space *space = nv_Space_new();


    nv_Body *wall_bottom = nv_Rect_new(
        nv_BodyType_STATIC,
        NV_VEC2(64.0, 67.0),
        0.0,
        nv_Material_STEEL,
        55.0, 5.0
    );

    nv_Space_add(space, wall_bottom);

    nv_Body *wall_left = nv_Rect_new(
        nv_BodyType_STATIC,
        NV_VEC2(34.0, 54.5),
        0.0,
        nv_Material_STEEL,
        5.0, 30.0
    );

    nv_Space_add(space, wall_left);

    nv_Body *wall_right = nv_Rect_new(
        nv_BodyType_STATIC,
        NV_VEC2(94.5, 54.5),
        0.0,
        nv_Material_STEEL,
        5.0, 30.0
    );

    nv_Space_add(space, wall_right);


    for (size_t y = 0; y < 12; y++) {
        for (size_t x = 0; x < 27; x++) {
            nv_Body *ball = nv_Circle_new(
                nv_BodyType_DYNAMIC,
                NV_VEC2(34.5 + 3.0 + x*2.0, 40.5 + y*2.0),
                0.0,
                nv_Material_WOOD,
                1.0
            );

            nv_Space_add(space, ball);
        }
    }

    PrecisionTimer timer;

    unsigned long long n;
    if (argc == 2) n = atoll(argv[1]);
    else n = 999;

    int iters = 8;
    int substeps = 1;

    double times[n];

    for (size_t i = 0; i < n; i++) {
        PrecisionTimer_start(&timer);

        nv_Space_step(space, 1.0/60.0, 8, 1);

        PrecisionTimer_stop(&timer);
        times[i] = timer.elapsed;

        if (i % (n / 10) == 0) {
            printf("%ld%% ", (long)((double)i / (double)n * 100.0));
        }
    }
    printf("100%%\n");

    Stats stats1;
    calculate_stats(&stats1, times, n);
    print_stats(stats1, n, 8, 1);


    nv_Space_free(space);
}