/*

  This file is a part of the Nova Physics Engine
  project and distributed under the MIT license.

  Copyright © Kadir Aksoy
  https://github.com/kadir014/Nova-Physics

*/

#include "example_base.h"


int main(int argc, char *argv[]) {
    // Create example
    Example *example = Example_new(
        1280, 720,
        "Nova Physics — Orbit Example",
        165.0,
        1.0/60.0
    );

    // Example settings
    example->space->gravity = nv_Vector2_zero;
    

    // Create planets & stars
    
    nv_Body *star = nv_Circle_new(
        nv_BodyType_DYNAMIC,
        (nv_Vector2){64.0, 36.0},
        0.0,
        15.0,
        1.0,
        5.0
    );
    
    nv_Space_add(example->space, star);

    nv_Body_set_is_attractor(star, true);


    nv_Body *planet1 = nv_Circle_new(
        nv_BodyType_DYNAMIC,
        (nv_Vector2){85.0, 28.0},
        0.0,
        2.0,
        1.0,
        1.5
    );

    nv_Space_add(example->space, planet1);

    nv_Body_apply_force(planet1, (nv_Vector2){0.0, 4.0e4});


    nv_Body *planet2 = nv_Circle_new(
        nv_BodyType_DYNAMIC,
        (nv_Vector2){30.0, 35.0},
        0.0,
        2.0,
        1.0,
        1.5
    );

    nv_Space_add(example->space, planet2);

    nv_Body_apply_force(planet2, (nv_Vector2){0.0, 4.0e4});


    nv_Body *planet3 = nv_Rect_new(
        nv_BodyType_DYNAMIC,
        (nv_Vector2){30.0, 55.0},
        0.0,
        2.0,
        1.0,
        3.1, 2.1
    );

    nv_Space_add(example->space, planet3);

    nv_Body_apply_force(planet3, (nv_Vector2){1.6e4, 2.0e4});


    // Run the example
    Example_run(example, false);

    // Free space allocated by example
    Example_free(example);

    return 0;
}