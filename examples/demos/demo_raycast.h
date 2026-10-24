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
                nvShape *shape = nvCircleShape_new(nvVector2_zero, 1.0f);
                nvRigidBody_add_shape(body, shape);
            }
            else {
                nvShape *shape = nvNGonShape_new(u32rand(3, 6), 1.0f, nvVector2_zero);
                nvRigidBody_add_shape(body, shape);
            }

            nvSpace_add_rigidbody(example->space, body);
        }
    }
}

void Raycast_update(ExampleContext *example) {
    nvVector2 direction = NV_VECTOR2(1.0f, 0.0f);
    size_t n = 100;

    nvVector2 origin = NV_VECTOR2(100.0f, 100.0f);

    for (size_t i = 0; i < n; i++) {
        nvVector2 from = origin;
        nvVector2 to = nvVector2_add(origin, nvVector2_mul(direction, 200.0f));
        nvArray *results = nvSpace_cast_ray(
            example->space,
            from,
            to,
            RAYCAST_RESULTS,
            nvCollisionFilter_default
        );

        for (size_t j = 0; j < results->size; j++) {
            nvRayCastResult result = NV_ARRAY_AT(results, j, nvRayCastResult);

            nvVector2 p0 = result.position;
            nvVector2 p1 = nvVector2_add(p0, result.normal);
            p0 = world_to_screen(example, p0);
            p0 = normalize_coords(example, p0);
            p1 = world_to_screen(example, p1);
            p1 = normalize_coords(example, p1);
            ADD_LINE_(p0.x, p0.y, 0.0, 0.0, 0.0, 0.0);
            ADD_LINE_(p0.x, p0.y, 1.0, 0.0, 0.0, 1.0);
            ADD_LINE_(p1.x, p1.y, 1.0, 0.0, 0.0, 1.0);
            ADD_LINE_(p1.x, p1.y, 0.0, 0.0, 0.0, 0.0);
        }

        // It's caller responsibility to cleanup cast & query arrays
        nvArray_free(results);

        direction = nvVector2_rotate(direction, NV_TAU / (nv_float)n);
    }
}