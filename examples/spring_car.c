/*

  This file is a part of the Nova Physics Engine
  project and distributed under the MIT license.

  Copyright © Kadir Aksoy
  https://github.com/kadir014/nova-physics

*/

#include "example_base.h"


void update(Example *example) {
    if (example->keys[SDL_SCANCODE_LEFT] || example->keys[SDL_SCANCODE_RIGHT]) {
        nv_Body *wheel1 = (nv_Body *)example->space->bodies->data[1];
        nv_Body *wheel2 = (nv_Body *)example->space->bodies->data[2];

        double strength = 3 * 1e3;

        if (example->keys[SDL_SCANCODE_LEFT]) {
            if (wheel1->angular_velocity > -20.0) 
                wheel1->torque -= strength;

            if (wheel2->angular_velocity > -20.0)
                wheel2->torque -= strength;
        }
        else {
            if (wheel1->angular_velocity < 20.0)
                wheel1->torque += strength;
            
            if (wheel2->angular_velocity < 20.0)
                wheel2->torque += strength;
        }
    }
}


void setup(Example *example) {
    // Create ground 
    nv_Body *ground = nv_Rect_new(
        nv_BodyType_STATIC,
        (nv_Vector2){64.0, 62.5},
        0.0,
        nv_Material_CONCRETE,
        185.0, 5.0
    );

    nv_Space_add(example->space, ground);


    // Create wheels

    nv_Body *wheel1 = nv_Circle_new(
        nv_BodyType_DYNAMIC,
        (nv_Vector2){59.0, 50.0},
        0.0,
        nv_Material_RUBBER,
        3.0
    );
    
    nv_Space_add(example->space, wheel1);

    nv_Body *wheel2 = nv_Circle_new(
        nv_BodyType_DYNAMIC,
        (nv_Vector2){71.0, 50.0},
        0.0,
        nv_Material_RUBBER,
        3.0
    );
    
    nv_Space_add(example->space, wheel2);


    // Create car body
    nv_Body *body = nv_Rect_new(
        nv_BodyType_DYNAMIC,
        (nv_Vector2){65.0, 40.0},
        0.0,
        nv_Material_WOOD,
        16.0, 6.0
    );
    
    nv_Space_add(example->space, body);


    // Create spring constraints

    double suspension_length = 5.5;
    double suspension_strength = 6.7;
    double suspension_damping = 0.5;

    nv_Constraint *spring1 = nv_SpringConstraint_new(
        wheel1, body,
        (nv_Vector2){0.0, 0.0}, (nv_Vector2){-8.0, 3.0},
        suspension_length,
        suspension_strength,
        suspension_damping
    );

    nv_Space_add_constraint(example->space, spring1);

    nv_Constraint *spring2 = nv_SpringConstraint_new(
        wheel1, body,
        (nv_Vector2){0.0, 0.0}, (nv_Vector2){-3.0, 3.0},
        suspension_length,
        suspension_strength,
        suspension_damping
    );


    nv_Space_add_constraint(example->space, spring2);

    nv_Constraint *spring3 = nv_SpringConstraint_new(
        wheel2, body,
        (nv_Vector2){0.0, 0.0}, (nv_Vector2){8.0, 3.0},
        suspension_length,
        suspension_strength,
        suspension_damping
    );

    nv_Space_add_constraint(example->space, spring3);

    nv_Constraint *spring4 = nv_SpringConstraint_new(
        wheel2, body,
        (nv_Vector2){0.0, 0.0}, (nv_Vector2){3.0, 3.0},
        suspension_length,
        suspension_strength,
        suspension_damping
    );

    nv_Space_add_constraint(example->space, spring4);
}


int main(int argc, char *argv[]) {
    // Create example
    Example *example = Example_new(
        1280, 720,
        "Nova Physics — Spring Car Example",
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