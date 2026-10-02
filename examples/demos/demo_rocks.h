/*

  This file is a part of the Nova Physics Engine
  project and distributed under the MIT license.

  Copyright © Kadir Aksoy
  https://github.com/kadir014/nova-physics

*/

#include "../common.h"


void Rocks_setup(ExampleContext *example) {
    nvRigidBody *ground;
    nvRigidBodyInitializer ground_init = nvRigidBodyInitializer_default;
    ground_init.position = NV_VECTOR2(64.0f, 72.0f - 2.5f);
    ground_init.material = (nvMaterial){.density=1.0f, .restitution=0.1f, .friction=0.5f};
    ground = nvRigidBody_new(ground_init);

    nvShape *ground_shape = nvBoxShape_new(300.0f, 5.0f, nvVector2_zero);
    nvRigidBody_add_shape(ground, ground_shape);

    nvSpace_add_rigidbody(example->space, ground);

    size_t num_points = NV_POLYGON_MAX_VERTICES;
    #ifdef NV_COMPILER_MSVC
        nvVector2 *points = NV_MALLOC(sizeof(nvVector2) * num_points);
    #else
        nvVector2 points[num_points];
    #endif

    for (size_t y = 0; y < 10; y++) {
        for (size_t x = 0; x < 120; x++) {
            // Fill convex hull points randomly
            for (size_t j = 0; j < num_points; j++) {
                points[j] = NV_VECTOR2(frand(0.0f, frand(4.0f, 7.0f)), frand(0.0f, 4.0f));
            }

            nvRigidBody *rock;
            nvRigidBodyInitializer rock_init = nvRigidBodyInitializer_default;
            rock_init.type = nvRigidBodyType_DYNAMIC;
            rock_init.position = NV_VECTOR2(
                (nv_float)x * 7.3f,
                (nv_float)y * 4.3f
            );
            rock_init.material = (nvMaterial){.density=1.0f, .restitution=0.05f, .friction=0.7f};
            rock = nvRigidBody_new(rock_init);

            nvShape *shape = nvConvexHullShape_new(
                points, num_points, NV_VECTOR2(5.0f, -5.0f), true
            );
            nvRigidBody_add_shape(rock, shape);

            nvSpace_add_rigidbody(example->space, rock);
        }
    }

    #ifdef NV_COMPILER_MSVC
        NV_FREE(points);
    #endif
}

void Rocks_update(ExampleContext *example) {}