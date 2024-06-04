/*

  This file is a part of the Nova Physics Engine
  project and distributed under the MIT license.

  Copyright © Kadir Aksoy
  https://github.com/kadir014/nova-physics

*/

#include "../common.h"


size_t stack_rows = 30;
size_t stack_cols = 10;
float stack_box_size = 2.0;

void Stack_setup(ExampleContext *example) {
    nvRigidBody *ground;
    nvRigidBodyInitializer ground_init = nvRigidBodyInitializer_default;
    ground_init.position = NV_VEC2(64.0, 72.0 - 2.5);
    ground = nvRigidBody_new(ground_init);

    nvShape *ground_shape = nvBoxShape_new(128.0, 5.0, nvVector2_zero);
    nvRigidBody_add_shape(ground, ground_shape);

    nvSpace_add_body(example->space, ground);

    nv_float start_y = 72.0 - 2.5 - 2.5 - stack_box_size / 2.0;

    for (size_t y = 0; y < stack_rows; y++) {
        // Random horizontal offset for one row
        float offset = frand(-0.15, 0.15);

        for (size_t x = 0; x < stack_cols; x++) {

            nvRigidBody *box;
            nvRigidBodyInitializer box_init = nvRigidBodyInitializer_default;
            box_init.type = nvRigidBodyType_DYNAMIC;
            box_init.position = NV_VEC2(
                64.0 - stack_box_size * ((nv_float)stack_cols * 0.5) + x * stack_box_size + offset,
                start_y - y * (stack_box_size - 0.01) // Sink a little so collisions happen in first frame
            );
            box_init.material = (nvMaterial){.density=1.0, .restitution=0.1, .friction=0.6,};
            box = nvRigidBody_new(box_init);

            nvShape *box_shape = nvBoxShape_new(stack_box_size, stack_box_size, nvVector2_zero);
            nvRigidBody_add_shape(box, box_shape);

            nvSpace_add_body(example->space, box);
        }
    }

    nvRigidBody *a = example->space->bodies->data[3];
    nvRigidBody *b = example->space->bodies->data[4];

    nvDistanceConstraintInitializer init = nvDistanceConstraintInitializer_default;
    init.a = a;
    init.b = b;
    init.length = 15.0;
    
    nvConstraint *distcons = nvDistanceConstraint_new(init);
    //nvSpace_add_constraint(example->space, distcons);
}

void Stack_update(ExampleContext *example) {
    char display_buf[8];
    const float ratio[] = {0.25, 0.62, 0.13};
    bool changed = false;

    {
        nk_layout_row(example->ui_ctx, NK_DYNAMIC, 16, 3, ratio);

        nk_label(example->ui_ctx, "Rows", NK_TEXT_LEFT);

        if (nk_slider_int(example->ui_ctx, 1, &stack_rows, 50, 1))
            changed = true;
        
        sprintf(display_buf, "%d", stack_rows);
        nk_label(example->ui_ctx, display_buf, NK_TEXT_LEFT);
    }
    {
        nk_layout_row(example->ui_ctx, NK_DYNAMIC, 16, 3, ratio);

        nk_label(example->ui_ctx, "Columns", NK_TEXT_LEFT);

        if (nk_slider_int(example->ui_ctx, 1, &stack_cols, 15, 1))
            changed = true;
        
        sprintf(display_buf, "%d", stack_cols);
        nk_label(example->ui_ctx, display_buf, NK_TEXT_LEFT);
    }
    {
        nk_layout_row(example->ui_ctx, NK_DYNAMIC, 16, 3, ratio);

        nk_label(example->ui_ctx, "Box size", NK_TEXT_LEFT);

        if (nk_slider_float(example->ui_ctx, 0.5, &stack_box_size, 5.0, 0.1))
            changed = true;
        
        sprintf(display_buf, "%3.1f", stack_box_size);
        nk_label(example->ui_ctx, display_buf, NK_TEXT_LEFT);
    }

    if (changed) {
        nvSpace_clear(example->space, true);
        Stack_setup(example);
    }
}