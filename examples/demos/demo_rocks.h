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
    ground_init.position = NV_VECTOR2(64.0, 72.0 - 2.5);
    ground_init.material = (nvMaterial){.density=1.0, .restitution=0.1, .friction=0.5};
    ground = nvRigidBody_new(ground_init);

    nvShape *ground_shape = nvBoxShape_new(102.0, 5.0, nvVector2_zero);
    nvRigidBody_add_shape(ground, ground_shape);

    nvSpace_add_rigidbody(example->space, ground);


    size_t num_points = 20;
    #ifdef NV_COMPILER_MSVC
        nvVector2 *points = NV_MALLOC(sizeof(nvVector2) * num_points);
    #else
        nvVector2 points[num_points];
    #endif


    for (size_t i = 0; i < 50; i++) {
        // Fill convex hull points randomly
        for (size_t j = 0; j < num_points; j++) {
            points[j] = NV_VECTOR2(frand(-2.0, 2.0), frand(-2.0, 2.0));
        }

        nvRigidBody *rock;
        nvRigidBodyInitializer rock_init = nvRigidBodyInitializer_default;
        rock_init.type = nvRigidBodyType_DYNAMIC;
        rock_init.position = NV_VECTOR2(frand(64.0 - 25.0, 64.0 + 25.0), frand(10.0, 50.0));
        rock_init.material = (nvMaterial){.density=1.0, .restitution=0.05, .friction=0.7};
        rock = nvRigidBody_new(rock_init);

        nvShape *shape = nvConvexHullShape_new(points, num_points, nvVector2_zero, true);
        nvRigidBody_add_shape(rock, shape);

        nvSpace_add_rigidbody(example->space, rock);
    }


    #ifdef NV_COMPILER_MSVC
        NV_FREE(points);
    #endif
}

void Rocks_update(ExampleContext *example) {}