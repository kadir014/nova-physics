/*

  This file is a part of the Nova Physics Engine
  project and distributed under the MIT license.

  Copyright © Kadir Aksoy
  https://github.com/kadir014/nova-physics

*/

#include "../common.h"


void Density_setup(ExampleContext *example) {
    nvRigidBodyInitializer body_init = nvRigidBodyInitializer_default;
    body_init.position = NV_VECTOR2(64.0, 45.0);
    nvRigidBody *bowl = nvRigidBody_new(body_init);

    nvRigidBody_add_shape(bowl, nvBoxShape_new(45.0, 1.0, NV_VECTOR2(0.0, 12.5)));
    nvRigidBody_add_shape(bowl, nvBoxShape_new(1.0, 25.0, NV_VECTOR2(-22.5, 0.0)));
    nvRigidBody_add_shape(bowl, nvBoxShape_new(1.0, 25.0, NV_VECTOR2(22.5, 0.0)));

    nvSpace_add_rigidbody(example->space, bowl);


    // Box bodies with density 1.0
    body_init.type = nvRigidBodyType_DYNAMIC;
    for (size_t i = 0; i < 600; i++) {
        body_init.position = NV_VECTOR2(frand(64.0 - 22.0, 64.0 + 22.0), frand(45.0 - 10.0, 45.0 + 12.0));
        body_init.angle = frand(-NV_PI, NV_PI);

        nvRigidBody *box = nvRigidBody_new(body_init);

        nvShape *box_shape = nvBoxShape_new(0.7, 1.3, nvVector2_zero);
        nvRigidBody_add_shape(box, box_shape);

        nvSpace_add_rigidbody(example->space, box);
    }


    // Dense balls
    body_init.material.density = 50.0;
    for (size_t i = 0; i < 3; i++) {
        body_init.position = NV_VECTOR2(64.0 - 15.0 + (nv_float)i * 14.0, 45.0 - 20.0);

        nvRigidBody *ball = nvRigidBody_new(body_init);

        nvShape *ball_shape = nvCircleShape_new(nvVector2_zero, 1.0);
        nvRigidBody_add_shape(ball, ball_shape);

        nvSpace_add_rigidbody(example->space, ball);
    }
}

void Density_update(ExampleContext *example) {}