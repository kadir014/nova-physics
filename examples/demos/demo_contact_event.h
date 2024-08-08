/*

  This file is a part of the Nova Physics Engine
  project and distributed under the MIT license.

  Copyright © Kadir Aksoy
  https://github.com/kadir014/nova-physics

*/

#include "../common.h"


void ContactEvent_added_callback(nvSpace *space, nvContactEvent event, void *user_arg) {
    nvRigidBody *void_ground = (nvRigidBody *)user_arg;

    if (event.body_a == void_ground) {
        nvSpace_remove_rigidbody(space, event.body_b);
    }
    else if (event.body_b == void_ground) {
        nvSpace_remove_rigidbody(space, event.body_a);
    }
}

void ContactEvent_setup(ExampleContext *example) {
    nvRigidBodyInitializer ground_init = nvRigidBodyInitializer_default;
    ground_init.position = NV_VECTOR2(64.0, 72.0 - 2.5);
    ground_init.material = (nvMaterial){.density=1.0, .restitution=1.0, .friction=0.5};
    nvRigidBody *void_ground = nvRigidBody_new(ground_init);

    nvShape *ground_shape = nvBoxShape_new(30.0, 2.0, nvVector2_zero);
    nvRigidBody_add_shape(void_ground, ground_shape);

    nvSpace_add_rigidbody(example->space, void_ground);


    ground_init.position = NV_VECTOR2(30.0, 30.0);
    ground_init.angle = NV_PI / 4.0 + 0.3;
    nvRigidBody *ramp0 = nvRigidBody_new(ground_init);

    nvShape *ramp0_shape = nvBoxShape_new(90.0, 2.0, nvVector2_zero);
    nvRigidBody_add_shape(ramp0, ramp0_shape);

    nvSpace_add_rigidbody(example->space, ramp0);


    ground_init.position = NV_VECTOR2(64.0 + 34.0, 30.0);
    ground_init.angle = -NV_PI / 4.0 - 0.3;
    nvRigidBody *ramp1 = nvRigidBody_new(ground_init);

    nvShape *ramp1_shape = nvBoxShape_new(90.0, 2.0, nvVector2_zero);
    nvRigidBody_add_shape(ramp1, ramp1_shape);

    nvSpace_add_rigidbody(example->space, ramp1);


    nvContactListener listener = {
        .on_contact_added = ContactEvent_added_callback,
        .on_contact_persisted = NULL,
        .on_contact_removed = NULL
    };

    nvSpace_set_contact_listener(example->space, listener, void_ground);
}

size_t spawn_frame = 0;

void ContactEvent_update(ExampleContext *example) {
    if (spawn_frame % 5 == 0) {
        nvRigidBodyInitializer body_init = nvRigidBodyInitializer_default;
        body_init.type = nvRigidBodyType_DYNAMIC;
        body_init.position = NV_VECTOR2(frand(64.0 - 50.0, 64.0 + 50.0), -15.0);
        body_init.material = (nvMaterial){.density=1.0, .restitution=1.0, .friction=0.5};
        nvRigidBody *body = nvRigidBody_new(body_init);

        nvShape *body_shape = nvNGonShape_new(u32rand(3, 6), 1.0, nvVector2_zero);
        nvRigidBody_add_shape(body, body_shape);

        nvSpace_add_rigidbody(example->space, body);
    }
    
    spawn_frame++;
}