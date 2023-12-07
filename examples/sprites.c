/*

  This file is a part of the Nova Physics Engine
  project and distributed under the MIT license.

  Copyright © Kadir Aksoy
  https://github.com/kadir014/nova-physics

*/

#include "example_base.h"


void setup(Example *example) {
    // Create ground
    nvBody *ground = nv_Rect_new(
        nv_BodyType_STATIC,
        NV_VEC2(64.0, 62.5),
        0.0,
        nvMaterial_CONCRETE,
        185.0, 5.0
    );

    nvSpace_add(example->space, ground);


    nvBody *box = nv_Rect_new(
        nv_BodyType_DYNAMIC,
        NV_VEC2(64.0, 45.0),
        0.0,
        nvMaterial_STEEL,
        9.0, 9.0
    );

    nvSpace_add(example->space, box);

    box = nv_Rect_new(
        nv_BodyType_DYNAMIC,
        NV_VEC2(64.0, 35.0),
        0.0,
        nvMaterial_WOOD,
        9.0, 9.0
    );

    nvSpace_add(example->space, box);

    box = nv_Rect_new(
        nv_BodyType_DYNAMIC,
        NV_VEC2(64.0, 25.0),
        0.0,
        nvMaterial_ICE,
        9.0, 9.0
    );

    nvSpace_add(example->space, box);

    nvArray_add(example->sprites, NULL); // Ground has no texture
    nvArray_add(example->sprites, load_image(example->renderer, "assets/steel_crate.png"));
    nvArray_add(example->sprites, load_image(example->renderer, "assets/wooden_crate.png"));
    nvArray_add(example->sprites, load_image(example->renderer, "assets/ice.png"));
}


int main(int argc, char *argv[]) {
    // Create example
    Example *example = Example_new(
        1280, 720,
        "Nova Physics  -  Sprites Example",
        165.0,
        1.0/60.0,
        ExampleTheme_LIGHT
    );

    // Set callbacks
    example->setup_callback = setup;

    // Run the example
    Example_run(example);

    // Free the space allocated by example
    Example_free(example);

    return 0;
}