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
    ground_init.position = NV_VECTOR2(64.0, 72.0 - 2.5);
    ground = nvRigidBody_new(ground_init);

    nvShape *ground_shape = nvBoxShape_new(128.0, 5.0, nvVector2_zero);
    nvRigidBody_add_shape(ground, ground_shape);

    nvSpace_add_rigidbody(example->space, ground);


    nv_float w = 4.0;
    for (size_t y = 0; y < 10; y++) {
        for (size_t x = 0; x < 10; x++) {

            nvRigidBody *body;
            nvRigidBodyInitializer body_init = nvRigidBodyInitializer_default;
            body_init.type = nvRigidBodyType_DYNAMIC;
            body_init.position = NV_VECTOR2(
                64.0 - w * ((nv_float)stack_cols * 0.5) + x * w,
                50.0 - y * w
            );
            body_init.material = (nvMaterial){.density=1.0, .restitution=0.2, .friction=0.3};
            body = nvRigidBody_new(body_init);

            nv_uint32 corners = u32rand(4, 8);
            add_star_shape(body, corners, 2.0);

            nvSpace_add_rigidbody(example->space, body);
        }
    }
}

void Compound_update(ExampleContext *example) {}