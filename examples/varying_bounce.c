/*

  This file is a part of the Nova Physics Engine
  project and distributed under the MIT license.

  Copyright © Kadir Aksoy
  https://github.com/kadir014/nova-physics

*/

#include "example_base.h"


void setup(Example *example) {
    nvMaterial ground_mat = {
        .density = 1.0,
        .restitution = 1.0,
        .friction = 0.0
    };

    // Create ground 
    nvBody *ground = nv_Rect_new(
        nvBodyType_STATIC,
        (nvVector2){64.0, 62.5},
        0.0,
        ground_mat,
        185.0, 5.0
    );

    nvSpace_add(example->space, ground);

    for (size_t i = 0; i < 5; i++) {

        nvMaterial material = {
            .density = 1.0,
            .restitution = (nv_float)i / 4.0,
            .friction = 0.0
        };

        printf("%u -> %f\n", i, (nv_float)i / 4.0);

        nvBody *ball = nv_Circle_new(
            nvBodyType_DYNAMIC,
            NV_VEC2(45.0 + (i * (8.0 + 1.0)), 20.0),
            0.0,
            material,
            4.0
        );

        nvSpace_add(example->space, ball);
    }
}


int main(int argc, char *argv[]) {
    // Create example
    Example *example = Example_new(
        1280, 720,
        "Nova Physics  -  Varying Restitution Example",
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