/*

  This file is a part of the Nova Physics Engine
  project and distributed under the MIT license.

  Copyright © Kadir Aksoy
  https://github.com/kadir014/nova-physics

*/

#include "example.h"


void PoolExample_setup(Example *example) {
    nvSpace *space = example->space;
    
    // Create borders of the pool

    nvBody *wall_bottom = nvBody_new(
        nvBodyType_STATIC,
        nvRectShape_new(60.0, 5.0),
        NV_VEC2(64.0, 62.5),
        0.0,
        nvMaterial_CONCRETE
    );

    nvSpace_add(space, wall_bottom);

    nvBody *wall_left = nvBody_new(
        nvBodyType_STATIC,
        nvRectShape_new(5.0, 40.0),
        NV_VEC2(24.0, 47.5),
        -NV_PI / 5.0,
        nvMaterial_CONCRETE
    );

    nvSpace_add(space, wall_left);

    nvBody *wall_right = nvBody_new(
        nvBodyType_STATIC,
        nvRectShape_new(5.0, 40.0),
        NV_VEC2(104.0, 47.5),
        NV_PI / 5.0,
        nvMaterial_CONCRETE
    );

    nvSpace_add(space, wall_right);


    // Add balls

    double radius = 0.7;

    nvMaterial ball_material = {
        .density = 1.0,
        .restitution = 0.0,
        .friction = 0.0
    };

    for (size_t y = 0; y < 18; y++) {
        for (size_t x = 0; x < 30; x++) {

            nvBody *ball = nvBody_new(
                nvBodyType_DYNAMIC,
                nvCircleShape_new(radius),
                NV_VEC2(33.0 + x * (radius * 2.0), 25.8 + y * (radius * 2.0)),
                0.0,
                ball_material
            );

            nvSpace_add(space, ball);
        }
    }


    // Add ship

    // Vertices array will get free'd along with bodies
    // once nvSpace_free is called (or Example_free in this case)
    nvArray *ship_vertices = nvArray_new();
    nvArray_add(ship_vertices, NV_VEC2_NEW(-5.0, -2.0));
    nvArray_add(ship_vertices, NV_VEC2_NEW(5.0, -2.0));
    nvArray_add(ship_vertices, NV_VEC2_NEW(3.0, 2.0));
    nvArray_add(ship_vertices, NV_VEC2_NEW(-3.0, 2.0));

    nvBody *ship = nvBody_new(
        nvBodyType_DYNAMIC,
        nvPolygonShape_new(ship_vertices),
        NV_VEC2(44.0, 15.0),
        0.0,
        nvMaterial_GLASS
    );

    nvSpace_add(space, ship);

    if (space->broadphase_algorithm == nvBroadPhaseAlg_SPATIAL_HASH_GRID)
        nvSpace_set_SHG(space, space->shg->bounds, 1.4, 1.4);
}