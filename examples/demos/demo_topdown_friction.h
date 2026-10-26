/*

  This file is a part of the Nova Physics Engine
  project and distributed under the MIT license.

  Copyright © Kadir Aksoy
  https://github.com/kadir014/nova-physics

*/

#include "../common.h"


void TopdownFriction_setup(ExampleContext *example) {
    nvSpace_set_gravity(example->space, nvVector2_zero);

    nvRigidBodyInitializer ground_init = nvRigidBodyInitializer_default;
    ground_init.type = nvRigidBodyType_STATIC;
    ground_init.position = NV_VECTOR2(64.0f, 72.0f);
    ground_init.material = (nvMaterial){.density=1.0f, .restitution=1.0f, .friction=0.0f};
    nvRigidBody *ground = nvRigidBody_new(ground_init);

    nv_float bar_width = 5.0f;

    nvShape *ground_shape = nvBoxShape_new(100.0f, bar_width, nvVector2_zero);
    nvRigidBody_add_shape(ground, ground_shape);
    nvShape *wall0_shape = nvBoxShape_new(bar_width, 100.0f, NV_VECTOR2(-50.0f, -50.0f));
    nvRigidBody_add_shape(ground, wall0_shape);
    nvShape *wall1_shape = nvBoxShape_new(bar_width, 100.0f, NV_VECTOR2(50.0f, -50.0f));
    nvRigidBody_add_shape(ground, wall1_shape);
    nvShape *ceiling_shape = nvBoxShape_new(100.0f, bar_width, NV_VECTOR2(0.0f, -100.0f));
    nvRigidBody_add_shape(ground, ceiling_shape);

    nvSpace_add_rigidbody(example->space, ground);

    nv_float size = 4.0f;
    for (size_t y = 0; y < 10; y++) {
        for (size_t x = 0; x < 10; x++) {
            nvVector2 pos = NV_VECTOR2(
                64.0 - (size * 2.5f) + x * size + (y % 2) * size * 0.5f,
                40.0f - y * size
            );

            nvRigidBodyInitializer box_init = nvRigidBodyInitializer_default;
            box_init.type = nvRigidBodyType_DYNAMIC;
            box_init.position = pos;
            box_init.material = (nvMaterial){.density=1.0f, .restitution=1.0f, .friction=0.0f};
            nvRigidBody *box = nvRigidBody_new(box_init);

            nvShape *box_shape = nvBoxShape_new(size * 0.5f, size * 0.5f, nvVector2_zero);
            nvRigidBody_add_shape(box, box_shape);

            nvSpace_add_rigidbody(example->space, box);

            nvVector2 force = nvVector2_rotate(
                NV_VECTOR2(0.0f, frand(10000.0f, 50000.0f)),
                frand(0.0f, NV_TAU)
            );
            nvRigidBody_apply_force(box, force);

            /*
                Increasing per-body damping scale will act as topdown friction.

                This is not as accurate as a friction constraint, but good enough
                for simple cases in games.
                
                As you increase the per-body damping scale, the topdown friction
                effect gets heavier proportionally.
            */
            nv_float damping_scale = frand(1.0f, 10000.0f);

            nvRigidBody_set_linear_damping_scale(box, damping_scale);
            nvRigidBody_set_angular_damping_scale(box, damping_scale * 1.5f);
        }
    }
}

void TopdownFriction_update(ExampleContext *example) {}