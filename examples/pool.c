/*

  This file is a part of the Nova Physics Engine
  project and distributed under the MIT license.

  Copyright © Kadir Aksoy
  https://github.com/kadir014/nova-physics

*/

#include "example_base.h"


void setup(Example *example) {
    // Create borders of the pool

    nv_Body *wall_bottom = nv_Rect_new(
        nv_BodyType_STATIC,
        NV_VEC2(64.0, 62.5),
        0.0,
        nv_Material_CONCRETE,
        60.0, 5.0
    );

    nv_Space_add(example->space, wall_bottom);

    nv_Body *wall_left = nv_Rect_new(
        nv_BodyType_STATIC,
        NV_VEC2(24.0, 47.5),
        -NV_PI / 5.0,
        nv_Material_CONCRETE,
        5.0, 40.0
    );

    nv_Space_add(example->space, wall_left);

    nv_Body *wall_right = nv_Rect_new(
        nv_BodyType_STATIC,
        NV_VEC2(104.0, 47.5),
        NV_PI / 5.0,
        nv_Material_CONCRETE,
        5.0, 40.0
    );

    nv_Space_add(example->space, wall_right);


    // Add balls

    double radius = 0.7;

    nv_Material ball_material = {
        .density = 1.5,
        .restitution = 0.0,
        .friction = 0.0
    };

    for (size_t y = 0; y < 15; y++) {
        for (size_t x = 0; x < 30; x++) {

            nv_Body *ball = nv_Circle_new(
                nv_BodyType_DYNAMIC,
                NV_VEC2(33.0 + x * (radius * 2.0), 25.8 + y * (radius * 2.0)),
                0.0,
                ball_material,
                radius
            );

            nv_Space_add(example->space, ball);
        }
    }


    // Add ship

    // Vertices array will get free'd along with bodies
    // once nv_Space_free is called (or Example_free in this case)
    nv_Array *ship_vertices = nv_Array_new();
    nv_Array_add(ship_vertices, NV_VEC2_NEW(-5.0, -2.0));
    nv_Array_add(ship_vertices, NV_VEC2_NEW(5.0, -2.0));
    nv_Array_add(ship_vertices, NV_VEC2_NEW(3.0, 2.0));
    nv_Array_add(ship_vertices, NV_VEC2_NEW(-3.0, 2.0));

    nv_Body *ship = nv_Polygon_new(
        nv_BodyType_DYNAMIC,
        NV_VEC2(44.0, 15.0),
        0.0,
        nv_Material_GLASS,
        ship_vertices
    );

    nv_Space_add(example->space, ship);
}


int main(int argc, char *argv[]) {
    // Create example
    Example *example = Example_new(
        1280, 720,
        "Nova Physics — Pool Example",
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