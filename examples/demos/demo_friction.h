/*

  This file is a part of the Nova Physics Engine
  project and distributed under the MIT license.

  Copyright © Kadir Aksoy
  https://github.com/kadir014/nova-physics

*/

#include "../common.h"


void Friction_add_ramp(nvSpace *space, nv_float y, nv_float friction) {
    nvRigidBodyInitializer ramp_init = nvRigidBodyInitializer_default;
    ramp_init.position = NV_VECTOR2(64.0f, y);
    ramp_init.angle = 0.35f;
    ramp_init.material = (nvMaterial){.density=1.0f, .restitution=0.0f, .friction=0.1f};
    nvRigidBody *ramp = nvRigidBody_new(ramp_init);

    nvShape *ramp_shape = nvBoxShape_new(100.0f, 0.2f, nvVector2_zero);
    nvRigidBody_add_shape(ramp, ramp_shape);

    nvSpace_add_rigidbody(space, ramp);

    nvRigidBodyInitializer box_init = nvRigidBodyInitializer_default;
    box_init.type = nvRigidBodyType_DYNAMIC;
    box_init.position = NV_VECTOR2(19.0f, y - 18.0f);
    //box_init.angle = 0.35;
    box_init.material = (nvMaterial){.density=1.0f, .restitution=0.0f, .friction=friction};
    nvRigidBody *box = nvRigidBody_new(box_init);

    nvShape *box_shape = nvBoxShape_new(1.0f, 1.0f, nvVector2_zero);
    nvRigidBody_add_shape(box, box_shape);

    nvSpace_add_rigidbody(space, box);
}


void Friction_setup(ExampleContext *example) {
    // Friction constant [0.0, 2.0]
    for (nv_float i = 0.0f; i < 10.0f; i += 1.0f) {
        Friction_add_ramp(example->space, i * 5.0f, i * 0.2f);
    }
}

void Friction_update(ExampleContext *example) {}