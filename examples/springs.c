/*

  This file is a part of the Nova Physics Engine
  project and distributed under the MIT license.

  Copyright © Kadir Aksoy
  https://github.com/kadir014/nova-physics

*/

#include "example_base.h"


void setup(Example *example) {
    // Create bodies

    nv_Body *circle1 = nv_Circle_new(
        nv_BodyType_STATIC,
        NV_VEC2(35.0, 20.0),
        0.0,
        nv_Material_WOOD,
        3.0
    );
    
    nv_Space_add(example->space, circle1);

    nv_Body *circle2 = nv_Circle_new(
        nv_BodyType_STATIC,
        NV_VEC2(95.0, 20.0),
        0.0,
        nv_Material_WOOD,
        3.0
    );
    
    nv_Space_add(example->space, circle2);

    nv_Body *platform = nv_Rect_new(
        nv_BodyType_DYNAMIC,
        NV_VEC2(65.0, 50.0),
        0.0,
        nv_Material_WOOD,
        70.0, 3.0
    );
    
    nv_Space_add(example->space, platform);

    // Create boxes

    nv_Body *box1 = nv_Rect_new(
        nv_BodyType_DYNAMIC,
        NV_VEC2(66.0, 29.0),
        0.0,
        nv_Material_STEEL,
        3.5, 3.5
    );
    
    nv_Space_add(example->space, box1);

    nv_Body *box2 = nv_Rect_new(
        nv_BodyType_DYNAMIC,
        NV_VEC2(62.5, 29.0),
        0.0,
        nv_Material_STEEL,
        3.5, 3.5
    );
    
    nv_Space_add(example->space, box2);

    nv_Body *box3 = nv_Rect_new(
        nv_BodyType_DYNAMIC,
        NV_VEC2(64.25, 25.5),
        0.0,
        nv_Material_WOOD,
        3.5, 3.5
    );
    
    nv_Space_add(example->space, box3);


    // Create spring constraints and attach bodies with it

    nv_Constraint *spring1 = nv_SpringConstraint_new(
        circle1, platform,
        NV_VEC2(0.0, 0.0), NV_VEC2(-30.0, 0.0),
        30.0, 4.45, 0.4
    );

    nv_Space_add_constraint(example->space, spring1);

    nv_Constraint *spring2 = nv_SpringConstraint_new(
        circle2, platform,
        NV_VEC2(0.0, 0.0), NV_VEC2(30.0, 0.0),
        30.0, 4.45, 0.4
    );

    nv_Space_add_constraint(example->space, spring2);
}


int main(int argc, char *argv[]) {
    // Create example
    Example *example = Example_new(
        1280, 720,
        "Nova Physics — Spring Constraint Example",
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