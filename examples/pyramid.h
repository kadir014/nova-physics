/*

  This file is a part of the Nova Physics Engine
  project and distributed under the MIT license.

  Copyright © Kadir Aksoy
  https://github.com/kadir014/nova-physics

*/

#include "example.h"


void PyramidExample_setup(Example *example) {
    nvSpace *space = example->space;
    
    // Create ground
    nvBody *ground = nv_Rect_new(
        nvBodyType_STATIC,
        NV_VEC2(64.0, 62.5),
        0.0,
        nvMaterial_CONCRETE,
        185.0, 5.0
    );

    nvSpace_add(space, ground);


    // Create bricks of the pyramid

    size_t floors = 32; // Height of the pyramid
    nv_float size = 1.5; // Brick size
    nv_float s2 = size / 2.0;
    nv_float y_gap = 0.0;

    for (size_t y = 0; y < floors; y++) {
        for (size_t x = 0; x < floors - y; x++) {

            nvBody *brick = nv_Rect_new(
                nvBodyType_DYNAMIC,
                NV_VEC2(
                    example->width / 20.0 - (floors * s2 - s2) + x * size + y * s2,
                    62.5 - 2.5 - s2 - y * (size + y_gap)
                ),
                0.0,
                nvMaterial_BASIC,
                size, size
            );

            nvSpace_add(space, brick);
        }
    }

    if (space->broadphase_algorithm == nvBroadPhaseAlg_SPATIAL_HASH_GRID)
        nvSpace_set_SHG(space, space->shg->bounds, 1.6, 1.6);
}