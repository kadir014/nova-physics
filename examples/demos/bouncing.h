/*

  This file is a part of the Nova Physics Engine
  project and distributed under the MIT license.

  Copyright © Kadir Aksoy
  https://github.com/kadir014/nova-physics

*/

#include "../common.h"


void Bouncing_setup(ExampleContext *example) {
    nvRigidBody *ground;
    nvRigidBodyInitializer ground_init = nvRigidBodyInitializer_default;
    ground_init.position = NV_VECTOR2(64.0, 72.0 - 2.5);
    ground_init.material = (nvMaterial){.density=1.0, .restitution=1.0, .friction=0.5};
    ground = nvRigidBody_new(ground_init);

    nvShape *ground_shape = nvBoxShape_new(102.0, 5.0, nvVector2_zero);
    nvRigidBody_add_shape(ground, ground_shape);

    nvSpace_add_rigidbody(example->space, ground);


    for (size_t x = 0; x < 100; x++) {
        nv_float e = (nv_float)x / 100.0;

        nvRigidBody *ball;
        nvRigidBodyInitializer ball_init = nvRigidBodyInitializer_default;
        ball_init.type = nvRigidBodyType_DYNAMIC;
        ball_init.position = NV_VECTOR2(
            (nv_float)x + 14.0,
            0.0
        );
        ball_init.material = (nvMaterial){.density=1.0, .restitution=e, .friction=0.5};
        ball = nvRigidBody_new(ball_init);

        // Just under 0.5 so balls don't collide horizontally
        nvShape *shape = nvCircleShape_new(nvVector2_zero, 0.49);
        nvRigidBody_add_shape(ball, shape);

        nvSpace_add_rigidbody(example->space, ball);
    }
}

void Bouncing_update(ExampleContext *example) {}