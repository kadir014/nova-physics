/*

  This file is a part of the Nova Physics Engine
  project and distributed under the MIT license.

  Copyright © Kadir Aksoy
  https://github.com/kadir014/nova-physics

*/

#include "novaphysics/novaphysics.h"


/*
    ┌┐       ┌┐ Small Pool benchmark
    ││ooooooo││ 324 (27x12) balls
    ││ooooooo││
    │└───────┘│
    └─────────┘
*/


int main() {
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


    nv_Space_free(space);
}