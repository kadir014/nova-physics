/*

  This file is a part of the Nova Physics Engine
  project and distributed under the MIT license.

  Copyright © Kadir Aksoy
  https://github.com/kadir014/nova-physics

*/

#ifndef NV_BENCHMARK_SCENE_PYRAMID_H
#define NV_BENCHMARK_SCENE_PYRAMID_H

#include "novaphysics/novaphysics.h"


void Pyramid_setup(nvSpace *space) {
    nvSpace_set_broadphase(space, nvBroadPhaseAlg_BVH);


    nvRigidBody *ground;
    nvRigidBodyInitializer ground_init = nvRigidBodyInitializer_default;
    ground_init.position = NV_VECTOR2(64.0f, 72.0f - 2.5f);
    ground = nvRigidBody_new(ground_init);

    nvShape *ground_shape = nvBoxShape_new(128.0f, 5.0f, nvVector2_zero);
    nvRigidBody_add_shape(ground, ground_shape);

    nvSpace_add_rigidbody(space, ground);


    size_t pyramid_base = 100;
    nv_float size = 1.0f;
    nv_float s2 = size / 2.0f;
    nv_float y_gap = 0.0f;
    nv_float start_y = 72.0f - 2.5f - 2.5f - s2;

    for (size_t y = 0; y < pyramid_base; y++) {
        for (size_t x = 0; x < pyramid_base - y; x++) {
            nvRigidBody *box;
            nvRigidBodyInitializer box_init = nvRigidBodyInitializer_default;
            box_init.type = nvRigidBodyType_DYNAMIC;
            box_init.position = NV_VECTOR2(
                64.0f - (pyramid_base * s2 - s2) + x * size + y * s2,
                start_y - y * (size + y_gap - 0.01f) // Sink a little so collisions happen in first frame
            );
            box_init.material = (nvMaterial){.density=1.0f, .restitution=0.0f, .friction=0.5f};
            box = nvRigidBody_new(box_init);

            nvShape *box_shape = nvBoxShape_new(size, size, nvVector2_zero);
            nvRigidBody_add_shape(box, box_shape);

            nvSpace_add_rigidbody(space, box);
        }
    }
}


#endif