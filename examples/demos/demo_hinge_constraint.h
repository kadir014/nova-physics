/*

  This file is a part of the Nova Physics Engine
  project and distributed under the MIT license.

  Copyright © Kadir Aksoy
  https://github.com/kadir014/nova-physics

*/

#include "../common.h"


void HingeConstraint_setup(ExampleContext *example) {
    /*
        3 bricks attached to world, showing different angular limits:
        First one is -90 to 180 degrees
        Second one is -90 to 90 degrees
        Third one is 0 to 45 degrees 
    */
    {
        nvRigidBodyInitializer body_init = nvRigidBodyInitializer_default;
        body_init.type = nvRigidBodyType_DYNAMIC;
        body_init.position = NV_VECTOR2(0.0f, 0.0f);
        nvRigidBody *body = nvRigidBody_new(body_init);

        nvShape *shape = nvRectShape_new(10.0f, 2.0f, nvVector2_zero);
        nvRigidBody_add_shape(body, shape);

        nvSpace_add_rigidbody(example->space, body);


        nvHingeConstraintInitializer cons_init = nvHingeConstraintInitializer_default;
        cons_init.a = NULL;
        cons_init.b = body;
        cons_init.anchor = NV_VECTOR2(-10.0f * 0.5, 0.0f);
        cons_init.enable_limits = true;
        cons_init.lower_limit = -NV_PI * 0.5f;
        cons_init.upper_limit = NV_PI;
        nvConstraint *hinge_cons = nvHingeConstraint_new(cons_init);
        nvSpace_add_constraint(example->space, hinge_cons);
    }
    {
        nvRigidBodyInitializer body_init = nvRigidBodyInitializer_default;
        body_init.type = nvRigidBodyType_DYNAMIC;
        body_init.position = NV_VECTOR2(15.0f, 0.0f);
        nvRigidBody *body = nvRigidBody_new(body_init);

        nvShape *shape = nvRectShape_new(10.0f, 2.0f, nvVector2_zero);
        nvRigidBody_add_shape(body, shape);

        nvSpace_add_rigidbody(example->space, body);


        nvHingeConstraintInitializer cons_init = nvHingeConstraintInitializer_default;
        cons_init.a = NULL;
        cons_init.b = body;
        cons_init.anchor = NV_VECTOR2(15.0f + -10.0f * 0.5, 0.0f);
        cons_init.enable_limits = true;
        cons_init.lower_limit = -NV_PI * 0.5f;
        cons_init.upper_limit = NV_PI * 0.5f;
        nvConstraint *hinge_cons = nvHingeConstraint_new(cons_init);
        nvSpace_add_constraint(example->space, hinge_cons);
    }
    {
        nvRigidBodyInitializer body_init = nvRigidBodyInitializer_default;
        body_init.type = nvRigidBodyType_DYNAMIC;
        body_init.position = NV_VECTOR2(30.0f, 0.0f);
        nvRigidBody *body = nvRigidBody_new(body_init);

        nvShape *shape = nvRectShape_new(10.0f, 2.0f, nvVector2_zero);
        nvRigidBody_add_shape(body, shape);

        nvSpace_add_rigidbody(example->space, body);


        nvHingeConstraintInitializer cons_init = nvHingeConstraintInitializer_default;
        cons_init.a = NULL;
        cons_init.b = body;
        cons_init.anchor = NV_VECTOR2(30.0f + -10.0f * 0.5, 0.0f);
        cons_init.enable_limits = true;
        cons_init.lower_limit = 0.0f;
        cons_init.upper_limit = NV_PI * 0.25f;
        nvConstraint *hinge_cons = nvHingeConstraint_new(cons_init);
        nvSpace_add_constraint(example->space, hinge_cons);
    }

    /*
        Two bricks joined together using a hinge constraint with angular limits,
        one is attached to world with a distance constraint.

        This shows the angular limits between two dynamic bodies.
    */
    {
        nvRigidBodyInitializer body_init = nvRigidBodyInitializer_default;
        body_init.type = nvRigidBodyType_DYNAMIC;
        body_init.position = NV_VECTOR2(0.0f, 20.0f);
        nvRigidBody *body0 = nvRigidBody_new(body_init);
        nvShape *shape0 = nvRectShape_new(10.0f, 2.0f, nvVector2_zero);
        nvRigidBody_add_shape(body0, shape0);
        nvSpace_add_rigidbody(example->space, body0);

        body_init.position = NV_VECTOR2(10.0f, 20.0f);
        nvRigidBody *body1 = nvRigidBody_new(body_init);
        nvShape *shape1 = nvRectShape_new(10.0f, 2.0f, nvVector2_zero);
        nvRigidBody_add_shape(body1, shape1);
        nvSpace_add_rigidbody(example->space, body1);


        nvHingeConstraintInitializer hinge_init = nvHingeConstraintInitializer_default;
        hinge_init.a = body0;
        hinge_init.b = body1;
        hinge_init.anchor = NV_VECTOR2(5.0f, 20.0f);
        hinge_init.enable_limits = true;
        hinge_init.lower_limit = -NV_PI * 0.5f;
        hinge_init.upper_limit = NV_PI * 0.5f;
        nvConstraint *hinge_cons = nvHingeConstraint_new(hinge_init);
        nvSpace_add_constraint(example->space, hinge_cons);
        nvConstraint_set_ignore_collision(hinge_cons, true);


        nvDistanceConstraintInitializer dist_init = nvDistanceConstraintInitializer_default;
        dist_init.a = NULL;
        dist_init.b = body0;
        dist_init.anchor_a = NV_VECTOR2(-5.0f, 20.0f);
        dist_init.anchor_b = NV_VECTOR2(-5.0f, 0.0f);
        dist_init.length = 3.5f;
        nvConstraint *dist_cons0 = nvDistanceConstraint_new(dist_init);
        nvSpace_add_constraint(example->space, dist_cons0);
    }

    /*
        A big box attached to world from one of its corners, without any angular limits.
    */
    {
        nvRigidBodyInitializer body_init = nvRigidBodyInitializer_default;
        body_init.type = nvRigidBodyType_DYNAMIC;
        body_init.position = NV_VECTOR2(30.0f, 30.0f);
        nvRigidBody *body = nvRigidBody_new(body_init);

        nvShape *shape = nvRectShape_new(10.0f, 10.0f, nvVector2_zero);
        nvRigidBody_add_shape(body, shape);

        nvSpace_add_rigidbody(example->space, body);


        nvHingeConstraintInitializer cons_init = nvHingeConstraintInitializer_default;
        cons_init.a = NULL;
        cons_init.b = body;
        cons_init.anchor = NV_VECTOR2(25.0f, 25.0f);
        cons_init.enable_limits = false;
        nvConstraint *hinge_cons = nvHingeConstraint_new(cons_init);
        nvSpace_add_constraint(example->space, hinge_cons);
    }
}

void HingeConstraint_update(ExampleContext *example) {}