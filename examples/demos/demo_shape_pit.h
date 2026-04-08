/*

  This file is a part of the Nova Physics Engine
  project and distributed under the MIT license.

  Copyright © Kadir Aksoy
  https://github.com/kadir014/nova-physics

*/

#include "../common.h"


#define PIT_ROWS 500
#define PIT_COLS 15
#define PIT_SIZE 1.0

int pit_shape = 0;

void ShapePit_setup(ExampleContext *example) {
    nvRigidBodyInitializer ground_init = nvRigidBodyInitializer_default;
    ground_init.position = NV_VECTOR2(64.0, 72.0 - 2.5);
    nvRigidBody *ground = nvRigidBody_new(ground_init);

    nvRigidBody_add_shape(ground, nvBoxShape_new(100.0, 5.0, nvVector2_zero));
    nvRigidBody_add_shape(ground, nvBoxShape_new(5.0, 150.0, NV_VECTOR2(-50.0, -75.0)));
    nvRigidBody_add_shape(ground, nvBoxShape_new(5.0, 150.0, NV_VECTOR2(50.0, -75.0)));

    nvSpace_add_rigidbody(example->space, ground);


    nv_float start_y = 0.0;

    for (size_t y = 0; y < PIT_ROWS; y++) {
        // Random horizontal offset for each row
        float offset = frand(-0.1f, 0.1f);

        for (size_t x = 0; x < PIT_COLS; x++) {

            nvRigidBodyInitializer body_init = nvRigidBodyInitializer_default;
            body_init.type = nvRigidBodyType_DYNAMIC;
            body_init.position = NV_VECTOR2(
                64.0 - PIT_SIZE * ((nv_float)PIT_COLS * 0.5) + x * PIT_SIZE + offset,
                start_y - (nv_float)y * 1.5
            );
            body_init.linear_velocity = NV_VECTOR2(0.0, 50.0);
            body_init.material = (nvMaterial){.density=1.0, .restitution=0.0, .friction=0.5};
            nvRigidBody *body = nvRigidBody_new(body_init);

            nvShape *body_shape;
            if (pit_shape == 1) {
                body_shape = nvNGonShape_new(3, PIT_SIZE * 0.5f, nvVector2_zero);
            }
            else if (pit_shape == 2) {
                body_shape = nvBoxShape_new(PIT_SIZE, PIT_SIZE, nvVector2_zero);
            }
            else if (pit_shape == 3) {
                body_shape = nvNGonShape_new(6, PIT_SIZE * 0.5f, nvVector2_zero);
            }
            else {
                body_shape = nvCircleShape_new(nvVector2_zero, PIT_SIZE * 0.5f);
            }

            nvRigidBody_add_shape(body, body_shape);

            nvSpace_add_rigidbody(example->space, body);
        }
    }
}

void ShapePit_update(ExampleContext *example) {
    bool changed = false;

    char *shape_labels[4] = {"Circle", "Triangle", "Box", "Hexagon"};

    {
        nk_layout_row_dynamic(example->ui_ctx, 16, 2);

        nk_label(example->ui_ctx, "Shape", NK_TEXT_LEFT);

        if (nk_combo_begin_label(example->ui_ctx, shape_labels[pit_shape], nk_vec2(nk_widget_width(example->ui_ctx), 200))) {
            nk_layout_row_dynamic(example->ui_ctx, 16, 1);
            for (int i = 0; i < 4; i++) {
                if (nk_combo_item_label(example->ui_ctx, shape_labels[i], NK_TEXT_LEFT)) {
                    pit_shape = i;
                    changed = true;
                }
            }
            nk_combo_end(example->ui_ctx);
        }
    }

    if (changed) {
        ExampleContext_reset(example);
    }
}