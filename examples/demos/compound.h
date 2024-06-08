/*

  This file is a part of the Nova Physics Engine
  project and distributed under the MIT license.

  Copyright © Kadir Aksoy
  https://github.com/kadir014/nova-physics

*/

#include "../common.h"


void Compound_setup(ExampleContext *example) {
    nvRigidBody *ground;
    nvRigidBodyInitializer ground_init = nvRigidBodyInitializer_default;
    ground_init.position = NV_VECTOR2(64.0, 72.0 - 2.5);
    ground = nvRigidBody_new(ground_init);

    nvShape *ground_shape = nvBoxShape_new(128.0, 5.0, nvVector2_zero);
    nvRigidBody_add_shape(ground, ground_shape);

    nvSpace_add_body(example->space, ground);


    nvRigidBody *box;
    nvRigidBodyInitializer box_init = nvRigidBodyInitializer_default;
    box_init.type = nvRigidBodyType_DYNAMIC;
    box_init.position = NV_VECTOR2(64.0, 50.0);
    box_init.material = (nvMaterial){.density=1.0, .restitution=0.1, .friction=0.6};
    box = nvRigidBody_new(box_init);

    nvShape *box_shape = nvBoxShape_new(2.0, 2.0, nvVector2_zero);
    nvRigidBody_add_shape(box, box_shape);

    nvShape *box_shape2 = nvBoxShape_new(2.0, 2.0, NV_VECTOR2(10.0, 0.0));
    nvRigidBody_add_shape(box, box_shape2);

    nvShape *box_shape3 = nvBoxShape_new(2.0, 2.0, NV_VECTOR2(0.0, 10.0));
    nvRigidBody_add_shape(box, box_shape3);

    printf(
        "mass info: %f %f {%f, %f}\n",
        nvRigidBody_get_mass(box),
        nvRigidBody_get_inertia(box),
        box->com.x,
        box->com.y
    );

    nvSpace_add_body(example->space, box);
}

void Compound_update(ExampleContext *example) {}