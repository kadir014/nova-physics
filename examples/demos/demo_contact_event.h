/*

  This file is a part of the Nova Physics Engine
  project and distributed under the MIT license.

  Copyright © Kadir Aksoy
  https://github.com/kadir014/nova-physics

*/

#include "../common.h"


static ExampleContext *g_Example = NULL;

void ContactEvent_added_callback(nvSpace *space, nvContactEvent event, void *user_arg) {
    nvRigidBody *remover = (nvRigidBody *)user_arg;

    // No one touched the remover floor
    if (event.body_a->id != remover->id && event.body_b->id != remover->id)
        return;

    nvRigidBody *to_be_removed =
        event.body_a->id == remover->id ?
        event.body_b :
        event.body_a;

    /*
        It is user's responsibility to clean external references to bodies and
        its attachments after removal.

        In this case, I set the mouse constraint to NULL because in the example
        demos, the body might be attached to mouse during removal.
    */
    if (g_Example && g_Example->mouse_cons) {
        nvRigidBody *a = nvDistanceConstraint_get_body_a(g_Example->mouse_cons);
        nvRigidBody *b = nvDistanceConstraint_get_body_b(g_Example->mouse_cons);
        if (
            nvRigidBody_get_id(a) == nvRigidBody_get_id(to_be_removed) ||
            nvRigidBody_get_id(b) == nvRigidBody_get_id(to_be_removed)
        ) {
            g_Example->mouse_cons = NULL;
        }
    }

    // After all the external references are cleaned, remove the body
    // Notice the "true" parameter, it lets space to free all resources linked to the body
    nvSpace_remove_rigidbody(space, to_be_removed, true);
}

void ContactEvent_setup(ExampleContext *example) {
    g_Example = example;

    nvRigidBodyInitializer ground_init = nvRigidBodyInitializer_default;
    ground_init.position = NV_VECTOR2(64.0f, 72.0f - 2.5f);
    ground_init.material = (nvMaterial){.density=1.0f, .restitution=1.0f, .friction=0.5f};
    nvRigidBody *void_ground = nvRigidBody_new(ground_init);

    nvShape *ground_shape = nvBoxShape_new(30.0f, 2.0f, nvVector2_zero);
    nvRigidBody_add_shape(void_ground, ground_shape);

    nvSpace_add_rigidbody(example->space, void_ground);


    ground_init.position = NV_VECTOR2(30.0f, 30.0f);
    ground_init.angle = NV_PI / 4.0f + 0.3f;
    nvRigidBody *ramp0 = nvRigidBody_new(ground_init);

    nvShape *ramp0_shape = nvBoxShape_new(90.0, 2.0f, nvVector2_zero);
    nvRigidBody_add_shape(ramp0, ramp0_shape);

    nvSpace_add_rigidbody(example->space, ramp0);


    ground_init.position = NV_VECTOR2(64.0f + 34.0f, 30.0f);
    ground_init.angle = -NV_PI / 4.0f - 0.3f;
    nvRigidBody *ramp1 = nvRigidBody_new(ground_init);

    nvShape *ramp1_shape = nvBoxShape_new(90.0f, 2.0f, nvVector2_zero);
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
        body_init.position = NV_VECTOR2(frand(64.0f - 50.0f, 64.0f + 50.0f), -15.0f);
        body_init.material = (nvMaterial){.density=1.0f, .restitution=1.0f, .friction=0.5f};
        nvRigidBody *body = nvRigidBody_new(body_init);

        nvShape *body_shape = nvNGonShape_new(u32rand(3, 6), 1.0f, nvVector2_zero);
        nvRigidBody_add_shape(body, body_shape);

        nvSpace_add_rigidbody(example->space, body);
    }
    
    spawn_frame++;
}