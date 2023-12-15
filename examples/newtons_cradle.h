/*

  This file is a part of the Nova Physics Engine
  project and distributed under the MIT license.

  Copyright © Kadir Aksoy
  https://github.com/kadir014/nova-physics

*/

#include "example.h"


void NewtonsCradleExample_setup(Example *example) {
    nvSpace *space = example->space;
    
    int n = 7; // Amount of balls
    nv_float radius = 4.2; // Radius of balls
    nv_float width = (radius + 0.01) * 2.0 * n; // Size of the cradle
    nv_float length = 30.0; // Length of the cradle links

    nvMaterial ball_material = (nvMaterial){
        .density = 1.5,
        .restitution = 1.0,
        .friction = 0.0
    };

    for (size_t i = 0; i < n; i++) {
        nvBody *holder = nv_Rect_new(
            nvBodyType_STATIC,
            NV_VEC2(
                example->width / 20.0 - width / 2.0 + i * radius * 2.0001 + radius,
                16.0
            ),
            0.0,
            nvMaterial_WOOD,
            3.5, 2.2
        );

        nvSpace_add(space, holder);

        nvBody *ball;
        if (i == 0) {
            ball = nv_Circle_new(
                nvBodyType_DYNAMIC,
                NV_VEC2(
                    example->width / 20.0 - width / 2.0 + i * radius * 2.0001 + radius - length/2.0,
                    16.0 + length + 1.1 + radius - length/2.0
                ),
                0.0,
                ball_material,
                radius
            );
        }
        else {
            ball = nv_Circle_new(
                nvBodyType_DYNAMIC,
                NV_VEC2(
                    example->width / 20.0 - width / 2.0 + i * radius * 2.0001 + radius,
                    16.0 + length + 1.1 + radius
                ),
                0.0,
                ball_material,
                radius
            );
        }

        nvSpace_add(space, ball);

        nvConstraint *dist_joint = nvDistanceJoint_new(
            holder, ball,
            nvVector2_zero, nvVector2_zero,
            length
        );

        nvSpace_add_constraint(space, dist_joint);
    }
}