/*

  This file is a part of the Nova Physics Engine
  project and distributed under the MIT license.

  Copyright © Kadir Aksoy
  https://github.com/kadir014/nova-physics

*/

#include "example_base.h"


void setup(Example *example) {
    nv_Material ground_mat = {
        .density = 3.0,
        .restitution = 1.0,
        .friction = 0.0
    };

    // Create ground 
    nv_Body *ground = nv_Rect_new(
        nv_BodyType_STATIC,
        (nv_Vector2){64.0, 62.5},
        0.0,
        ground_mat,
        185.0, 5.0
    );

    nv_Space_add(example->space, ground);

    for (size_t i = 0; i < 5; i++) {
        nv_Material material = {
            .density = 1.0,
            .restitution = (double)i / (5.0 / 6.0) / 5.0,
            .friction = 0.0
        };

        nv_Body *ball = nv_Circle_new(
            nv_BodyType_DYNAMIC,
            NV_VEC2(45.0 + (i * (8.0 + 1.0)), 20.0),
            0.0,
            material,
            4.0
        );

        nv_Space_add(example->space, ball);
    }
}


int main(int argc, char *argv[]) {
    // Create example
    Example *example = Example_new(
        1280, 720,
        "Nova Physics — Varying Restitution Example",
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