/*

  This file is a part of the Nova Physics Engine
  project and distributed under the MIT license.

  Copyright © Kadir Aksoy
  https://github.com/kadir014/nova-physics

*/

#include "example_base.h"


void setup(Example *example) {
    int n = 5; // Amount of balls
    nv_float radius = 4.5; // Radius of balls
    nv_float width = radius * 2.0 * n; // Size of the cradle
    nv_float length = 20.0; // Length of the cradle links

    nv_Material ball_material = (nv_Material){
        .density = 1.5,
        .restitution = 1.0,
        .friction = 0.0
    };

    for (size_t i = 0; i < 5; i++) {
        nv_Body *holder = nv_Rect_new(
            nv_BodyType_STATIC,
            NV_VEC2(example->width / 20.0 - width / 2.0 + i * radius * 2.0, 16.0),
            0.0,
            nv_Material_WOOD,
            3.5, 2.2
        );

        nv_Space_add(example->space, holder);

        nv_Body *ball = nv_Circle_new(
            nv_BodyType_DYNAMIC,
            NV_VEC2(
                example->width / 20.0 - width / 2.0 + i * radius * 2.0,
                16.0 + length + 1.1 + radius
            ),
            0.0,
            ball_material,
            radius
        );

        nv_Space_add(example->space, ball);

        nv_Constraint *dist_joint = nv_DistanceJoint_new(
            holder, ball,
            nv_Vector2_zero, nv_Vector2_zero,
            length
        );

        nv_Space_add_constraint(example->space, dist_joint);
    }
}


int main(int argc, char *argv[]) {
    // Create example
    Example *example = Example_new(
        1280, 720,
        "Nova Physics — Newton's Cradle Example",
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