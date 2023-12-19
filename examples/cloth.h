/*

  This file is a part of the Nova Physics Engine
  project and distributed under the MIT license.

  Copyright © Kadir Aksoy
  https://github.com/kadir014/nova-physics

*/

#include "example.h"


void ClothExample_setup(Example *example) {
    nvSpace *space = example->space;
    
    // Basically disable broadphase
    nvSpace_set_SHG(space, (nvAABB){0.0, 0.0, 1.0, 1.0}, 1.0, 1.0);
    
    int cols = 50;
    int rows = 50;
    nv_float size = 0.75;
    nv_float gap = 0.3;

    for (nv_float y = 0.0; y < rows; y++) {
        for (nv_float x = 0.0; x < cols; x++) {

            nvBodyType type;
            if ((y == 0.0 && x == 0.0) || (y == 0.0 && x == cols - 1)) type = nvBodyType_STATIC;
            else type = nvBodyType_DYNAMIC;
            type = nvBodyType_DYNAMIC;

            nvBody *ball = nvBody_new(
                type,
                nvCircleShape_new(size / 2.0),
                NV_VEC2(
                    64.0 + x * (size + gap) - ((size + gap) * (nv_float)cols / 2.0),
                    y * (size + gap) + 10.0
                ),
                0.0,
                (nvMaterial){0.3, 0.0, 0.0}
            );
            ball->enable_collision = false;
            nvSpace_add(space, ball);
        }
    }

    nvConstraint *link;
    nv_float link_stiffness = 600.0;
    nv_float link_damping = 5.0;
    bool use_springs = true;

    for (size_t y = 0; y < rows; y++) {
        for (size_t x = 0; x < cols; x++) {
            if (x > 0) {
                nvBody *body0 = space->bodies->data[y * rows + x + 1];
                nvBody *body1 = space->bodies->data[y * rows + (x - 1) + 1];

                if (use_springs) {
                    link = nvSpring_new(
                        body0, body1,
                        nvVector2_zero, nvVector2_zero,
                        size + gap,
                        link_stiffness, link_damping
                    );
                }
                else {
                    link = nvDistanceJoint_new(
                        body0, body1,
                        nvVector2_zero, nvVector2_zero,
                        size + gap
                    );
                }

                nvSpace_add_constraint(space, link);
            }

            if (y > 0) {
                nvBody *body0 = space->bodies->data[(y - 1) * rows + x + 1];
                nvBody *body1 = space->bodies->data[y * rows + x + 1];

                if (use_springs) {
                    link = nvSpring_new(
                        body0, body1,
                        nvVector2_zero, nvVector2_zero,
                        size + gap,
                        link_stiffness, link_damping
                    );
                }
                else {
                    link = nvDistanceJoint_new(
                        body0, body1,
                        nvVector2_zero, nvVector2_zero,
                        size + gap
                    );
                }

                nvSpace_add_constraint(space, link);
            }

            else {
                nvBody *body0 = NULL;
                nvBody *body1 = space->bodies->data[y * rows + x + 1];

                if (use_springs) {
                    link = nvSpring_new(
                        body0, body1,
                        NV_VEC2(body1->position.x, body1->position.y - size - gap), nvVector2_zero,
                        size + gap,
                        link_stiffness, link_damping
                    );
                }
                else {
                    link = nvDistanceJoint_new(
                        body0, body1,
                        NV_VEC2(body1->position.x, body1->position.y - size - gap), nvVector2_zero,
                        size + gap
                    );
                }

                nvSpace_add_constraint(space, link);
            }
        }
    }

    // Apply horizontal force to some cloth nodes
    for (size_t i = 0; i < space->bodies->size; i++) {
        if (i > 1000) {
            nvBody *body = space->bodies->data[i];
            nvBody_apply_force(body, NV_VEC2(frand(-70.0, 350.0), frand(-100.0, 200.0)));
        }
    }

    example->switches[4]->on = false; // Disable drawing constraints
}