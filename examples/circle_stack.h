/*

  This file is a part of the Nova Physics Engine
  project and distributed under the MIT license.

  Copyright © Kadir Aksoy
  https://github.com/kadir014/nova-physics

*/

#include "example.h"


void CircleStackExample_setup(Example *example) {
    nvSpace *space = example->space;
    
    // Create ground & walls
    nvBody *ground = nvBody_new(
        nvBodyType_STATIC,
        nvRectShape_new(128.0, 5.0),
        (nvVector2){64.0, 70.0},
        0.0,
        nvMaterial_CONCRETE
    );

    nvSpace_add(space, ground);

    // Create stacked circles

    int cols = 12; // Columns of the stack
    int rows = 30; // Rows of the stack
    double size = 1.0; // Size of the circles
    double s2 = size * 2.0;

    for (size_t y = 0; y < rows; y++) {

        for (size_t x = 0; x < cols; x ++) {

            nvBody *ball = nvBody_new(
                nvBodyType_DYNAMIC,
                nvCircleShape_new(size),
                NV_VEC2(
                    128.0 / 2.0 - 38.0 - ((double)cols * s2) / 2.0 + size + s2 * (x * 4.5),
                    62.5 - 2.5 - size - y * s2
                ),
                0.0,
                nvMaterial_BASIC
            );

            nvSpace_add(space, ball);
        }
    }

    if (space->broadphase_algorithm == nvBroadPhaseAlg_SPATIAL_HASH_GRID)
        nvSpace_set_SHG(space, space->shg->bounds, 2.0, 2.0);
}