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
        NV_VEC2(40.0, 60.0),
        0.0,
        nv_Material_WOOD,
        30, 5.0
    );

    nv_Space_add(example->space, ground);

    // Create chain parts

    nv_float height = 15;
    nv_float width = 20;
    nv_float radius = 0.5; // Radius of the chain parts

    for (size_t y = 0; y < height; y++) {
        for (size_t x = 0; x < width; x++) {
            
            // Change the starting point every two rows
            size_t xx = y % 2 == 0 ? width - x - 1 : x;

            nv_Body *chain_part = nv_Circle_new(
                nv_BodyType_DYNAMIC,
                NV_VEC2(
                    30 + xx * radius * 2.0,
                    60 - 2.5 - height * radius * 2.0 + radius + y * radius * 2.0
                ),
                0.0,
                (nv_Material){2.0, 0.0, 0.2},
                radius
            );

            nv_Space_add(example->space, chain_part);
        }
    }

    // Link chain parts

    for (size_t i = 2; i < width * height + 1; i++) {
        nv_Constraint *link = nv_DistanceJoint_new(
            (nv_Body *)example->space->bodies->data[i],
            (nv_Body *)example->space->bodies->data[i + 1],
            nv_Vector2_zero,
            nv_Vector2_zero,
            radius * 2.0
        );

        nv_Space_add_constraint(example->space, link);
    }
}


int main(int argc, char *argv[]) {
    // Create example
    Example *example = Example_new(
        1280, 720,
        "Nova Physics  -  Mould Effect Example",
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