/*

  This file is a part of the Nova Physics Engine
  project and distributed under the MIT license.

  Copyright © Kadir Aksoy
  https://github.com/kadir014/nova-physics

*/

#include "example.h"


void StackExample_setup(Example *example) {
    nvSpace *space = example->space;
    
    // Create ground & walls
    nvBody *ground = nvBody_new(
        nvBodyType_STATIC,
        nvRectShape_new(128.0, 5.0),
        NV_VEC2(64.0, 70.0),
        0.0,
        (nvMaterial){1.0, 0.1, 0.65}
    );

    nvSpace_add(space, ground);

    nv_float offsets[20] = {
        -0.3, 0.1, 0.0, 0.2, -0.15,
        0.05, -0.09, 0.04, -0.1, 0.3,
        0.2, 0.24, -0.017, 0.17, 0.03,
        0.3, 0.0, -0.06, 0.25, 0.08
    };

    // Create stacked boxes

    int cols = 12;
    int rows = 20;
    nv_float size = 3.0;
    nv_float s2 = size / 2.0;
    nv_float gap = 0.0;

    size_t x = 0;
    size_t y = 0;

    nv_float horizontal_offset = 0.0;

    for (y = 0; y < rows; y++) {
        for (x = 0; x < cols; x ++) {
            if (y > x + 8) continue;

            nv_float offset = offsets[(x + y) % 20] * horizontal_offset;

            nvBody *box = nvBody_new(
                nvBodyType_DYNAMIC,
                nvRectShape_new(size, size),
                NV_VEC2(
                    128.0 / 2.0 - 25.0 - ((nv_float)cols * size) / 2.0 + s2 + size * x + offset + (x * 4.5),
                    70 - 2.5 - s2 - y * (size + gap)
                ),
                0.0,
                (nvMaterial){1.0, 0.0, 0.5}
            );

            nvSpace_add(space, box);
        }
    }


    if (space->broadphase_algorithm == nvBroadPhaseAlg_SPATIAL_HASH_GRID)
        nvSpace_set_SHG(space, space->shg->bounds, 3.8, 3.8);
}