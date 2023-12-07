/*

  This file is a part of the Nova Physics Engine
  project and distributed under the MIT license.

  Copyright © Kadir Aksoy
  https://github.com/kadir014/nova-physics

*/

#include "example_base.h"


void setup(Example *example) {
    // Create platforms
    nvBody *platform0 = nv_Rect_new(
        nv_BodyType_STATIC,
        NV_VEC2(64.0, 18.0 + 5.0),
        0.0,
        nvMaterial_BASIC,
        80.0, 2.0
    );

    nvSpace_add(example->space, platform0);

    nvBody *platform1 = nv_Rect_new(
        nv_BodyType_STATIC,
        NV_VEC2(64.0, 36.0 + 5.0),
        0.0,
        nvMaterial_BASIC,
        80.0, 2.0
    );

    nvSpace_add(example->space, platform1);

    nvBody *platform2 = nv_Rect_new(
        nv_BodyType_STATIC,
        NV_VEC2(64.0, 54.0 + 5.0),
        0.0,
        nvMaterial_BASIC,
        80.0, 2.0
    );

    nvSpace_add(example->space, platform2);

    // Create dominos
    for (int y = 0; y < 3; y++) {
        for (int x = 0; x < 18; x++) {
            nvBody *domino = nv_Rect_new(
                nv_BodyType_DYNAMIC,
                NV_VEC2(64.0 - 40.0 + 0.5 + x * 4.65, 18.0 * (y + 1) - 1.0 - 3.5 + 5.0),
                0.0,
                nvMaterial_BASIC,
                1.0, 7.0
            );

            nvSpace_add(example->space, domino);

            // Push the first domino block
            if (x == 0 && y == 0) {
                nvBody_apply_force_at(domino, NV_VEC2(900.0, 0.0), NV_VEC2(0.0, -3.0));
            }
        }
    }

    // Link end dominos

    nvConstraint *dist_joint_0 = nvDistanceJoint_new(
        NULL, (nvBody *)example->space->bodies->data[(18 * 1 - 1) + 4],
        NV_VEC2(64.0 + 40.0 - 0.5, 18.0 + 6.5), NV_VEC2(0.0, 3.5 - 0.5),
        3.0
    );

    nvSpace_add_constraint(example->space, dist_joint_0);

    nvConstraint *dist_joint_1 = nvDistanceJoint_new(
        NULL, (nvBody *)example->space->bodies->data[18 + 4],
        NV_VEC2(64.0 - 40.0 + 0.5, 36.0 + 6.5), NV_VEC2(0.0, 3.5 - 0.5),
        3.0
    );

    nvSpace_add_constraint(example->space, dist_joint_1);
}


int main(int argc, char *argv[]) {
    // Create example
    Example *example = Example_new(
        1280, 720,
        "Nova Physics — Domino Example",
        165.0,
        1.0/60.0,
        ExampleTheme_DARK
    );

    // Set callbacks
    example->setup_callback = setup;

    // Run the example
    Example_run(example);

    // Free the space allocated by example
    Example_free(example);

    return 0;
}