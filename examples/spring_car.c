/*

  This file is a part of the Nova Physics Engine
  project and distributed under the MIT license.

  Copyright © Kadir Aksoy
  https://github.com/kadir014/nova-physics

*/

#include "example_base.h"


void update(Example *example) {
    if (example->keys[SDL_SCANCODE_LEFT] || example->keys[SDL_SCANCODE_RIGHT]) {
        nvBody *wheel1 = (nvBody *)example->space->bodies->data[6];
        nvBody *wheel2 = (nvBody *)example->space->bodies->data[5];

        double strength = 12.0 * 1e2;
        double limit = 30.0;

        if (example->keys[SDL_SCANCODE_LEFT]) {
            if (wheel1->angular_velocity > -limit) 
                wheel1->torque -= strength;

            if (wheel2->angular_velocity > -limit)
                wheel2->torque -= strength;
        }
        else {
            if (wheel1->angular_velocity < limit)
                wheel1->torque += strength;
            
            if (wheel2->angular_velocity < limit)
                wheel2->torque += strength;
        }
    }
}


void setup(Example *example) {
    // Create ground 
    nvBody *ground = nv_Rect_new(
        nvBodyType_STATIC,
        NV_VEC2(64.0, 36.0 + 25.0),
        0.0,
        nvMaterial_CONCRETE,
        128.0, 22.0
    );

    nvSpace_add(example->space, ground);

    nvBody *ground2 = nv_Rect_new(
        nvBodyType_STATIC,
        NV_VEC2(75.0, 50.0),
        -0.3,
        nvMaterial_CONCRETE,
        15.0, 3.0
    );

    nvSpace_add(example->space, ground2);

    nvBody *ground3 = nv_Rect_new(
        nvBodyType_STATIC,
        NV_VEC2(86.0, 43.0),
        -0.8,
        nvMaterial_CONCRETE,
        15.0, 3.0
    );

    nvSpace_add(example->space, ground3);

    nvBody *ground4 = nv_Rect_new(
        nvBodyType_STATIC,
        NV_VEC2(92.0, 35.5),
        -1.1,
        nvMaterial_CONCRETE,
        7.0, 3.0
    );

    nvSpace_add(example->space, ground4);


    // Create wheels

    nvMaterial wheel_mat = (nvMaterial){1.5, 0.3, 3.0};

    nvBody *wheel1 = nv_Circle_new(
        nvBodyType_DYNAMIC,
        NV_VEC2(53.0, 32.0),
        0.0,
        wheel_mat,
        2.0
    );
    wheel1->collision_group = 1;
    nvSpace_add(example->space, wheel1);

    nvBody *wheel2 = nv_Circle_new(
        nvBodyType_DYNAMIC,
        NV_VEC2(57.0, 32.0),
        0.0,
        wheel_mat,
        2.0
    );
    wheel2->collision_group = 1;
    nvSpace_add(example->space, wheel2);


    // Create car body
    nvBody *body = nv_Rect_new(
        nvBodyType_DYNAMIC,
        NV_VEC2(55.0, 30.0),
        0.0,
        (nvMaterial){4.0, 0.3, 0.5},
        10.0, 3.0
    );
    body->collision_group = 1;
    nvSpace_add(example->space, body);


    // Create spring constraints

    double suspension_length = 2.3;
    double suspension_strength = 2700.0;
    double suspension_damping = 150.00;

    nvConstraint *spring1 = nvSpring_new(
        wheel1, body,
        NV_VEC2(0.0, 0.0), NV_VEC2(-4.0, 0.0),
        suspension_length,
        suspension_strength * 1.0,
        suspension_damping
    );

    nvSpace_add_constraint(example->space, spring1);

    nvConstraint *spring2 = nvSpring_new(
        wheel1, body,
        NV_VEC2(0.0, 0.0), NV_VEC2(-1.5, 0.0),
        suspension_length,
        suspension_strength * 6.0,
        suspension_damping * 2.0
    );


    nvSpace_add_constraint(example->space, spring2);

    nvConstraint *spring3 = nvSpring_new(
        wheel2, body,
        NV_VEC2(0.0, 0.0), NV_VEC2(4.0, 0.0),
        suspension_length,
        suspension_strength * 1.0,
        suspension_damping
    );

    nvSpace_add_constraint(example->space, spring3);

    nvConstraint *spring4 = nvSpring_new(
        wheel2, body,
        NV_VEC2(0.0, 0.0), NV_VEC2(1.5, 0.0),
        suspension_length,
        suspension_strength * 6.0,
        suspension_damping * 2.0
    );

    nvSpace_add_constraint(example->space, spring4);
}


int main(int argc, char *argv[]) {
    // Create example
    Example *example = Example_new(
        1280, 720,
        "Nova Physics  -  Spring Car Example",
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