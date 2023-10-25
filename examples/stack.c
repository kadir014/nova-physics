/*

  This file is a part of the Nova Physics Engine
  project and distributed under the MIT license.

  Copyright © Kadir Aksoy
  https://github.com/kadir014/nova-physics

*/

#include "example_base.h"


void update(Example *example) {
    if (example->keys[SDL_SCANCODE_UP]) {
        nv_Body *body = example->space->bodies->data[2];
        nv_Body_apply_impulse(body, NV_VEC2(0.0, -40.0), NV_VEC2(0.0, 0.0));
    }
}


void setup(Example *example) {
    //Create ground & walls
    nv_Body *ground = nv_Rect_new(
        nv_BodyType_STATIC,
        NV_VEC2(64.0, 70.0),
        0.0,
        (nv_Material){1.0, 0.1, 0.65},
        128.0, 5.0
    );

    nv_Space_add(example->space, ground);

    nv_float offsets[20] = {
        -0.3, 0.1, 0.0, 0.2, -0.15,
        0.05, -0.09, 0.04, -0.1, 0.3,
        0.2, 0.24, -0.017, 0.17, 0.03,
        0.3, 0.0, -0.06, 0.25, 0.08
    };

    // Create stacked boxes

    int cols = 12;
    int rows = 20;
    nv_float size = 2.0;
    nv_float s2 = size / 2.0;
    nv_float gap = 0.0;

    for (size_t y = 0; y < rows; y++) {
        for (size_t x = 0; x < cols; x ++) {
            nv_float offset = offsets[(x + y) % 20] * 0.0;

            nv_Body *box = nv_Rect_new(
                nv_BodyType_DYNAMIC,
                NV_VEC2(
                    example->width / 20.0 - 30.0 - ((nv_float)cols * size) / 2.0 + s2 + size * x + offset + (x * 4.5),
                    70 - 2.5 - s2 - y * (size + gap)
                ),
                0.0,
                (nv_Material){1.0, 0.0, 0.5},
                size, size
            );

            nv_Space_add(example->space, box);
        }
    }

    nv_Space_set_SHG(example->space, example->space->shg->bounds, 2.0, 2.0);
}


int main(int argc, char *argv[]) {
    // Create example
    Example *example = Example_new(
        1280, 720,
        "Nova Physics  -  Stacking Example",
        165.0,
        1.0/60.0,
        ExampleTheme_DARK
    );

    // Set callbacks
    example->setup_callback = setup;
    example->update_callback = update;

    // Run the example
    Example_run(example);

    // Free the space allocated by example
    Example_free(example);

    return 0;
}