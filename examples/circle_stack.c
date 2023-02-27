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

    // Some basic material with no restitution (inelastic)
    nv_Material basic_material = {
        .density = 1.0,
        .restitution = 1.0,
        .friction = nv_Material_WOOD.friction,
    };

    // Create stacking circles

    double size = 1.0; // Radius of the circles
    double s2 = size * 2.0;

    for (size_t y = 0; y < 30; y++) {
        for (size_t x = 0; x < 1; x ++) {

            nv_Body *circle = nv_Circle_new(
                nv_BodyType_DYNAMIC,
                NV_VEC2(
                    example->width / 20.0 + s2,
                    62.5 - 2.5 - size - y * (s2 + 0.2)
                ),
                0.0,
                basic_material,
                size
            );

            nv_Space_add(example->space, circle);
        }
    }
}


int main(int argc, char *argv[]) {
    // Create example
    Example *example = Example_new(
        1280, 720,
        "Nova Physics — Cricle Stack Example",
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