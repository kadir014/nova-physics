/*

  This file is a part of the Nova Physics Engine
  project and distributed under the MIT license.

  Copyright © Kadir Aksoy
  https://github.com/kadir014/nova-physics

*/

#include "../common.h"


void FarWorld_setup(ExampleContext *example) {
    // In my tests, 32-bit float scenes start to break at 1e7 units away from origin
    // and 64-bit floats start at 1e14 units away. (Only on one axis).

    nvVector2 far_coords = NV_VECTOR2(5e6f, 0.0f);
    example->camera = nvVector2_sub(far_coords, NV_VECTOR2(60.0f, 40.0f));

    nvRigidBodyInitializer ground_init = nvRigidBodyInitializer_default;
    ground_init.position = far_coords;
    nvRigidBody *ground = nvRigidBody_new(ground_init);

    nvRigidBody_add_shape(ground, nvBoxShape_new(31.0f, 1.0f, NV_VECTOR2(15.0f, 5.0f)));
    nvRigidBody_add_shape(ground, nvBoxShape_new(1.0f, 11.0f, NV_VECTOR2(30.0f, 0.0f)));
    nvRigidBody_add_shape(ground, nvCircleShape_new(NV_VECTOR2(0.0f, 0.0f), 5.5f));

    nvSpace_add_rigidbody(example->space, ground);


    for (size_t y = 0; y < 10; y++) {
        for (size_t x = 0; x < 10; x++) {

            nvRigidBodyInitializer body_init = nvRigidBodyInitializer_default;
            body_init.type = nvRigidBodyType_DYNAMIC;
            body_init.position = NV_VECTOR2(
                (nv_float)x * 2.0f,
                -10.0f - ((nv_float)y * 2.0f)
            );
            body_init.position = nvVector2_add(body_init.position, far_coords);
            nvRigidBody *body = nvRigidBody_new(body_init);

            nvRigidBody_add_shape(body, nvNGonShape_new(3, 1.0f, nvVector2_zero));

            nvSpace_add_rigidbody(example->space, body);
        }
    }
}

void FarWorld_update(ExampleContext *example) {}