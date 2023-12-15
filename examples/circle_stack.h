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
   nvBody *ground = nv_Rect_new(
        nvBodyType_STATIC,
        (nvVector2){64.0, 70.0},
        0.0,
        nvMaterial_CONCRETE,
        128.0, 5.0
    );

    nvSpace_add(space, ground);

    nvBody *wall_l = nv_Rect_new(
        nvBodyType_STATIC,
        (nvVector2){2.0, 36.0},
        0.0,
        nvMaterial_CONCRETE,
        5.0, 72.0
    );

    nvSpace_add(space, wall_l);

    nvBody *wall_r = nv_Rect_new(
        nvBodyType_STATIC,
        (nvVector2){126.0, 36.0},
        0.0,
        nvMaterial_CONCRETE,
        5.0, 72.0
    );

    nvSpace_add(space, wall_r);

    // Some basic material with no restitution (inelastic)
    nvMaterial basic_material = {
        .density = 1.0,
        .restitution = 0.0,
        .friction = nvMaterial_WOOD.friction,
    };

    // Create stacked circles

    int cols = 35; // Columns of the stack
    int rows = 25; // Rows of the stack
    double size = 1.0; // Size of the circles
    double s2 = size * 2.0;

    for (size_t y = 0; y < rows; y++) {

        for (size_t x = 0; x < cols; x ++) {

            nvBody *ball = nv_Circle_new(
                nvBodyType_DYNAMIC,
                NV_VEC2(
                    example->width / 20.0 - ((double)cols * s2) / 2.0 + size + s2 * x,
                    62.5 - 2.5 - size - y * s2
                ),
                0.0,
                basic_material,
                size
            );

            nvSpace_add(space, ball);
        }
    }
}