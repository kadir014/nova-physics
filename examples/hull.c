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
        128.0, 5.0
    );

    nv_Space_add(example->space, ground);

    for (size_t i = 0; i < 20; i++) {
        nv_Array *points = nv_Array_new();
        for (size_t i = 0; i < 15; i++) {
            nv_Array_add(points, NV_VEC2_NEW(frand(-7.0, 7.0), frand(-7.0, 7.0)));
        }

        nv_Body *rock = nv_Body_new(
            nv_BodyType_DYNAMIC,
            nv_ShapeFactory_ConvexHull(points),
            NV_VEC2(frand(50.0, 78.0), frand(17.0, 35.0)),
            0.0,
            nv_Material_CONCRETE
        );

        nv_Space_add(example->space, rock);

        nv_Array_free_each(points, free);
        nv_Array_free(points);
    }
}


int main(int argc, char *argv[]) {
    // Create example
    Example *example = Example_new(
        1280, 720,
        "Nova Physics  -  Hull Example",
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