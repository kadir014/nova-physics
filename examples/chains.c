/*

  This file is a part of the Nova Physics Engine
  project and distributed under the MIT license.

  Copyright © Kadir Aksoy
  https://github.com/kadir014/nova-physics

*/

#include "example_base.h"


void setup(Example *example) {
    nv_float pos_ratio0 = 60.0;
    nv_float pos_ratio1 = 180.0;

    // Create rectangle chain parts

    int length = 20; // Length of the chain
    nv_float width = 0.7; // Width of the chain parts
    nv_float height = 1.4; // Height of the chain parts

    for (size_t i = 0; i < length; i++) {
        nvBody *chain_part = nv_Rect_new(
            nv_BodyType_DYNAMIC,
            NV_VEC2(
                example->width / 20.0 - example->width / pos_ratio1,
                10 + i * height
            ),
            0.0,
            (nvMaterial){1.0, 0.0, 0.0},
            width, height
        );
        chain_part->collision_group = 1;
        nvSpace_add(example->space, chain_part);

        // Temporary solution to avoid constraints exploding
        nvBody_apply_force(chain_part, NV_VEC2((nv_float)(i%10)*50.0, 0.0)); 
    }

    // Link chain parts

    nv_float link_length = height; // Length of each link

    for (size_t i = 0; i < length; i++) {
        nvConstraint *link;
        if (i == 0) {
            link = nvSpring_new(
                NULL,
                (nvBody *)example->space->bodies->data[i + 1],
                NV_VEC2(example->width / 20.0 - example->width / pos_ratio1, 10.0),
                NV_VEC2(0.0, -height / 2.0 + 0.001),
                1.0, 600.0, 25.0
            );
        }
        else {
            link = nvDistanceJoint_new(
                (nvBody *)example->space->bodies->data[i],
                (nvBody *)example->space->bodies->data[i + 1],
                NV_VEC2(0.0,  height / 2.0 - 0.001),
                NV_VEC2(0.0, -height / 2.0 + 0.001),
                link_length
            );
        }

        nvSpace_add_constraint(example->space, link);
    }

    // Cache this to accurately create & link other chain bodies
    size_t old_length = example->space->bodies->size - 1;


    // Create circle chain parts

    length = 30; // Length of the chain
    nv_float radius = 0.7; // Radius of the chain parts

    for (size_t i = 0; i < length; i++) {
        nvBody *chain_part = nv_Circle_new(
            nv_BodyType_DYNAMIC,
            NV_VEC2(
                example->width / 20.0 - example->width / pos_ratio0,
                10 + i * radius * 2.0
            ),
            0.0,
            (nvMaterial){1.0, 0.0, 0.0},
            radius
        );
        chain_part->collision_group = 2;
        nvSpace_add(example->space, chain_part);

        // Temporary solution to avoid constraints exploding
        nvBody_apply_force(chain_part, NV_VEC2((nv_float)(i%10)*50.0, 0.0)); 
    }

    // Link chain parts

    for (size_t i = old_length; i < old_length + length; i++) {
        nvConstraint *link;
        if (i == old_length) {
            link = nvSpring_new(
                NULL,
                (nvBody *)example->space->bodies->data[i + 1],
                NV_VEC2(example->width / 20.0 - example->width / pos_ratio0, 10.0),
                NV_VEC2(0.0, -height / 2.0 + 0.001),
                1.0, 600.0, 25.0
            );
        }
        else {
            link = nvDistanceJoint_new(
                (nvBody *)example->space->bodies->data[i],
                (nvBody *)example->space->bodies->data[i + 1],
                nvVector2_zero,
                nvVector2_zero,
                radius * 2.0 + 0.01
            );
        }

        nvSpace_add_constraint(example->space, link);
    }

    // Cache this to accurately create & link other chain bodies
    old_length = example->space->bodies->size - 1;


    // Create chain parts

    length = 15; // Length of the chain
    nv_float size = 1.0; // Size of the chain parts

    for (size_t i = 0; i < length; i++) {
        nvBody *chain_part = nv_Rect_new(
            nv_BodyType_DYNAMIC,
            NV_VEC2(
                example->width / 20.0 + example->width / pos_ratio0,
                10 + i * (size + 0.5) / 3.0
            ),
            0.0,
            (nvMaterial){1.0, 0.0, 0.0},
            size, size
        );
        chain_part->collision_group = 3;
        nvSpace_add(example->space, chain_part);
    }

    // Link chain parts with springs

    link_length = 0.5;
    nv_float spring_stiffness = 120.0;
    nv_float spring_damping = 35.0;

    for (size_t i = old_length; i < old_length + length; i++) {
        nvConstraint *link;
        if (i == old_length) {
            link = nvSpring_new(
                NULL,
                (nvBody *)example->space->bodies->data[i + 1],
                NV_VEC2(example->width / 20.0 + example->width / pos_ratio0, 10.0),
                NV_VEC2(-size / 2.0, -height / 2.0 + 0.001),
                1.0, 600.0, 25.0
            );
        }
        else {
                link = nvSpring_new(
                    (nvBody *)example->space->bodies->data[i],
                    (nvBody *)example->space->bodies->data[i + 1],
                    NV_VEC2(size / 2.0, size / 2.0),
                    NV_VEC2(-size / 2.0, -size / 2.0),
                    link_length, spring_stiffness, spring_damping
                );
        }

        nvSpace_add_constraint(example->space, link);
    }

    old_length = example->space->bodies->size - 1;

    // Create chain parts

    length = 30; // Length of the chain
    width = 0.75; // Width of the chain parts
    height = 1.5; // Height of the chain parts

    for (size_t i = 0; i < length; i++) {
        nvBody *chain_part = nv_Rect_new(
            nv_BodyType_DYNAMIC,
            NV_VEC2(
                example->width / 20.0 + example->width / pos_ratio1,
                10 + i * height
            ),
            0.0,
            (nvMaterial){1.0, 0.0, 0.0},
            width, height
        );
        chain_part->collision_group = 4;
        nvSpace_add(example->space, chain_part);

        // Temporary solution to avoid constraints exploding
        nvBody_apply_force(chain_part, NV_VEC2((nv_float)(i%10)*50.0, 0.0));
    }

    // Link chain parts

    for (size_t i = old_length; i < old_length + length; i++) {
        nvConstraint *link;
        if (i == old_length) {
            link = nvSpring_new(
                NULL,
                (nvBody *)example->space->bodies->data[i + 1],
                NV_VEC2(example->width / 20.0 + example->width / pos_ratio1, 10.0),
                NV_VEC2(0.0, -height / 2.0 + 0.001),
                1.0, 600.0, 25.0
            );
        }
        else {
            link = nvHingeJoint_new(
                (nvBody *)example->space->bodies->data[i],
                (nvBody *)example->space->bodies->data[i + 1],
                nvVector2_add(
                    ((nvBody *)example->space->bodies->data[i])->position,
                    NV_VEC2(0.0, height/2.0)
                )
            );
        }

        nvSpace_add_constraint(example->space, link);
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