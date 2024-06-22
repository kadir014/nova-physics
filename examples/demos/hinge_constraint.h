/*

  This file is a part of the Nova Physics Engine
  project and distributed under the MIT license.

  Copyright © Kadir Aksoy
  https://github.com/kadir014/nova-physics

*/

#include "../common.h"


void HingeConstraint_setup(ExampleContext *example) {

    // Three bricks showing angular limits
    {
        nvRigidBodyInitializer body_init = nvRigidBodyInitializer_default;
        body_init.type = nvRigidBodyType_STATIC;
        body_init.position = NV_VECTOR2(50.0, 15.0);
        nvRigidBody *body0 = nvRigidBody_new(body_init);

        nvShape *body0_shape = nvRectShape_new(4.0, 2.0, nvVector2_zero);
        nvRigidBody_add_shape(body0, body0_shape);

        nvSpace_add_rigidbody(example->space, body0);


        body_init.type = nvRigidBodyType_DYNAMIC;
        body_init.position = NV_VECTOR2(50.0 + 4.0, 15.0);
        nvRigidBody *body1 = nvRigidBody_new(body_init);

        nvShape *body1_shape = nvRectShape_new(4.0, 2.0, nvVector2_zero);
        nvRigidBody_add_shape(body1, body1_shape);

        nvSpace_add_rigidbody(example->space, body1);


        body_init.position = NV_VECTOR2(50.0 - 4.0, 15.0);
        nvRigidBody *body2 = nvRigidBody_new(body_init);

        nvShape *body2_shape = nvRectShape_new(4.0, 2.0, nvVector2_zero);
        nvRigidBody_add_shape(body2, body2_shape);

        nvSpace_add_rigidbody(example->space, body2);


        nvHingeConstraintInitializer cons_init = nvHingeConstraintInitializer_default;
        cons_init.a = body0;
        cons_init.b = body1;
        cons_init.anchor = NV_VECTOR2(50.0 + 2.0, 15.0);
        cons_init.enable_limits = true;
        cons_init.lower_limit = -NV_PI * 0.5;
        cons_init.upper_limit = NV_PI * 0.5;
        nvConstraint *hinge_cons0 = nvHingeConstraint_new(cons_init);
        nvSpace_add_constraint(example->space, hinge_cons0);

        cons_init.a = body0;
        cons_init.b = body2;
        cons_init.anchor = NV_VECTOR2(50.0 - 2.0, 15.0);
        cons_init.lower_limit = 0.0;
        cons_init.upper_limit = NV_PI * 0.25;
        nvConstraint *hinge_cons1 = nvHingeConstraint_new(cons_init);
        nvSpace_add_constraint(example->space, hinge_cons1);

        // Ignore collision of bodies connected with hinge constraint
        hinge_cons0->ignore_collision = true;
        hinge_cons1->ignore_collision = true;
    }


    // Create a bridge with restricted angle
    {
        nvRigidBodyInitializer body_init = nvRigidBodyInitializer_default;
        body_init.type = nvRigidBodyType_DYNAMIC;

        nvRigidBody *prev;
        for (size_t i = 0; i < 7; i++) {
            body_init.position = NV_VECTOR2(50.0 + (nv_float)i * 4, 30.0);
            nvRigidBody *body = nvRigidBody_new(body_init);
            nvShape *body_shape = nvRectShape_new(4.0, 2.0, nvVector2_zero);
            nvRigidBody_add_shape(body, body_shape);
            nvSpace_add_rigidbody(example->space, body);

            // If this is the first segment, connect to world
            // Else, connect to previous segment
            nvRigidBody *a;
            nvRigidBody *b;
            if (i == 0) {
                a = NULL;
                b = body;
            }
            else {
                a = prev;
                b = body;
            }

            nvHingeConstraintInitializer cons_init = nvHingeConstraintInitializer_default;
            cons_init.a = a;
            cons_init.b = b;
            cons_init.anchor = NV_VECTOR2(50.0 + (nv_float)i * 4 - 2.0, 30.0);
            cons_init.enable_limits = true;
            cons_init.lower_limit = 0.0;
            cons_init.upper_limit = 0.0;
            nvConstraint *hinge_cons = nvHingeConstraint_new(cons_init);
            nvSpace_add_constraint(example->space, hinge_cons);

            hinge_cons->ignore_collision = true;

            prev = body;
        }
    }
}

void HingeConstraint_update(ExampleContext *example) {}