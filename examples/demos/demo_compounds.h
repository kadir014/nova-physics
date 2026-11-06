/*

  This file is a part of the Nova Physics Engine
  project and distributed under the MIT license.

  Copyright © Kadir Aksoy
  https://github.com/kadir014/nova-physics

*/

#include "../common.h"


void add_shuriken(
    nvRigidBody *body,
    size_t n,
    nv_float inner_radius,
    nv_float outer_radius
) {
    if (n <= 1) return;

    nv_float theta = NV_TAU / (nv_float)n;
    nvVector2 inner_arm = NV_VECTOR2(inner_radius, 0.0f);
    nvVector2 outer_arm = NV_VECTOR2(inner_radius + outer_radius, 0.0f);

    for (size_t i = 0; i < n; i++) {
        nvVector2 left = nvVector2_rotate(inner_arm, -theta * 0.5f);
        nvVector2 right = nvVector2_rotate(inner_arm, theta * 0.5f);

        nvVector2 points[4] = {
            nvVector2_zero,
            left,
            outer_arm,
            right
        };

        nvShape *part = nvConvexHullShape_new(points, 4, nvVector2_zero, false);
        nvRigidBody_add_shape(body, part);

        inner_arm = nvVector2_rotate(inner_arm, theta);
        outer_arm = nvVector2_rotate(outer_arm, theta);
    }

    nvShape *center = nvCircleShape_new(nvVector2_zero, inner_radius * 1.35);
    nvRigidBody_add_shape(body, center);
}

void Compounds_setup(ExampleContext *example) {
    nvRigidBodyInitializer ground_init = nvRigidBodyInitializer_default;
    ground_init.position = NV_VECTOR2(0.0f, 0.0f);
    nvRigidBody *ground = nvRigidBody_new(ground_init);

    for (size_t i = 0; i < 75 + 1; i++) {
        nvVector2 pos = NV_VECTOR2((nv_float)i, 100.0f);
        nvRigidBody_add_shape(ground, nvBoxShape_new(1.0f, 1.0f, pos));
    }
    for (size_t i = 0; i < 100; i++) {
        nvVector2 pos = NV_VECTOR2(0.0f, (nv_float)i);
        nvRigidBody_add_shape(ground, nvBoxShape_new(1.0f, 1.0f, pos));
    }
    for (size_t i = 0; i < 100; i++) {
        nvVector2 pos = NV_VECTOR2(75.0f, (nv_float)i);
        nvRigidBody_add_shape(ground, nvBoxShape_new(1.0f, 1.0f, pos));
    }

    nvSpace_add_rigidbody(example->space, ground);


    nv_float start_y = 0.0;

    for (size_t y = 0; y < 50; y++) {
        for (size_t x = 0; x < 36; x++) {

            nvRigidBodyInitializer body_init = nvRigidBodyInitializer_default;
            body_init.type = nvRigidBodyType_DYNAMIC;
            body_init.position = NV_VECTOR2(
                2.0f + (nv_float)x * 2.02f,
                100.0f - (2.0f + (nv_float)y * 2.1f)
            );
            body_init.material = (nvMaterial){.density=1.0f, .restitution=0.0f, .friction=0.3f};
            nvRigidBody *body = nvRigidBody_new(body_init);

            add_shuriken(body, 3, 0.35f, 1.0f);

            nvSpace_add_rigidbody(example->space, body);
        }
    }
}

void Compounds_update(ExampleContext *example) {}