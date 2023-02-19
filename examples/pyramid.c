/*

  This file is a part of the Nova Physics Engine
  project and distributed under the MIT license.

  Copyright © Kadir Aksoy
  https://github.com/kadir014/nova-physics

*/

#include "example_base.h"


int main(int argc, char *argv[]) {
    // Create example
    Example *example = Example_new(
        1280, 720,
        "Nova Physics — Pyramid Example",
        165.0,
        1.0/60.0
    );

    example->substeps = 3;


    // Create ground 
    nv_Body *ground = nv_Rect_new(
        nv_BodyType_STATIC,
        (nv_Vector2){64.0, 62.5},
        0.0,
        nv_Material_WOOD,
        185.0, 5.0
    );

    nv_Space_add(example->space, ground);


    // Create bricks of the pyramid
    double size = 2.0;
    for (size_t y = 0; y < 15; y++) {
        for (size_t x = 0; x < 15-y; x++) {
            nv_Body *rect = nv_Rect_new(
                nv_BodyType_DYNAMIC,
                (nv_Vector2){34.0+x*size+y*(size/2), 62.5-2.5-size/2.0 - y*size},
                0.0,
                nv_Material_WOOD,
                size, size
            );

            rect->material.static_friction = 1.3;
            rect->material.dynamic_friction = 0.8;

            nv_Space_add(example->space, rect);
        }
    }


    // Run the example
    Example_run(example, false);

    // Free space allocated by example
    Example_free(example);

    return 0;
}