/*

  This file is a part of the Nova Physics Engine
  project and distributed under the MIT license.

  Copyright © Kadir Aksoy
  https://github.com/kadir014/nova-physics

*/

#include "example_base.h"


void setup(Example *example) {
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
            nv_BodyType_STATIC,
            NV_VEC2(
                example->width / 20.0 - width / 2.0 + i * radius * 2.0001 + radius,
                16.0
            ),
            0.0,
            nvMaterial_WOOD,
            3.5, 2.2
        );

        nvSpace_add(example->space, holder);

        nvBody *ball;
        if (i == 0) {
            ball = nv_Circle_new(
                nv_BodyType_DYNAMIC,
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
                nv_BodyType_DYNAMIC,
                NV_VEC2(
                    example->width / 20.0 - width / 2.0 + i * radius * 2.0001 + radius,
                    16.0 + length + 1.1 + radius
                ),
                0.0,
                ball_material,
                radius
            );
        }

        nvSpace_add(example->space, ball);

        nvConstraint *dist_joint = nvDistanceJoint_new(
            holder, ball,
            nvVector2_zero, nvVector2_zero,
            length
        );

        nvSpace_add_constraint(example->space, dist_joint);
    }
}


int main(int argc, char *argv[]) {
    // Create example
    Example *example = Example_new(
        1280, 720,
        "Nova Physics  -  Newton's Cradle Example",
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