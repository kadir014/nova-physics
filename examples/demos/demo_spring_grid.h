/*

  This file is a part of the Nova Physics Engine
  project and distributed under the MIT license.

  Copyright © Kadir Aksoy
  https://github.com/kadir014/nova-physics

*/

#include "../common.h"


int springgrid_rows = 30;
int springgrid_cols = 30;
nk_bool springgrid_diagonals = nk_true;
nk_bool springgrid_enable_spring = nk_false;

void SpringGrid_setup(ExampleContext *example) {
    nvRigidBody ***nodes = NV_MALLOC(sizeof(nvRigidBody) * springgrid_rows);
    for (size_t i = 0; i < springgrid_rows; i++) {
        nodes[i] = NV_MALLOC(sizeof(nvRigidBody) * springgrid_cols);
    }

    nv_float dist = 0.75f;

    for (int y = 0; y < springgrid_rows; y++) {
        for (int x = 0; x < springgrid_cols; x++) {
            int cx = springgrid_cols / 2;
            int lx = 4;
            int rx = springgrid_cols % 2 == 0 ? lx - 1 : lx; 

            nvRigidBody *node;
            nvRigidBodyInitializer body_init = nvRigidBodyInitializer_default;
            body_init.type = nvRigidBodyType_DYNAMIC;

            if (y == 0 && x > cx-lx && x < cx+rx)
                body_init.type = nvRigidBodyType_STATIC;

            body_init.position = NV_VECTOR2(
                64.0f - (nv_float)springgrid_cols * (dist * 0.5) + (nv_float)x * dist,
                (nv_float)y * dist
            );
            node = nvRigidBody_new(body_init);

            nvRigidBody_add_shape(node, nvCircleShape_new(nvVector2_zero, 0.25f));

            nvRigidBody_disable_collisions(node);

            nvSpace_add_rigidbody(example->space, node);

            nodes[y][x] = node;
        }
    }

    nvDistanceConstraintInitializer spring_init = nvDistanceConstraintInitializer_default;
    spring_init.spring = springgrid_enable_spring;
    spring_init.hertz = 2.5f;
    spring_init.damping = 0.2f;
    spring_init.anchor_a = nvVector2_zero;
    spring_init.anchor_b = nvVector2_zero;

    nv_float r = 0.25f;
    nvVector2 rr = nvVector2_rotate(NV_VECTOR2(r, 0.0f), NV_PI/4.0f);
    nvVector2 rrp = nvVector2_perp(rr);

    for (size_t y = 0; y < springgrid_rows; y++) {
        for (size_t x = 0; x < springgrid_cols; x++) {
            if (y > 0) {
                spring_init.a = nodes[y][x];
                spring_init.b = nodes[y - 1][x];
                spring_init.anchor_a = NV_VECTOR2(0.0, -r);
                spring_init.anchor_b = NV_VECTOR2(0.0, r);
                spring_init.length = nvVector2_dist(
                    nvVector2_add(nvRigidBody_get_position(spring_init.a), spring_init.anchor_a),
                    nvVector2_add(nvRigidBody_get_position(spring_init.b), spring_init.anchor_b)
                );

                nvConstraint *cons = nvDistanceConstraint_new(spring_init);
                nvSpace_add_constraint(example->space, cons);
            }

            if (x > 0) {
                spring_init.a = nodes[y][x];
                spring_init.b = nodes[y][x - 1];
                spring_init.anchor_a = NV_VECTOR2(-r, 0.0);
                spring_init.anchor_b = NV_VECTOR2(r, 0.0);
                spring_init.length = nvVector2_dist(
                    nvVector2_add(nvRigidBody_get_position(spring_init.a), spring_init.anchor_a),
                    nvVector2_add(nvRigidBody_get_position(spring_init.b), spring_init.anchor_b)
                );

                nvConstraint *cons = nvDistanceConstraint_new(spring_init);
                nvSpace_add_constraint(example->space, cons);
            }

            if (springgrid_diagonals && y > 0 && x > 0) {
                {
                    spring_init.a = nodes[y][x];
                    spring_init.b = nodes[y - 1][x - 1];
                    spring_init.anchor_a = nvVector2_neg(rr);
                    spring_init.anchor_b = rr;
                    spring_init.length = nvVector2_dist(
                        nvVector2_add(nvRigidBody_get_position(spring_init.a), spring_init.anchor_a),
                        nvVector2_add(nvRigidBody_get_position(spring_init.b), spring_init.anchor_b)
                    );

                    nvConstraint *cons = nvDistanceConstraint_new(spring_init);
                    nvSpace_add_constraint(example->space, cons);
                }
                {
                    spring_init.a = nodes[y][x - 1];
                    spring_init.b = nodes[y - 1][x];
                    spring_init.anchor_a = nvVector2_neg(rrp);
                    spring_init.anchor_b = rrp;
                    spring_init.length = nvVector2_dist(
                        nvVector2_add(nvRigidBody_get_position(spring_init.a), spring_init.anchor_a),
                        nvVector2_add(nvRigidBody_get_position(spring_init.b), spring_init.anchor_b)
                    );

                    nvConstraint *cons = nvDistanceConstraint_new(spring_init);
                    nvSpace_add_constraint(example->space, cons);
                }
            }
        }
    }

    for (size_t i = 0; i < springgrid_rows; i++) {
        NV_FREE(nodes[i]);
    }
    NV_FREE(nodes);
}

void SpringGrid_update(ExampleContext *example) {
    char display_buf[8];
    const float ratio[] = {0.25f, 0.62f, 0.13f};
    bool changed = false;

    {
        nk_layout_row(example->ui_ctx, NK_DYNAMIC, 16, 3, ratio);

        nk_label(example->ui_ctx, "Rows", NK_TEXT_LEFT);

        if (nk_slider_int(example->ui_ctx, 1, &springgrid_rows, 100, 1))
            changed = true;
        
        sprintf(display_buf, "%d", springgrid_rows);
        nk_label(example->ui_ctx, display_buf, NK_TEXT_LEFT);
    }
    {
        nk_layout_row(example->ui_ctx, NK_DYNAMIC, 16, 3, ratio);

        nk_label(example->ui_ctx, "Columns", NK_TEXT_LEFT);

        if (nk_slider_int(example->ui_ctx, 1, &springgrid_cols, 100, 1))
            changed = true;
        
        sprintf(display_buf, "%d", springgrid_cols);
        nk_label(example->ui_ctx, display_buf, NK_TEXT_LEFT);
    }
    {
        nk_layout_row_dynamic(example->ui_ctx, 16, 1);
        if (nk_checkbox_label(example->ui_ctx, "Diagonal Links", &springgrid_diagonals))
            changed = true;
    }
    {
        nk_layout_row_dynamic(example->ui_ctx, 16, 1);
        if (nk_checkbox_label(example->ui_ctx, "Stiff <-> Soft", &springgrid_enable_spring))
            changed = true;
    }

    if (changed) {
        ExampleContext_reset(example);
    }
}