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
    nvBody *ground = nvBody_new(
        nvBodyType_STATIC,
        nvRectShape_new(200.0, 5.0),
        NV_VEC2(64.0, 62.5),
        0.0,
        nvMaterial_CONCRETE
    );

    nvSpace_add(space, ground);


    // Create bricks of the pyramid

    size_t base = get_slider_setting("Pyramid base");
    nv_float size = get_slider_setting("Box size");
    nv_float s2 = size / 2.0;
    nv_float y_gap = get_slider_setting("Air gap");

    for (size_t y = 0; y < base; y++) {
        for (size_t x = 0; x < base - y; x++) {

            nvBody *brick = nvBody_new(
                nvBodyType_DYNAMIC,
                nvRectShape_new(size, size),
                NV_VEC2(
                    128.0 / 2.0 - (base * s2 - s2) + x * size + y * s2,
                    62.5 - 2.5 - s2 - y * (size + y_gap)
                ),
                0.0,
                nvMaterial_BASIC
            );

            nvSpace_add(space, brick);
        }
    }

    if (space->broadphase_algorithm == nvBroadPhaseAlg_SPATIAL_HASH_GRID)
        nvSpace_set_SHG(space, space->shg->bounds, size + (size * 0.2), size + (size * 0.2));
}


void PyramidExample_init(ExampleEntry *entry) {
    add_slider_setting(entry, "Pyramid base", SliderType_INTEGER, 32, 3, 100);
    add_slider_setting(entry, "Box size", SliderType_FLOAT, 1.5, 0.5, 3.0);
    add_slider_setting(entry, "Air gap", SliderType_FLOAT, 0.0, 0.0, 1.5);
}