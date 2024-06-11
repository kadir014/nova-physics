/*

  This file is a part of the Nova Physics Engine
  project and distributed under the MIT license.

  Copyright © Kadir Aksoy
  https://github.com/kadir014/nova-physics

*/

#include "../common.h"


void Constraints_setup(ExampleContext *example) {
    nvRigidBody *ground;
    nvRigidBodyInitializer ground_init = nvRigidBodyInitializer_default;
    ground_init.position = NV_VECTOR2(64.0, 72.0 - 2.5);
    ground = nvRigidBody_new(ground_init);

    nvShape *ground_shape = nvBoxShape_new(128.0, 5.0, nvVector2_zero);
    nvRigidBody_add_shape(ground, ground_shape);

    nvSpace_add_body(example->space, ground);

    
    nvRigidBodyInitializer body_init = nvRigidBodyInitializer_default;
    body_init.type = nvRigidBodyType_STATIC;
    body_init.position = NV_VECTOR2(0.0, 10.0);
    nvRigidBody *body0 = nvRigidBody_new(body_init);

    nvShape *body0_shape = nvBoxShape_new(10.0, 5.0, nvVector2_zero);
    nvRigidBody_add_shape(body0, body0_shape);

    nvSpace_add_body(example->space, body0);


    body_init.type = nvRigidBodyType_DYNAMIC;
    body_init.position = NV_VECTOR2(10.0, 10.0);
    nvRigidBody *body1 = nvRigidBody_new(body_init);

    nvShape *body1_shape = nvBoxShape_new(10.0, 5.0, nvVector2_zero);
    nvRigidBody_add_shape(body1, body1_shape);

    nvSpace_add_body(example->space, body1);


    nvHingeConstraintInitializer cons_init = nvHingeConstraintInitializer_default;
    cons_init.a = body1;
    cons_init.b = body0;
    cons_init.anchor = NV_VECTOR2(5.0, 10.0);
    cons_init.enable_limits = true;
    nvConstraint *hing_cons = nvHingeConstraint_new(cons_init);
    nvSpace_add_constraint(example->space, hing_cons);
}

void Constraints_update(ExampleContext *example) {}