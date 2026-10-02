/*

  This file is a part of the Nova Physics Engine
  project and distributed under the MIT license.

  Copyright © Kadir Aksoy
  https://github.com/kadir014/nova-physics

*/

#include "../common.h"


void DistanceConstraint_setup(ExampleContext *example) {
    {
        // High mass ratio double pendulum
        {
            nvRigidBodyInitializer body_init = nvRigidBodyInitializer_default;
            body_init.type = nvRigidBodyType_DYNAMIC;
            body_init.position = NV_VECTOR2(64.0f, 15.0f);
            nvRigidBody *body0 = nvRigidBody_new(body_init);

            nvShape *body0_shape = nvCircleShape_new(nvVector2_zero, 0.5f);
            nvRigidBody_add_shape(body0, body0_shape);

            nvSpace_add_rigidbody(example->space, body0);


            body_init.position = NV_VECTOR2(64.0f, 20.0f);
            body_init.material.density = 2.0f;
            nvRigidBody *body1 = nvRigidBody_new(body_init);

            nvShape *body1_shape = nvCircleShape_new(nvVector2_zero, 2.3f);
            nvRigidBody_add_shape(body1, body1_shape);

            nvSpace_add_rigidbody(example->space, body1);


            nvDistanceConstraintInitializer cons_init = nvDistanceConstraintInitializer_default;
            cons_init.a = NULL;
            cons_init.b = body0;
            cons_init.length = 5.0f;
            cons_init.anchor_a = NV_VECTOR2(64.0f, 10.0f);
            nvConstraint *dist0_cons = nvDistanceConstraint_new(cons_init);
            nvSpace_add_constraint(example->space, dist0_cons);

            cons_init.a = body0;
            cons_init.b = body1;
            cons_init.anchor_a = nvVector2_zero;
            nvConstraint *dist1_cons = nvDistanceConstraint_new(cons_init);
            nvSpace_add_constraint(example->space, dist1_cons);
        }

        // High mass ratio spring double pendulum
        {
            nvRigidBodyInitializer body_init = nvRigidBodyInitializer_default;
            body_init.type = nvRigidBodyType_DYNAMIC;
            body_init.position = NV_VECTOR2(74.0f, 15.0f);
            nvRigidBody *body0 = nvRigidBody_new(body_init);

            nvShape *body0_shape = nvCircleShape_new(nvVector2_zero, 0.5f);
            nvRigidBody_add_shape(body0, body0_shape);

            nvSpace_add_rigidbody(example->space, body0);


            body_init.position = NV_VECTOR2(74.0f, 20.0f);
            body_init.material.density = 2.0f;
            nvRigidBody *body1 = nvRigidBody_new(body_init);

            nvShape *body1_shape = nvCircleShape_new(nvVector2_zero, 2.3f);
            nvRigidBody_add_shape(body1, body1_shape);

            nvSpace_add_rigidbody(example->space, body1);


            nvDistanceConstraintInitializer cons_init = nvDistanceConstraintInitializer_default;
            cons_init.a = NULL;
            cons_init.b = body0;
            cons_init.length = 5.0f;
            cons_init.anchor_a = NV_VECTOR2(74.0f, 10.0f);
            cons_init.spring = true;
            cons_init.hertz = 1.2f;
            cons_init.damping = 0.1f;
            nvConstraint *dist0_cons = nvDistanceConstraint_new(cons_init);
            nvSpace_add_constraint(example->space, dist0_cons);

            cons_init.a = body0;
            cons_init.b = body1;
            cons_init.anchor_a = nvVector2_zero;
            nvConstraint *dist1_cons = nvDistanceConstraint_new(cons_init);
            nvSpace_add_constraint(example->space, dist1_cons);
        }

        // Double pendulum
        {
            nvRigidBodyInitializer body_init = nvRigidBodyInitializer_default;
            body_init.type = nvRigidBodyType_DYNAMIC;
            body_init.position = NV_VECTOR2(64.0f, 40.0f);
            nvRigidBody *body0 = nvRigidBody_new(body_init);

            nvShape *body0_shape = nvCircleShape_new(nvVector2_zero, 0.5f);
            nvRigidBody_add_shape(body0, body0_shape);

            nvSpace_add_rigidbody(example->space, body0);


            body_init.position = NV_VECTOR2(64.0f, 45.0f);
            nvRigidBody *body1 = nvRigidBody_new(body_init);

            nvShape *body1_shape = nvCircleShape_new(nvVector2_zero, 0.5f);
            nvRigidBody_add_shape(body1, body1_shape);

            nvSpace_add_rigidbody(example->space, body1);


            nvDistanceConstraintInitializer cons_init = nvDistanceConstraintInitializer_default;
            cons_init.a = NULL;
            cons_init.b = body0;
            cons_init.length = 5.0f;
            cons_init.anchor_a = NV_VECTOR2(64.0f, 35.0f);
            nvConstraint *dist0_cons = nvDistanceConstraint_new(cons_init);
            nvSpace_add_constraint(example->space, dist0_cons);

            cons_init.a = body0;
            cons_init.b = body1;
            cons_init.anchor_a = nvVector2_zero;
            nvConstraint *dist1_cons = nvDistanceConstraint_new(cons_init);
            nvSpace_add_constraint(example->space, dist1_cons);
        }

        // Spring double pendulum
        {
            nvRigidBodyInitializer body_init = nvRigidBodyInitializer_default;
            body_init.type = nvRigidBodyType_DYNAMIC;
            body_init.position = NV_VECTOR2(74.0f, 40.0f);
            nvRigidBody *body0 = nvRigidBody_new(body_init);

            nvShape *body0_shape = nvCircleShape_new(nvVector2_zero, 0.5f);
            nvRigidBody_add_shape(body0, body0_shape);

            nvSpace_add_rigidbody(example->space, body0);


            body_init.position = NV_VECTOR2(74.0f, 45.0f);
            nvRigidBody *body1 = nvRigidBody_new(body_init);

            nvShape *body1_shape = nvCircleShape_new(nvVector2_zero, 0.5f);
            nvRigidBody_add_shape(body1, body1_shape);

            nvSpace_add_rigidbody(example->space, body1);


            nvDistanceConstraintInitializer cons_init = nvDistanceConstraintInitializer_default;
            cons_init.a = NULL;
            cons_init.b = body0;
            cons_init.length = 5.0f;
            cons_init.anchor_a = NV_VECTOR2(74.0f, 35.0f);
            cons_init.spring = true;
            cons_init.hertz = 1.2f;
            cons_init.damping = 0.1f;
            nvConstraint *dist0_cons = nvDistanceConstraint_new(cons_init);
            nvSpace_add_constraint(example->space, dist0_cons);

            cons_init.a = body0;
            cons_init.b = body1;
            cons_init.anchor_a = nvVector2_zero;
            nvConstraint *dist1_cons = nvDistanceConstraint_new(cons_init);
            nvSpace_add_constraint(example->space, dist1_cons);
        }

        // Spring parameters
        {
            nvDistanceConstraintInitializer cons_init = nvDistanceConstraintInitializer_default;
            cons_init.a = NULL;
            cons_init.length = 5.0f;
            cons_init.spring = true;

            // Spring frequency 0.25 -> 5.0

            for (size_t x = 0; x < 10; x++) {
                cons_init.hertz = 0.25f + (nv_float)x * 0.475f;
                cons_init.damping = 0.1f;

                nvRigidBodyInitializer body_init = nvRigidBodyInitializer_default;
                body_init.type = nvRigidBodyType_DYNAMIC;
                body_init.position = NV_VECTOR2(90.0f + (nv_float)x * 6.0f, 12.0f);
                nvRigidBody *body = nvRigidBody_new(body_init);
                nvShape *body_shape = nvRectShape_new(1.0f, 1.0f, nvVector2_zero);
                nvRigidBody_add_shape(body, body_shape);
                nvSpace_add_rigidbody(example->space, body);

                cons_init.b = body;
                cons_init.anchor_a = NV_VECTOR2(90.0f + (nv_float)x * 6.0f, 10.0f);

                nvSpace_add_constraint(example->space, nvDistanceConstraint_new(cons_init));
            }
        }
    }
}

void DistanceConstraint_update(ExampleContext *example) {}