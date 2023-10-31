/*

  This file is a part of the Nova Physics Engine
  project and distributed under the MIT license.

  Copyright © Kadir Aksoy
  https://github.com/kadir014/nova-physics

*/

#include "example_base.h"


void update(Example *example) {
    if (example->keys[SDL_SCANCODE_LEFT] || example->keys[SDL_SCANCODE_RIGHT]) {
        nv_Body *wheel1 = (nv_Body *)example->space->bodies->data[4];
        nv_Body *wheel2 = (nv_Body *)example->space->bodies->data[5];

        double strength = 7.0 * 1e2;
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
    nv_Body *ground = nv_Rect_new(
        nv_BodyType_STATIC,
        NV_VEC2(64.0, 36.0 + 25.0),
        0.0,
        nv_Material_CONCRETE,
        128.0, 22.0
    );

    nv_Space_add(example->space, ground);

    nv_Body *ground2 = nv_Rect_new(
        nv_BodyType_STATIC,
        NV_VEC2(75.0, 50.0),
        -0.3,
        nv_Material_CONCRETE,
        15.0, 3.0
    );

    nv_Space_add(example->space, ground2);

    nv_Body *ground3 = nv_Rect_new(
        nv_BodyType_STATIC,
        NV_VEC2(86.0, 43.0),
        -0.8,
        nv_Material_CONCRETE,
        15.0, 3.0
    );

    nv_Space_add(example->space, ground3);

    nv_Body *ground4 = nv_Rect_new(
        nv_BodyType_STATIC,
        NV_VEC2(92.0, 35.5),
        -1.1,
        nv_Material_CONCRETE,
        7.0, 3.0
    );

    nv_Space_add(example->space, ground4);


    // Create wheels

    nv_Material wheel_mat = (nv_Material){1.5, 0.8, 3.0};

    nv_Body *wheel1 = nv_Circle_new(
        nv_BodyType_DYNAMIC,
        NV_VEC2(49.0, 40.0),
        0.0,
        wheel_mat,
        1.5
    );
    
    nv_Space_add(example->space, wheel1);

    nv_Body *wheel2 = nv_Circle_new(
        nv_BodyType_DYNAMIC,
        NV_VEC2(61.0, 40.0),
        0.0,
        wheel_mat,
        1.5
    );
    
    nv_Space_add(example->space, wheel2);


    // Create car body
    nv_Body *body = nv_Rect_new(
        nv_BodyType_DYNAMIC,
        NV_VEC2(55.0, 30.0),
        0.0,
        (nv_Material){4.0, 0.3, 0.5},
        10.0, 3.0
    );
    
    nv_Space_add(example->space, body);


    // Create spring constraints

    double suspension_length = 2.5;
    double suspension_strength = 1500.0;
    double suspension_damping = 150.00;

    nv_Constraint *spring1 = nv_Spring_new(
        wheel1, body,
        NV_VEC2(0.0, 0.0), NV_VEC2(-4.0, 1.5),
        suspension_length,
        suspension_strength,
        suspension_damping
    );

    nv_Space_add_constraint(example->space, spring1);

    nv_Constraint *spring2 = nv_Spring_new(
        wheel1, body,
        NV_VEC2(0.0, 0.0), NV_VEC2(-1.5, 1.5),
        suspension_length,
        suspension_strength * 3.0,
        suspension_damping
    );


    nv_Space_add_constraint(example->space, spring2);

    nv_Constraint *spring3 = nv_Spring_new(
        wheel2, body,
        NV_VEC2(0.0, 0.0), NV_VEC2(4.0, 1.5),
        suspension_length,
        suspension_strength,
        suspension_damping
    );

    nv_Space_add_constraint(example->space, spring3);

    nv_Constraint *spring4 = nv_Spring_new(
        wheel2, body,
        NV_VEC2(0.0, 0.0), NV_VEC2(1.5, 1.5),
        suspension_length,
        suspension_strength * 3.0,
        suspension_damping
    );

    nv_Space_add_constraint(example->space, spring4);
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