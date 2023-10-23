/*

  This file is a part of the Nova Physics Engine
  project and distributed under the MIT license.

  Copyright © Kadir Aksoy
  https://github.com/kadir014/nova-physics

*/

#include "example_base.h"


void setup(Example *example) {
    // Create ground & walls

    nv_Body *ground = nv_Rect_new(
        nv_BodyType_STATIC,
        NV_VEC2(64.0, 72.0 + 2.5),
        0.0,
        nv_Material_CONCRETE,
        128.0, 5.0
    );

    nv_Space_add(example->space, ground);

    nv_Body *ceiling = nv_Rect_new(
        nv_BodyType_STATIC,
        NV_VEC2(64.0, 0 - 2.5),
        0.0,
        nv_Material_CONCRETE,
        128.0, 5.0
    );

    nv_Space_add(example->space, ceiling);

    nv_Body *walll = nv_Rect_new(
        nv_BodyType_STATIC,
        NV_VEC2(0.0 - 2.55, 36.0),
        0.0,
        nv_Material_CONCRETE,
        5.0, 72.0
    );

    nv_Space_add(example->space, walll);

    nv_Body *wallr = nv_Rect_new(
        nv_BodyType_STATIC,
        NV_VEC2(128.0 + 2.5, 36.0),
        0.0,
        nv_Material_CONCRETE,
        5.0, 72.0
    );

    nv_Space_add(example->space, wallr);


    nv_Vector2 radius = {10.0 / 2.0, 0.0};

    nv_Array *vertices = nv_Array_new();
    for (size_t i = 0; i < 3; i++) {
        nv_Array_add(vertices, NV_VEC2_NEW(radius.x, radius.y));
        radius = nv_Vector2_rotate(radius, 2.0 * NV_PI / 3.0); // 2pi/3 = 120 degrees
    }


    nv_Body *big_ball = nv_Circle_new(
        nv_BodyType_DYNAMIC,
        NV_VEC2(94.0, 36.0),
        0.0,
        nv_Material_BASIC,
        5.0
    );

    nv_Space_add(example->space, big_ball);
}


void update(Example *example) {
    if (example->space->bodies->size > 2000) return;

    if (example->counter < 4) return;
    else example->counter = 0;

    nv_Material basic_material = {
        .density = 1.0,
        .restitution = 0.1,
        .friction = 0.2
    };

    nv_float n = 8;

    for (size_t x = 0; x < n; x++) {
        nv_Body *body;

        //int r =irand(0, 3);
        int r = x % 4;

        nv_float size = 1.5;

        // Circle
        if (r == 0) {
            body = nv_Circle_new(
                nv_BodyType_DYNAMIC,
                NV_VEC2(
                    64.0 - (n * size) / 2.0 + size / 2.0 + size * x,
                    10.0
                ),
                0.0,
                basic_material,
                size / 2.0 + 0.03
            );
        }
        // Box
        else if (r == 1) {
            body = nv_Rect_new(
                nv_BodyType_DYNAMIC,
                NV_VEC2(
                    64.0 - (n * size) / 2.0 + size / 2.0 + size* x,
                    10.0
                ),
                0.0,
                basic_material,
                size, size
            );
        }
        // Pentagon
        else if (r == 2) {
            nv_Array *vertices = nv_Array_new();

            nv_Vector2 radius = {size / 2.0 + 0.1, 0.0};

            for (size_t i = 0; i < 5; i++) {
                nv_Array_add(vertices, NV_VEC2_NEW(radius.x, radius.y));
                radius = nv_Vector2_rotate(radius, 2.0 * NV_PI / 5.0); // 2pi/5 = 72 degrees
            }

            body = nv_Polygon_new(
                nv_BodyType_DYNAMIC,
                NV_VEC2(
                    64.0 - (n * size) / 2.0 + size / 2.0 + size * x,
                    10.0
                ),
                0.0,
                basic_material,
                vertices
            );
        }
        // Triangle
        else if (r == 3) {
            nv_Vector2 radius = {size / 2.0 + 0.1, 0.0};
            nv_Array *vertices = nv_Array_new();
            for (size_t i = 0; i < 3; i++) {
                nv_Array_add(vertices, NV_VEC2_NEW(radius.x, radius.y));
                radius = nv_Vector2_rotate(radius, 2.0 * NV_PI / 3.0); // 2pi/3 = 120 degrees
            }

            body = nv_Polygon_new(
                nv_BodyType_DYNAMIC,
                NV_VEC2(
                    64.0 - (n * size) / 2.0 + size / 2.0 + size * x,
                    10.0
                ),
                0.0,
                basic_material,
                vertices
            );
        }

        // Have all bodies have the same mass and inertia
        nv_Body_set_mass(body, 3.5);

        nv_Space_add(example->space, body);

        // Apply downward force
        nv_float strength = 10.0 * 1e3 / 1.0;
        nv_Body_apply_force(body, NV_VEC2(0.0, strength));
    }
}


int main(int argc, char *argv[]) {
    // Create example
    Example *example = Example_new(
        1280, 720,
        "Nova Physics  -  Fountain Example",
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