/*

  This file is a part of the Nova Physics Engine
  project and distributed under the MIT license.

  Copyright © Kadir Aksoy
  https://github.com/kadir014/nova-physics

*/

#include "../common.h"


int pyramid_base = 100;
float pyramid_air_gap = 1.0;
float pyramid_box_size = 1.0;

void Pyramid_setup(ExampleContext *example) {
    nvRigidBody *ground;
    nvRigidBodyInitializer ground_init = nvRigidBodyInitializer_default;
    ground_init.position = NV_VECTOR2(64.0, 72.0 - 2.5);
    ground = nvRigidBody_new(ground_init);

    nvShape *ground_shape = nvBoxShape_new(128.0, 5.0, nvVector2_zero);
    nvRigidBody_add_shape(ground, ground_shape);

    nvSpace_add_rigidbody(example->space, ground);


    nv_float size = pyramid_box_size;
    nv_float s2 = size / 2.0;
    nv_float y_gap = pyramid_air_gap;
    nv_float start_y = 72.0 - 2.5 - 2.5 - s2;

    for (size_t y = 0; y < pyramid_base; y++) {
        for (size_t x = 0; x < pyramid_base - y; x++) {
            nvRigidBody *box;
            nvRigidBodyInitializer box_init = nvRigidBodyInitializer_default;
            box_init.type = nvRigidBodyType_DYNAMIC;
            box_init.position = NV_VECTOR2(
                64.0 - (pyramid_base * s2 - s2) + x * size + y * s2,
                start_y - y * (size + y_gap - 0.01) // Sink a little so collisions happen in first frame
            );
            box_init.material = (nvMaterial){.density=1.0, .restitution=0.0, .friction=0.5};
            box = nvRigidBody_new(box_init);

            nvShape *box_shape = nvBoxShape_new(size, size, nvVector2_zero);
            nvRigidBody_add_shape(box, box_shape);

            nvSpace_add_rigidbody(example->space, box);
        }
    }
}

void Pyramid_update(ExampleContext *example) {
    char display_buf[8];
    const float ratio[] = {0.25f, 0.62f, 0.13f};
    bool changed = false;

    {
        nk_layout_row(example->ui_ctx, NK_DYNAMIC, 16, 3, ratio);

        nk_label(example->ui_ctx, "Base", NK_TEXT_LEFT);

        if (nk_slider_int(example->ui_ctx, 3, &pyramid_base, 100, 1))
            changed = true;
        
        sprintf(display_buf, "%d", pyramid_base);
        nk_label(example->ui_ctx, display_buf, NK_TEXT_LEFT);
    }
    {
        nk_layout_row(example->ui_ctx, NK_DYNAMIC, 16, 3, ratio);

        nk_label(example->ui_ctx, "Air gap", NK_TEXT_LEFT);

        if (nk_slider_float(example->ui_ctx, 0.0f, &pyramid_air_gap, 1.0f, 0.1f))
            changed = true;
        
        sprintf(display_buf, "%3.1f", pyramid_air_gap);
        nk_label(example->ui_ctx, display_buf, NK_TEXT_LEFT);
    }
    {
        nk_layout_row(example->ui_ctx, NK_DYNAMIC, 16, 3, ratio);

        nk_label(example->ui_ctx, "Box size", NK_TEXT_LEFT);
    
        if (nk_slider_float(example->ui_ctx, 0.5f, &pyramid_box_size, 2.5f, 0.1f))
            changed = true;
        
        sprintf(display_buf, "%3.1f", pyramid_box_size);
        nk_label(example->ui_ctx, display_buf, NK_TEXT_LEFT);
    }

    if (changed) {
        nvSpace_clear(example->space, true);
        Pyramid_setup(example);
    }
}