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
        "Nova Physics — Pool Example",
        165.0,
        1.0/60.0
    );

    // Example settings
    example->substeps = 1;
    example->draw_contacts = false;
    example->draw_dirs = false;
    

    // Create borders of the pool

    nv_Body *ground = nv_Rect_new(
        nv_BodyType_STATIC,
        (nv_Vector2){64.0, 62.5},
        0.0,
        2.0,
        NV_COR_STEEL,
        60.0, 5.0
    );

    nv_Space_add(example->space, ground);

    nv_Body *wall_l = nv_Rect_new(
        nv_BodyType_STATIC,
        (nv_Vector2){24.0, 47.5},
        -NV_PI / 5.0,
        2.0,
        NV_COR_STEEL,
        5.0, 40.0
    );

    nv_Space_add(example->space, wall_l);

    nv_Body *wall_r = nv_Rect_new(
        nv_BodyType_STATIC,
        (nv_Vector2){104.0, 47.5},
        NV_PI / 5.0,
        2.0,
        NV_COR_STEEL,
        5.0, 40.0
    );

    nv_Space_add(example->space, wall_r);


    // Add balls

    double radius = 1.0;

    for (size_t y = 0; y < 15; y++) {
        for (size_t x = 0; x < 25; x++) {
            nv_Body *ball = nv_Circle_new(
                nv_BodyType_DYNAMIC,
                (nv_Vector2){33.0 + x*(radius*2.0), 25.8 + y*(radius*2.0)},
                0.0,
                1.0,
                0.3,
                radius
            );

            ball->static_friction = 0.15;
            ball->dynamic_friction = 0.07;

            nv_Space_add(example->space, ball);
        }
    }


    // Add ship

    // Vertices will get free'd along with bodies
    // once nv_Space_free is called (or Example_free in this case)
    nv_Vector2Array *ship_vertices = nv_Vector2Array_new();
    nv_Vector2Array_add(ship_vertices, (nv_Vector2){-5.0, -3.0});
    nv_Vector2Array_add(ship_vertices, (nv_Vector2){5.0, -3.0});
    nv_Vector2Array_add(ship_vertices, (nv_Vector2){3.0, 3.0});
    nv_Vector2Array_add(ship_vertices, (nv_Vector2){-3.0, 3.0});

    nv_Body *ship = nv_Polygon_new(
        nv_BodyType_DYNAMIC,
        (nv_Vector2){44.0, 15.0},
        0.0,
        2.0,
        0.5,
        ship_vertices
    );

    nv_Space_add(example->space, ship);


    // Run the example
    Example_run(example, false);

    // Free space allocated by example
    Example_free(example);

    return 0;
}