/*

  This file is a part of the Nova Physics Engine
  project and distributed under the MIT license.

  Copyright © Kadir Aksoy
  https://github.com/kadir014/nova-physics

*/

#include "example.h"


void VaryingBounceExample_setup(Example *example) {
    nvSpace *space = example->space;
    
    nvMaterial ground_mat = {
        .density = 1.0,
        .restitution = 1.0,
        .friction = 0.0
    };

    // Create ground 
    nvBody *ground = nvBody_new(
        nvBodyType_STATIC,
        nvRectShape_new(185.0, 5.0),
        (nvVector2){64.0, 62.5},
        0.0,
        ground_mat
    );

    nvSpace_add(space, ground);

    for (size_t i = 0; i < 5; i++) {

        nvMaterial material = {
            .density = 1.0,
            .restitution = (nv_float)i / 4.0,
            .friction = 0.0
        };

        nvBody *ball = nvBody_new(
            nvBodyType_DYNAMIC,
            nvCircleShape_new(4.0),
            NV_VEC2(45.0 + (i * (8.0 + 1.0)), 20.0),
            0.0,
            material
        );

        nvSpace_add(space, ball);
    }
}