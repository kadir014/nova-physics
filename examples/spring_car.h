/*

  This file is a part of the Nova Physics Engine
  project and distributed under the MIT license.

  Copyright © Kadir Aksoy
  https://github.com/kadir014/nova-physics

*/

#include "example.h"


void SpringCarExample_update(Example *example) {
    nvSpace *space = example->space;

    if (example->keys[SDL_SCANCODE_LEFT] || example->keys[SDL_SCANCODE_RIGHT]) {
        nvBody *wheel1 = (nvBody *)space->bodies->data[5];

        double strength = 18.0 * 1e2;
        double limit = 30.0;

        if (example->keys[SDL_SCANCODE_LEFT]) {
            if (wheel1->angular_velocity > -limit) 
                wheel1->torque -= strength;
        }
        else {
            if (wheel1->angular_velocity < limit)
                wheel1->torque += strength;
        }
    }
}


void SpringCarExample_setup(Example *example) {
    nvSpace *space = example->space;
    
    // Create ground 
    nvBody *ground = nvBody_new(
        nvBodyType_STATIC,
        nvRectShape_new(128.0, 22.0),
        NV_VEC2(64.0, 36.0 + 25.0),
        0.0,
        nvMaterial_CONCRETE
    );

    nvSpace_add(space, ground);

    nvBody *ground2 = nvBody_new(
        nvBodyType_STATIC,
        nvRectShape_new(15.0, 3.0),
        NV_VEC2(75.0, 50.0),
        -0.3,
        nvMaterial_CONCRETE
    );

    nvSpace_add(space, ground2);

    nvBody *ground3 = nvBody_new(
        nvBodyType_STATIC,
        nvRectShape_new(15.0, 3.0),
        NV_VEC2(86.0, 43.0),
        -0.8,
        nvMaterial_CONCRETE
    );

    nvSpace_add(space, ground3);

    nvBody *ground4 = nvBody_new(
        nvBodyType_STATIC,
        nvRectShape_new(7.0, 3.0),
        NV_VEC2(92.0, 35.5),
        -1.1,
        nvMaterial_CONCRETE
    );

    nvSpace_add(space, ground4);


    // Create wheels

    nvMaterial wheel_mat = (nvMaterial){1.5, 0.3, 3.0};

    nvBody *wheel1 = nvBody_new(
        nvBodyType_DYNAMIC,
        nvCircleShape_new(1.8),
        NV_VEC2(53.0, 32.0),
        0.0,
        wheel_mat
    );
    wheel1->collision_group = 1;
    nvSpace_add(space, wheel1);

    nvBody *wheel2 = nvBody_new(
        nvBodyType_DYNAMIC,
        nvCircleShape_new(1.8),
        NV_VEC2(57.0, 32.0),
        0.0,
        wheel_mat
    );
    wheel2->collision_group = 1;
    nvSpace_add(space, wheel2);


    // Create car body
    nvArray *car_body_vertices = nvArray_new();
    nvArray_add(car_body_vertices, NV_VEC2_NEW(-5.0, 2.5));
    nvArray_add(car_body_vertices, NV_VEC2_NEW(-5.0, 0.0));
    nvArray_add(car_body_vertices, NV_VEC2_NEW(-3.0, -2.5));
    nvArray_add(car_body_vertices, NV_VEC2_NEW(1.0, -2.5));
    nvArray_add(car_body_vertices, NV_VEC2_NEW(5.0, 0.0));
    nvArray_add(car_body_vertices, NV_VEC2_NEW(5.0, 2.5));

    // Fix the centroid of the polygon shape
    nvVector2 centroid = nv_polygon_centroid(car_body_vertices);
    for (size_t i = 0; i < car_body_vertices->size; i++) {
        nvVector2 vert = NV_TO_VEC2(car_body_vertices->data[i]);

        vert = nvVector2_sub(vert, centroid);

        NV_TO_VEC2P(car_body_vertices->data[i])->x = vert.x;
        NV_TO_VEC2P(car_body_vertices->data[i])->y = vert.y;
    }

    nvBody *car_body = nvBody_new(
        nvBodyType_DYNAMIC,
        nvPolygonShape_new(car_body_vertices),
        NV_VEC2(55.0, 30.0),
        0.0,
        (nvMaterial){4.0, 0.3, 0.5}
    );
    car_body->collision_group = 1;
    nvSpace_add(space, car_body);


    // Create spring constraints

    double suspension_length = 2.3;
    double suspension_strength = 2500.0;
    double suspension_damping = 150.00;

    nvConstraint *spring1 = nvSpring_new(
        wheel1, car_body,
        NV_VEC2(0.0, 0.0), NV_VEC2(-3.5, 0.4),
        suspension_length,
        suspension_strength,
        suspension_damping
    );

    nvSpace_add_constraint(space, spring1);

    nvConstraint *spring2 = nvSpring_new(
        wheel1, car_body,
        NV_VEC2(0.0, 0.0), NV_VEC2(-1.0, 0.4),
        suspension_length,
        suspension_strength * 6.0,
        suspension_damping * 2.0
    );


    nvSpace_add_constraint(space, spring2);

    nvConstraint *spring3 = nvSpring_new(
        wheel2, car_body,
        NV_VEC2(0.0, 0.0), NV_VEC2(4.0, 0.4),
        suspension_length,
        suspension_strength,
        suspension_damping
    );

    nvSpace_add_constraint(space, spring3);

    nvConstraint *spring4 = nvSpring_new(
        wheel2, car_body,
        NV_VEC2(0.0, 0.0), NV_VEC2(1.5, 0.4),
        suspension_length,
        suspension_strength * 6.0,
        suspension_damping * 2.0
    );

    nvSpace_add_constraint(space, spring4);
}