/*

  This file is a part of the Nova Physics Engine
  project and distributed under the MIT license.

  Copyright © Kadir Aksoy
  https://github.com/kadir014/nova-physics

*/

#include "../common.h"


#define RAYCAST_RESULTS 2048

void Raycast_setup(ExampleContext *example) {
    for (size_t x = 0; x < 100; x++) {
        for (size_t y = 0; y < 100; y++) {

            nvRigidBodyInitializer body_init = nvRigidBodyInitializer_default;
            body_init.type = nvRigidBodyType_STATIC;
            body_init.position = NV_VECTOR2(
                (nv_float)x * 2.0f,
                (nv_float)y * 2.0f
            );
            body_init.angle = frand(0, NV_PI);
            nvRigidBody *body = nvRigidBody_new(body_init);

            if (!u32rand(0, 10)) {
                nvShape *shape = nvCircleShape_new(nvVector2_zero, 1.0);
                nvRigidBody_add_shape(body, shape);
            }
            else {
                nvShape *shape = nvNGonShape_new(u32rand(3, 6), 1.0, nvVector2_zero);
                nvRigidBody_add_shape(body, shape);
            }

            nvSpace_add_rigidbody(example->space, body);
        }
    }
}

void Raycast_update(ExampleContext *example) {
    nvRayCastResult results[RAYCAST_RESULTS];
    size_t num_results;
    nvVector2 direction = NV_VECTOR2(1.0, 0.0);
    size_t n = 100;

    nvVector2 origin = NV_VECTOR2(100.0, 100.0);

    for (size_t i = 0; i < n; i++) {
        nvSpace_cast_ray(
            example->space,
            origin,
            nvVector2_add(origin, nvVector2_mul(direction, 200.0)),
            results,
            &num_results,
            RAYCAST_RESULTS
        );

        direction = nvVector2_rotate(direction, NV_PI / (nv_float)n);
    }
}