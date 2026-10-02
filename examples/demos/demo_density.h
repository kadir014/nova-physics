/*

  This file is a part of the Nova Physics Engine
  project and distributed under the MIT license.

  Copyright © Kadir Aksoy
  https://github.com/kadir014/nova-physics

*/

#include "../common.h"


void Density_setup(ExampleContext *example) {
    nvRigidBodyInitializer body_init = nvRigidBodyInitializer_default;
    body_init.position = NV_VECTOR2(64.0f, 45.0f);
    nvRigidBody *bowl = nvRigidBody_new(body_init);

    nvRigidBody_add_shape(bowl, nvBoxShape_new(45.0f, 1.0f, NV_VECTOR2(0.0f, 12.5f)));
    nvRigidBody_add_shape(bowl, nvBoxShape_new(1.0f, 25.0f, NV_VECTOR2(-22.5f, 0.0f)));
    nvRigidBody_add_shape(bowl, nvBoxShape_new(1.0f, 25.0f, NV_VECTOR2(22.5f, 0.0f)));

    nvSpace_add_rigidbody(example->space, bowl);


    // Box bodies with density 1.0
    body_init.type = nvRigidBodyType_DYNAMIC;
    for (size_t i = 0; i < 600; i++) {
        body_init.position = NV_VECTOR2(frand(64.0f - 22.0f, 64.0f + 22.0f), frand(45.0f - 10.0f, 45.0f + 12.0f));
        body_init.angle = frand(-NV_PI, NV_PI);

        nvRigidBody *box = nvRigidBody_new(body_init);

        nvShape *box_shape = nvBoxShape_new(0.7f, 1.3f, nvVector2_zero);
        nvRigidBody_add_shape(box, box_shape);

        nvSpace_add_rigidbody(example->space, box);
    }


    // Dense balls
    body_init.material.density = 50.0f;
    for (size_t i = 0; i < 3; i++) {
        body_init.position = NV_VECTOR2(64.0f - 15.0f + (nv_float)i * 14.0f, 45.0f - 20.0f);

        nvRigidBody *ball = nvRigidBody_new(body_init);

        nvShape *ball_shape = nvCircleShape_new(nvVector2_zero, 1.0f);
        nvRigidBody_add_shape(ball, ball_shape);

        nvSpace_add_rigidbody(example->space, ball);
    }
}

void Density_update(ExampleContext *example) {}