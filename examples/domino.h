/*

  This file is a part of the Nova Physics Engine
  project and distributed under the MIT license.

  Copyright © Kadir Aksoy
  https://github.com/kadir014/nova-physics

*/

#include "example.h"


void DominoExample_setup(Example *example) {
    nvSpace *space = example->space;

    // Create platforms
    nvBody *platform0 = nvBody_new(
        nvBodyType_STATIC,
        nvRectShape_new(80.0, 2.0),
        NV_VEC2(64.0, 18.0 + 5.0),
        0.0,
        nvMaterial_BASIC
    );

    nvSpace_add(space, platform0);

    nvBody *platform1 = nvBody_new(
        nvBodyType_STATIC,
        nvRectShape_new(80.0, 2.0),
        NV_VEC2(64.0, 36.0 + 5.0),
        0.0,
        nvMaterial_BASIC
    );

    nvSpace_add(space, platform1);

    nvBody *platform2 = nvBody_new(
        nvBodyType_STATIC,
        nvRectShape_new(80.0, 2.0),
        NV_VEC2(64.0, 54.0 + 5.0),
        0.0,
        nvMaterial_BASIC
    );

    nvSpace_add(space, platform2);

    // Create dominos
    for (int y = 0; y < 3; y++) {
        for (int x = 0; x < 18; x++) {
            nvBody *domino = nvBody_new(
                nvBodyType_DYNAMIC,
                nvRectShape_new(1.0, 7.0),
                NV_VEC2(64.0 - 40.0 + 0.5 + x * 4.65, 18.0 * (y + 1) - 1.0 - 3.5 + 5.0),
                0.0,
                nvMaterial_BASIC
            );

            nvSpace_add(space, domino);

            // Push the first domino block
            if (x == 0 && y == 0) {
                nvBody_apply_force_at(domino, NV_VEC2(900.0, 0.0), NV_VEC2(0.0, -3.0));
            }
        }
    }

    // Link end dominos

    nvConstraint *hinge_joint_0 = nvHingeJoint_new(
        NULL, (nvBody *)space->bodies->data[(18 * 1 - 1) + 4],
        NV_VEC2(64.0 + 40.0 - 0.5, 18.0 + 6.5)
    );

    nvSpace_add_constraint(space, hinge_joint_0);

    nvConstraint *hinge_joint_1 = nvHingeJoint_new(
        NULL, (nvBody *)space->bodies->data[18 + 4],
        NV_VEC2(64.0 - 40.0 + 0.5, 36.0 + 6.5)
    );

    nvSpace_add_constraint(space, hinge_joint_1);
}