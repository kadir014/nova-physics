/*

  This file is a part of the Nova Physics Engine
  project and distributed under the MIT license.

  Copyright © Kadir Aksoy
  https://github.com/kadir014/nova-physics

*/

#include "example_base.h"


void setup(Example *example) {
    nvSpace_set_SHG(example->space, example->space->shg->bounds, 1.4, 1.4);

    // Create borders of the pool

    nvBody *wall_bottom = nv_Rect_new(
        nv_BodyType_STATIC,
        NV_VEC2(64.0, 62.5),
        0.0,
        nvMaterial_CONCRETE,
        60.0, 5.0
    );

    nvSpace_add(example->space, wall_bottom);

    nvBody *wall_left = nv_Rect_new(
        nv_BodyType_STATIC,
        NV_VEC2(24.0, 47.5),
        -NV_PI / 5.0,
        nvMaterial_CONCRETE,
        5.0, 40.0
    );

    nvSpace_add(example->space, wall_left);

    nvBody *wall_right = nv_Rect_new(
        nv_BodyType_STATIC,
        NV_VEC2(104.0, 47.5),
        NV_PI / 5.0,
        nvMaterial_CONCRETE,
        5.0, 40.0
    );

    nvSpace_add(example->space, wall_right);


    // Add balls

    double radius = 0.7;

    nvMaterial ball_material = {
        .density = 1.0,
        .restitution = 0.0,
        .friction = 0.0
    };

    for (size_t y = 0; y < 18; y++) {
        for (size_t x = 0; x < 30; x++) {

            nvBody *ball = nv_Circle_new(
                nv_BodyType_DYNAMIC,
                NV_VEC2(33.0 + x * (radius * 2.0), 25.8 + y * (radius * 2.0)),
                0.0,
                ball_material,
                radius
            );

            nvSpace_add(example->space, ball);
        }
    }


    // Add ship

    // Vertices array will get free'd along with bodies
    // once nvSpace_free is called (or Example_free in this case)
    nvArray *ship_vertices = nvArray_new();
    nvArray_add(ship_vertices, NV_VEC2_NEW(-5.0, -2.0));
    nvArray_add(ship_vertices, NV_VEC2_NEW(5.0, -2.0));
    nvArray_add(ship_vertices, NV_VEC2_NEW(3.0, 2.0));
    nvArray_add(ship_vertices, NV_VEC2_NEW(-3.0, 2.0));

    nvBody *ship = nv_Polygon_new(
        nv_BodyType_DYNAMIC,
        NV_VEC2(44.0, 15.0),
        0.0,
        nvMaterial_GLASS,
        ship_vertices
    );

    nvSpace_add(example->space, ship);
}


int main(int argc, char *argv[]) {
    // Create example
    Example *example = Example_new(
        1280, 720,
        "Nova Physics  -  Pool Example",
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