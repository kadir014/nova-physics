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
        (nv_Vector2){64.0, 62.5},
        0.0,
        nv_Material_CONCRETE,
        185.0, 5.0
    );

    nv_Space_add(example->space, ground);

    nv_Material basic_material = {
        .density = 2.0,
        .restitution = 1.0,
        .static_friction = nv_Material_WOOD.static_friction,
        .dynamic_friction = nv_Material_WOOD.dynamic_friction
    };

    nv_Body *box1 = nv_Rect_new(
        nv_BodyType_DYNAMIC,
        NV_VEC2(20.0, 20.0),
        0.0,
        basic_material,
        5.0, 5.0
    );

    nv_Space_add(example->space, box1);

    //nv_Body_apply_force(box1, NV_VEC2(0.0, 5.0 * 10e3 * example->sliders[2]->value));
}


int main(int argc, char *argv[]) {
    // Create example
    Example *example = Example_new(
        1280, 720,
        "Nova Physics — Stacking Example",
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