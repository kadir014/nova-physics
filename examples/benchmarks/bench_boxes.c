/*

  This file is a part of the Nova Physics Engine
  project and distributed under the MIT license.

  Copyright © Kadir Aksoy
  https://github.com/kadir014/nova-physics

*/

#include "novaphysics/novaphysics.h"
#include "benchmark_base.h"


/*
    ┌┐         ┌┐ Boxes benchmark
    ││□◇□◇□◇□◇□││ 660 (20x33) rotated boxes
    ││◇□◇□◇□◇□◇││
    ││□◇□◇□◇□◇□││
    │└─────────┘│
    └───────────┘
*/


int main(int argc, char *argv[]) {
    nv_Space *space = nv_Space_new();


    nv_Body *wall_bottom = nv_Rect_new(
        nv_BodyType_STATIC,
        NV_VEC2(64.0, 70.0),
        0.0,
        nv_Material_STEEL,
        110.0, 5.0
    );

    nv_Space_add(space, wall_bottom);

    nv_Body *wall_left = nv_Rect_new(
        nv_BodyType_STATIC,
        NV_VEC2(11.5, 37.5),
        0.0,
        nv_Material_STEEL,
        5.0, 60.0
    );

    nv_Space_add(space, wall_left);

    nv_Body *wall_right = nv_Rect_new(
        nv_BodyType_STATIC,
        NV_VEC2(116.5, 37.5),
        0.0,
        nv_Material_STEEL,
        5.0, 60.0
    );

    nv_Space_add(space, wall_right);


    for (size_t y = 0; y < 20; y++) {
        for (size_t x = 0; x < 33; x++) {
            nv_Body *box = nv_Rect_new(
                nv_BodyType_DYNAMIC,
                NV_VEC2(11.5 + 4.2 + x*3.0, 8.5 + y*3.0),
                x / 10.0 + y / 10.0,
                nv_Material_WOOD,
                2.2, 2.2
            );

            nv_Space_add(space, box);
        }
    }

    PrecisionTimer timer;

    unsigned long long n;
    if (argc == 2) n = atoll(argv[1]);
    else n = 999;

    int iters = 10;
    int substeps = 3;

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