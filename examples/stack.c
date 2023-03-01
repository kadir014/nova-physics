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

    // Some basic material with no restitution (inelastic)
    nv_Material basic_material = {
        .density = 2.0,
        .restitution = 0.0,
        .friction = nv_Material_WOOD.friction,
    };

    // Create stacked boxes

    int cols = 6; // Columns of the stack
    int rows = 17; // Rows of the stack
    double size = 3.25; // Size of the boxes
    double s2 = size / 2.0;
    double random_offset = 0.75; // Random horizontal offset magnitude

    for (size_t y = 0; y < rows; y++) {
    
        double offset = 0;
        if (random_offset > 0.0)
            offset = frand(-random_offset / 2.0, random_offset / 2.0);

        for (size_t x = 0; x < cols; x ++) {

            nv_Body *box = nv_Rect_new(
                nv_BodyType_DYNAMIC,
                NV_VEC2(
                    example->width / 20.0 - ((double)cols * size) / 2.0 + s2 + size * x + offset,
                    62.5 - 2.5 - s2 - y * size
                ),
                0.0,
                basic_material,
                size, size
            );

            nv_Space_add(example->space, box);
        }
    }
}


void update(Example *example) {
    if (example->counter < 5) return;
    else example->counter = 0;

    if (example->keys[SDL_SCANCODE_SPACE]) {
        nv_Vector2 mouse = NV_VEC2(example->mouse.px, example->mouse.py);
        nv_Vector2 delta = nv_Vector2_sub(NV_VEC2(64.0, mouse.y), mouse);
        nv_Vector2 dir = nv_Vector2_normalize(delta);
        nv_Vector2 pos = mouse;

        nv_Body *ball = nv_Circle_new(
            nv_BodyType_DYNAMIC,
            pos,
            0.0,
            (nv_Material){1.5, 0.3, 0.5},
            1.5
        );

        nv_Space_add(example->space, ball);

        nv_Vector2 force = nv_Vector2_muls(dir, 15.0 * 10e3);

        nv_Body_apply_force(ball, force);
    }
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
    example->update_callback = update;

    // Run the example
    Example_run(example);

    // Free the space allocated by example
    Example_free(example);

    return 0;
}