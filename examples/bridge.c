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
        "Nova Physics — Bridge Example",
        165.0,
        1.0/60.0
    );


    // Create grounds & bridge
    nv_Body *ground_left = nv_Rect_new(
        nv_BodyType_STATIC,
        (nv_Vector2){10.0, 52.5},
        0.0,
        nv_Material_CONCRETE,
        30.0, 40.0
    );

    nv_Space_add(example->space, ground_left);

    for (size_t i = 0; i < 25; i++) {
        nv_Body *bridge_part = nv_Rect_new(
            nv_BodyType_DYNAMIC,
            (nv_Vector2){28+i*3.0, 33},
            0.0,
            nv_Material_CONCRETE,
            3.0, 2.0
        );

        nv_Space_add(example->space, bridge_part);
    }

    nv_Body *ground_right = nv_Rect_new(
        nv_BodyType_STATIC,
        (nv_Vector2){118.0, 52.5},
        0.0,
        nv_Material_CONCRETE,
        30.0, 40.0
    );

    nv_Space_add(example->space, ground_right);

    // Link bridge parts with spring constraints
    for (size_t i = 0; i < 26; i++) {
        nv_Vector2 anchor_a;
        nv_Vector2 anchor_b;
        
        if (i == 0) {
            anchor_a = (nv_Vector2){15.0, -20.0};
            anchor_b = (nv_Vector2){-1.5, 0.0};
        }
        else if (i == 25) {
            anchor_a = (nv_Vector2){1.5, 0.0};
            anchor_b = (nv_Vector2){-15, -20.0};
        }
        else {
            // Offset anchors by 0.001 so they don't intersect
            anchor_a = (nv_Vector2){1.5-0.001, 0.0};
            anchor_b = (nv_Vector2){-1.5+0.001, 0.0};
        }

        nv_Body *a = (nv_Body *)example->space->bodies->data[i];
        nv_Body *b = (nv_Body *)example->space->bodies->data[i+1];

        nv_Constraint *spring = nv_SpringConstraint_new(
            a, b,
            anchor_a, anchor_b,
            0.1, 60.0, 0.67
        );

        nv_Space_add_constraint(example->space, spring);
    }

    // Create objects on top of the bridge
    for (size_t i = 0; i < 6; i++) {
        nv_Body *ball = nv_Circle_new(
            nv_BodyType_DYNAMIC,
            (nv_Vector2){35+i*2.2, 23},
            0.0,
            nv_Material_GLASS,
            2.2
        );

        nv_Space_add(example->space, ball);
    }


    // Run the example
    Example_run(example);

    // Free the space allocated by example
    Example_free(example);

    return 0;
}