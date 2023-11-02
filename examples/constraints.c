/*

  This file is a part of the Nova Physics Engine
  project and distributed under the MIT license.

  Copyright © Kadir Aksoy
  https://github.com/kadir014/nova-physics

*/

#include "example_base.h"


void setup(Example *example) {
     nv_Body *platform = nv_Rect_new(
        nv_BodyType_DYNAMIC,
        NV_VEC2(65.0, 50.0),
        0.0,
        nv_Material_WOOD,
        70.0, 3.0
    );
    
    nv_Space_add(example->space, platform);

    // Create spring constraints and attach bodies

    nv_Constraint *spring1 = nv_Spring_new(
        NULL, platform,
        NV_VEC2(35.0, 20.0), NV_VEC2(-30.0, 0.0),
        30.0, 500.0, 3.0
    );

    nv_Space_add_constraint(example->space, spring1);

    nv_Constraint *spring2 = nv_Spring_new(
        NULL, platform,
        NV_VEC2(95.0, 20.0), NV_VEC2(30.0, 0.0),
        30.0, 500.0, 3.0
    );

    nv_Space_add_constraint(example->space, spring2);


    nv_Body *circle = nv_Circle_new(
        nv_BodyType_DYNAMIC,
        NV_VEC2(64.0, 20.0),
        0.0,
        nv_Material_WOOD,
        3.0
    );

    nv_Space_add(example->space, circle);

    // Create distance joint constraint and attach bodies
    nv_Constraint *dist_joint = nv_DistanceJoint_new(
        NULL, circle,
        NV_VEC2(64.0, 10.0), NV_VEC2(0.0, -3.0),
        7.0
    );

    nv_Space_add_constraint(example->space, dist_joint);


    // Create boxes

    nv_Body *box1 = nv_Rect_new(
        nv_BodyType_DYNAMIC,
        NV_VEC2(66.0, 29.0),
        0.0,
        nv_Material_BASIC,
        3.5, 3.5
    );
    
    nv_Space_add(example->space, box1);

    nv_Body *box2 = nv_Rect_new(
        nv_BodyType_DYNAMIC,
        NV_VEC2(62.5, 29.0),
        0.0,
        nv_Material_BASIC,
        3.5, 3.5
    );
    
    nv_Space_add(example->space, box2);

    nv_Body *box3 = nv_Rect_new(
        nv_BodyType_DYNAMIC,
        NV_VEC2(64.25, 25.5),
        0.0,
        nv_Material_BASIC,
        3.5, 3.5
    );
    
    nv_Space_add(example->space, box3);
}


int main(int argc, char *argv[]) {
    // Create example
    Example *example = Example_new(
        1280, 720,
        "Nova Physics  -  Constraints Example",
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