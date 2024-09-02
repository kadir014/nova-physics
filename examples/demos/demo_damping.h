/*

  This file is a part of the Nova Physics Engine
  project and distributed under the MIT license.

  Copyright © Kadir Aksoy
  https://github.com/kadir014/nova-physics

*/

#include "../common.h"


void Damping_add_body0(nvSpace *space, nv_float y, nv_float linear_damping_scale) {
    nvRigidBodyInitializer box_init = nvRigidBodyInitializer_default;
    box_init.type = nvRigidBodyType_DYNAMIC;
    box_init.position = NV_VECTOR2(40.0, y + 5.0);
    nvRigidBody *box = nvRigidBody_new(box_init);

    nvShape *box_shape = nvBoxShape_new(1.0, 1.0, nvVector2_zero);
    nvRigidBody_add_shape(box, box_shape);

    nvRigidBody_set_linear_damping_scale(box, linear_damping_scale);
    nvRigidBody_apply_force(box, NV_VECTOR2(1000.0, 0.0));

    nvSpace_add_rigidbody(space, box);
}

void Damping_add_body1(nvSpace *space, nv_float y, nv_float angular_damping_scale) {
    nvRigidBodyInitializer box_init = nvRigidBodyInitializer_default;
    box_init.type = nvRigidBodyType_DYNAMIC;
    box_init.position = NV_VECTOR2(40.0, y + 35.0);
    nvRigidBody *box = nvRigidBody_new(box_init);

    nvShape *box_shape = nvBoxShape_new(2.0, 1.0, nvVector2_zero);
    nvRigidBody_add_shape(box, box_shape);

    nvRigidBody_set_angular_damping_scale(box, angular_damping_scale);
    nvRigidBody_apply_torque(box, 1000.0);

    nvSpace_add_rigidbody(space, box);
}


void Damping_setup(ExampleContext *example) {
    nvSpace_set_gravity(example->space, nvVector2_zero);

    // Damping scale 1.0 (100%) means the damping value specified in nvSpaceSettings
    // is not affected by this body's scale factor

    // linear damping 0% -> 50,000%
    // body with 0% damping scale will keep moving whereas others will start to slow down
    for (nv_float i = 0.0; i < 10.0; i += 1.0) {
        Damping_add_body0(example->space, i * 2.0, i * 50.0);
    }

    // angular damping 0% -> 50,000%
    // body with 0% damping scale will keep rotating whereas others will start to slow down
    for (nv_float i = 0.0; i < 10.0; i += 1.0) {
        Damping_add_body1(example->space, i * 4.0, i * 50.0);
    }
}

void Damping_update(ExampleContext *example) {}