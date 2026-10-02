/*

  This file is a part of the Nova Physics Engine
  project and distributed under the MIT license.

  Copyright © Kadir Aksoy
  https://github.com/kadir014/nova-physics

*/

#include "../common.h"


nvRigidBody *softbody_frame;

void SoftBody_setup(ExampleContext *example) {
    nvRigidBodyInitializer ground_init = nvRigidBodyInitializer_default;
    ground_init.type = nvRigidBodyType_DYNAMIC;
    ground_init.position = NV_VECTOR2(64.0f, 72.0f);
    nvRigidBody *ground = nvRigidBody_new(ground_init);

    nv_float bar_width = 1.0f;

    nvShape *ground_shape = nvBoxShape_new(51.0f, bar_width, nvVector2_zero);
    nvRigidBody_add_shape(ground, ground_shape);
    nvShape *wall0_shape = nvBoxShape_new(bar_width, 51.0f, NV_VECTOR2(-25.0f, -25.0f));
    nvRigidBody_add_shape(ground, wall0_shape);
    nvShape *wall1_shape = nvBoxShape_new(bar_width, 51.0f, NV_VECTOR2(25.0f, -25.0f));
    nvRigidBody_add_shape(ground, wall1_shape);
    nvShape *ceiling_shape = nvBoxShape_new(51.0f, bar_width, NV_VECTOR2(0.0f, -50.0f));
    nvRigidBody_add_shape(ground, ceiling_shape);

    nvSpace_add_rigidbody(example->space, ground);

    softbody_frame = ground;

    
    nvHingeConstraintInitializer cons_init = nvHingeConstraintInitializer_default;
    cons_init.a = NULL;
    cons_init.b = ground;
    cons_init.anchor = NV_VECTOR2(64.0f, 72.0f - 25.0f);
    nvConstraint *hinge_cons0 = nvHingeConstraint_new(cons_init);
    nvSpace_add_constraint(example->space, hinge_cons0);


    nv_float size = 5.5f;
    for (size_t y = 0; y < 7; y++) {
        for (size_t x = 0; x < 6; x++) {
            nvVector2 pos = NV_VECTOR2(64.0 - (size * 2.5f) + x * size + (y % 2) * size/2.0f, 67.0f - y * size);
            create_circle_softbody(example, pos, 12, 2.5f, 0.6f);
        }
    }
}

void SoftBody_update(ExampleContext *example) {}