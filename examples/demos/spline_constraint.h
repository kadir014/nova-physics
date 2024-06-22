/*

  This file is a part of the Nova Physics Engine
  project and distributed under the MIT license.

  Copyright © Kadir Aksoy
  https://github.com/kadir014/nova-physics

*/

#include "../common.h"


void SplineConstraint_setup(ExampleContext *example) {
    {
        nvRigidBodyInitializer body_init = nvRigidBodyInitializer_default;
        body_init.type = nvRigidBodyType_DYNAMIC;
        body_init.position = NV_VECTOR2(30.0, 15.0);
        nvRigidBody *body = nvRigidBody_new(body_init);

        nvShape *body_shape = nvBoxShape_new(2.0, 2.0, nvVector2_zero);
        nvRigidBody_add_shape(body, body_shape);

        nvSpace_add_rigidbody(example->space, body);

        nvSplineConstraintInitializer cons_init = nvSplineConstraintInitializer_default;
        cons_init.body = body;
        cons_init.anchor = NV_VECTOR2(30.0, 15.0);
        nvConstraint *spline_cons = nvSplineConstraint_new(cons_init);

        nvVector2 points[8] = {
            NV_VECTOR2(20.0, 10.0),
            NV_VECTOR2(25.0, 20.0),
            NV_VECTOR2(30.0, 15.0),
            NV_VECTOR2(35.0, 20.0),
            NV_VECTOR2(40.0, 10.0),
            NV_VECTOR2(45.0, 15.0),
            NV_VECTOR2(50.0, 10.0),
            NV_VECTOR2(55.0, 20.0)
        };
        nvSplineConstraint_set_control_points(spline_cons, points, 8);

        nvSpace_add_constraint(example->space, spline_cons);
    }
}

void SplineConstraint_update(ExampleContext *example) {}