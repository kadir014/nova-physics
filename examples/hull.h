/*

  This file is a part of the Nova Physics Engine
  project and distributed under the MIT license.

  Copyright © Kadir Aksoy
  https://github.com/kadir014/nova-physics

*/

#include "example.h"


void HullExample_setup(Example *example) {
    nvSpace *space = example->space;
    
    // Create ground
    nvBody *ground = nvBody_new(
        nvBodyType_STATIC,
        nvRectShape_new(128.0, 5.0),
        NV_VEC2(64.0, 62.5),
        0.0,
        nvMaterial_CONCRETE
    );

    nvSpace_add(space, ground);

    for (size_t i = 0; i < 20; i++) {
        nvArray *points = nvArray_new();
        for (size_t j = 0; j < 15; j++) {
            nvArray_add(points, NV_VEC2_NEW(frand(-7.0, 7.0), frand(-7.0, 7.0)));
        }

        nvBody *rock = nvBody_new(
            nvBodyType_DYNAMIC,
            nvConvexHullShape_new(points),
            NV_VEC2(frand(50.0, 78.0), frand(17.0, 35.0)),
            0.0,
            nvMaterial_CONCRETE
        );

        nvSpace_add(space, rock);

        nvArray_free_each(points, free);
        nvArray_free(points);
    }
}