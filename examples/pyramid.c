/*

  This file is a part of the Nova Physics Engine
  project and distributed under the MIT license.

  Copyright © Kadir Aksoy
  https://github.com/kadir014/nova-physics

*/

#include "example_base.h"


void setup(Example *example) {
    // Create ground
    nv_Body *ground = nv_Rect_new(
        nv_BodyType_STATIC,
        NV_VEC2(64.0, 62.5),
        0.0,
        nv_Material_CONCRETE,
        185.0, 5.0
    );

    nv_Space_add(example->space, ground);


    // Create bricks of the pyramid

    int floors = 32; // Height of the pyramid
    nv_float size = 1.3; // Brick size
    nv_float s2 = size / 2.0;

    for (size_t y = 0; y < floors; y++) {
        for (size_t x = 0; x < floors - y; x++) {

            nv_Body *brick = nv_Rect_new(
                nv_BodyType_DYNAMIC,
                NV_VEC2(
                    example->width / 20.0 - (floors * s2 - s2) + x * size + y * s2,
                    62.5 - 2.5 - s2 - y * (size + 0.1)
                ),
                0.0,
                nv_Material_BASIC,
                size, size
            );

            nv_Space_add(example->space, brick);
        }
    }

    nv_Space_set_SHG(example->space, example->space->shg->bounds, 1.5, 1.5);
}


int main(int argc, char *argv[]) {
    // Create example
    Example *example = Example_new(
        1280, 720,
        "Nova Physics  -  Pyramid Example",
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