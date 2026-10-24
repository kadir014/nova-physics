/*

  This file is a part of the Nova Physics Engine
  project and distributed under the MIT license.

  Copyright © Kadir Aksoy
  https://github.com/kadir014/nova-physics

*/

#include "../common.h"


void QueryAABB_setup(ExampleContext *example) {
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

void QueryAABB_update(ExampleContext *example) {
    // Clear colored user data
    {
        nvRigidBody *body = NULL;
        size_t body_i = 0;
        while (nvSpace_iter_bodies(example->space, &body, &body_i)) {
            body->user_data = NULL;
        }
    }

    // Size of the query AABB
    nv_float width = 60.0f;
    nv_float height = 30.0f;

    nvAABB aabb = {
        .min_x = example->before_zoom.x - width * 0.5f,
        .min_y = example->before_zoom.y - height * 0.5f,
        .max_x = example->before_zoom.x + width * 0.5f,
        .max_y = example->before_zoom.y + height * 0.5f
    };

    nvRefArray *overlaps = nvSpace_query_aabb(
        example->space, aabb, nvCollisionFilter_default, false
    );

    for (size_t i = 0; i < overlaps->size; i++) {
        nvRigidBody *body = overlaps->data[i];

        // Just make it non-null with a valid pointer.
        body->user_data = example->space;
    }

    nvRefArray_free(overlaps);

    draw_aabb(example, aabb, (FColor){0.0, 1.0, 0.0, 1.0});
}