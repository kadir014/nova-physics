/*

  This file is a part of the Nova Physics Engine
  project and distributed under the MIT license.

  Copyright © Kadir Aksoy
  https://github.com/kadir014/nova-physics

*/

#include "example.h"


void FountainExample_setup(Example *example) {
    nvSpace *space = example->space;

    // Create ground & walls

    nv_float offset = 0.5;

    nvBody *ground = nvBody_new(
        nvBodyType_STATIC,
        nvRectShape_new(128.0, 5.0),
        NV_VEC2(64.0, 72.0 + 2.5 - offset),
        0.0,
        nvMaterial_CONCRETE
    );

    nvSpace_add(space, ground);

    nvBody *ceiling = nvBody_new(
        nvBodyType_STATIC,
        nvRectShape_new(128.0, 5.0),
        NV_VEC2(64.0, 0 - 2.5 + offset),
        0.0,
        nvMaterial_CONCRETE
    );

    nvSpace_add(space, ceiling);

    nvBody *walll = nvBody_new(
        nvBodyType_STATIC,
        nvRectShape_new(5.0, 72.0),
        NV_VEC2(0.0 - 2.5 + offset, 36.0),
        0.0,
        nvMaterial_CONCRETE
    );

    nvSpace_add(space, walll);

    nvBody *wallr = nvBody_new(
        nvBodyType_STATIC,
        nvRectShape_new(5.0, 72.0),
        NV_VEC2(128.0 + 2.5 - offset, 36.0),
        0.0,
        nvMaterial_CONCRETE
    );

    nvSpace_add(space, wallr);

    if (space->broadphase_algorithm == nvBroadPhaseAlg_SPATIAL_HASH_GRID) {
        // The boundary can't be divided by 3.0 so some walls are left outside the SHG
        // To solve this just make SHG boundaries slightly bigger 
        nvAABB bounds = {0.0, 0.0, 129.0, 75.0};
        nvSpace_set_SHG(space, bounds, 3.0, 3.0);
    }
}


void FountainExample_update(Example *example) {
    nvSpace *space = example->space;

    if (space->bodies->size > get_slider_setting("Max bodies")) return;

    if (example->counter <= get_slider_setting("Spawn rate")) return;
    else example->counter = 0;

    nvMaterial basic_material = {
        .density = 1.0,
        .restitution = 0.1,
        .friction = 0.1
    };

    nvBody *body;
    nv_float n = 4;
    nv_float size = 2.5;

    for (size_t x = 0; x < n; x++) {

        int r = ((space->bodies->size % 7) + x) % 4;

        // Circle
        if (r == 0) {
            body = nvBody_new(
                nvBodyType_DYNAMIC,
                nvCircleShape_new(size / 2.0 + 0.03),
                NV_VEC2(
                    64.0 - (n * size) / 2.0 + size / 2.0 + size * x,
                    10.0
                ),
                0.0,
                basic_material
            );
        }
        // Box
        else if (r == 1) {
            body = nvBody_new(
                nvBodyType_DYNAMIC,
                nvRectShape_new(size, size),
                NV_VEC2(
                    64.0 - (n * size) / 2.0 + size / 2.0 + size* x,
                    10.0
                ),
                0.0,
                basic_material
            );
        }
        // Pentagon
        else if (r == 2) {
            body = nvBody_new(
                nvBodyType_DYNAMIC,
                nvNGonShape_new(6, size),
                NV_VEC2(
                    64.0 - (n * size) / 2.0 + size / 2.0 + size * x,
                    10.0
                ),
                0.0,
                basic_material
            );
        }
        // Triangle
        else if (r == 3) {
            body = nvBody_new(
                nvBodyType_DYNAMIC,
                nvNGonShape_new(3, size),
                NV_VEC2(
                    64.0 - (n * size) / 2.0 + size / 2.0 + size * x,
                    10.0
                ),
                0.0,
                basic_material
            );
        }

        // Have all bodies have the same mass and inertia
        nvBody_set_mass(body, 3.5);

        nvSpace_add(space, body);

        // Apply downward force
        nv_float strength = 10.0 * 1e3 / 1.0;
        nvBody_apply_force(body, NV_VEC2(0.0, strength));
    }
}


void FountainExample_init(ExampleEntry *entry) {
    add_slider_setting(entry, "Max bodies", SliderType_INTEGER, 1500, 500, 2000);
    add_slider_setting(entry, "Spawn rate", SliderType_INTEGER, 5, 1, 10);
}