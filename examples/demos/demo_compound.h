/*

  This file is a part of the Nova Physics Engine
  project and distributed under the MIT license.

  Copyright © Kadir Aksoy
  https://github.com/kadir014/nova-physics

*/

#include "../common.h"


void Compound_setup(ExampleContext *example) {
    nvRigidBody *ground;
    nvRigidBodyInitializer ground_init = nvRigidBodyInitializer_default;
    ground_init.position = NV_VECTOR2(64.0f, 72.0f - 2.5f);
    ground = nvRigidBody_new(ground_init);

    nvShape *ground_shape = nvBoxShape_new(128.0f, 5.0f, nvVector2_zero);
    nvRigidBody_add_shape(ground, ground_shape);

    nvSpace_add_rigidbody(example->space, ground);


    nv_float w = 4.0;
    for (size_t y = 0; y < 10; y++) {
        for (size_t x = 0; x < 10; x++) {

            nvRigidBody *body;
            nvRigidBodyInitializer body_init = nvRigidBodyInitializer_default;
            body_init.type = nvRigidBodyType_DYNAMIC;
            body_init.position = NV_VECTOR2(
                64.0f - w * (10.0f * 0.5f) + x * w,
                50.0f - y * w
            );
            body_init.material = (nvMaterial){.density=1.0f, .restitution=0.2f, .friction=0.3f};
            body = nvRigidBody_new(body_init);

            nv_uint32 corners = u32rand(4, 8);
            add_star_shape(body, corners, 2.0f);

            nvSpace_add_rigidbody(example->space, body);
        }
    }
}

void Compound_update(ExampleContext *example) {}