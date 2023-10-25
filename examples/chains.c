/*

  This file is a part of the Nova Physics Engine
  project and distributed under the MIT license.

  Copyright © Kadir Aksoy
  https://github.com/kadir014/nova-physics

*/

#include "example_base.h"


void setup(Example *example) {
    // Create rectangle chain parts

    int length = 30; // Length of the chain
    nv_float width = 0.7; // Width of the chain parts
    nv_float height = 1.4; // Height of the chain parts

    for (size_t i = 0; i < length; i++) {
        nv_Body *chain_part = nv_Rect_new(
            i == 0 ? nv_BodyType_STATIC : nv_BodyType_DYNAMIC,
            NV_VEC2(
                example->width / 20.0 - 15.0,
                10 + i * height
            ),
            0.0,
            (nv_Material){1.0, 0.0, 0.0},
            width, height
        );

        nv_Space_add(example->space, chain_part);
    }

    // Link chain parts

    nv_float link_length = 0.1; // Length of each link

    for (size_t i = 1; i < length; i++) {
        nv_Constraint *link = nv_DistanceJoint_new(
            (nv_Body *)example->space->bodies->data[i],
            (nv_Body *)example->space->bodies->data[i + 1],
            NV_VEC2(0.0,  height / 2.0 - 0.001),
            NV_VEC2(0.0, -height / 2.0 + 0.001),
            link_length + 0.002
        );

        nv_Space_add_constraint(example->space, link);
    }

    // Cache this to accurately create & link other chain bodies
    size_t old_length = example->space->bodies->size - 1;


    // Create circle chain parts

    length = 30; // Length of the chain
    nv_float radius = 0.7; // Radius of the chain parts

    for (size_t i = 0; i < length; i++) {
        nv_Body *chain_part = nv_Circle_new(
            i == 0 ? nv_BodyType_STATIC : nv_BodyType_DYNAMIC,
            NV_VEC2(
                example->width / 20.0,
                10 + i * radius * 2.0
            ),
            0.0,
            (nv_Material){1.0, 0.0, 0.0},
            radius
        );

        nv_Space_add(example->space, chain_part);

        // Temporary solution to circle chain going crazy
        nv_Body_apply_force(chain_part, NV_VEC2((nv_float)(i%10)*50.0, 0.0)); 
    }

    // Link chain parts

    for (size_t i = old_length + 1; i < old_length + length; i++) {
        nv_Constraint *link = nv_DistanceJoint_new(
            (nv_Body *)example->space->bodies->data[i],
            (nv_Body *)example->space->bodies->data[i + 1],
            nv_Vector2_zero,
            nv_Vector2_zero,
            radius * 2.0 + 0.01
        );

        nv_Space_add_constraint(example->space, link);
    }

    // Cache this to accurately create & link other chain bodies
    old_length = example->space->bodies->size - 1;


    // Create chain parts

    length = 17; // Length of the chain
    nv_float size = 2.5; // Size of the chain parts

    for (size_t i = 0; i < length; i++) {
        nv_Body *chain_part = nv_Rect_new(
            i == 0 ? nv_BodyType_STATIC : nv_BodyType_DYNAMIC,
            NV_VEC2(
                example->width / 20.0 + 15.0,
                10 + i * (size + 1.5) / 3.0
            ),
            0.0,
            (nv_Material){1.0, 0.0, 0.0},
            size, size / 3.0
        );

        nv_Space_add(example->space, chain_part);
    }

    // Link chain parts with very stiff springs

    link_length = 0.5;

    for (size_t i = old_length + 1; i < old_length + length; i++) {
        nv_Constraint *link1 = nv_Spring_new(
            (nv_Body *)example->space->bodies->data[i],
            (nv_Body *)example->space->bodies->data[i + 1],
            NV_VEC2(-size / 2.0, size / 3.0 / 2.0),
            NV_VEC2(-size / 2.0, -size / 3.0 / 2.0),
            link_length, 0.6, 0.3
        );

        nv_Space_add_constraint(example->space, link1);

        nv_Constraint *link2 = nv_Spring_new(
            (nv_Body *)example->space->bodies->data[i],
            (nv_Body *)example->space->bodies->data[i + 1],
            NV_VEC2(size / 2.0, size / 3.0 / 2.0),
            NV_VEC2(size / 2.0, -size / 3.0 / 2.0),
            link_length, 0.6, 0.3
        );

        nv_Space_add_constraint(example->space, link2);
    }
}


int main(int argc, char *argv[]) {
    // Create example
    Example *example = Example_new(
        1280, 720,
        "Nova Physics  -  Chains Example",
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