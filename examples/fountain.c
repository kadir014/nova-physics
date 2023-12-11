/*

  This file is a part of the Nova Physics Engine
  project and distributed under the MIT license.

  Copyright © Kadir Aksoy
  https://github.com/kadir014/nova-physics

*/

#include "example_base.h"


#define MAX_BODIES 1100


void setup(Example *example) {
    // Create ground & walls

    nv_float offset = 0.5;

    nvBody *ground = nv_Rect_new(
        nvBodyType_STATIC,
        NV_VEC2(64.0, 72.0 + 2.5 - offset),
        0.0,
        nvMaterial_CONCRETE,
        128.0, 5.0
    );

    nvSpace_add(example->space, ground);

    nvBody *ceiling = nv_Rect_new(
        nvBodyType_STATIC,
        NV_VEC2(64.0, 0 - 2.5 + offset),
        0.0,
        nvMaterial_CONCRETE,
        128.0, 5.0
    );

    nvSpace_add(example->space, ceiling);

    nvBody *walll = nv_Rect_new(
        nvBodyType_STATIC,
        NV_VEC2(0.0 - 2.5 + offset, 36.0),
        0.0,
        nvMaterial_CONCRETE,
        5.0, 72.0
    );

    nvSpace_add(example->space, walll);

    nvBody *wallr = nv_Rect_new(
        nvBodyType_STATIC,
        NV_VEC2(128.0 + 2.5 - offset, 36.0),
        0.0,
        nvMaterial_CONCRETE,
        5.0, 72.0
    );

    nvSpace_add(example->space, wallr);

    if (example->space->broadphase_algorithm == nvBroadPhaseAlg_SPATIAL_HASH_GRID) {
        // The boundary can't be divided by 3.0 so some walls are left outside the SHG
        // To solve this just make SHG boundaries slightly bigger 
        nvAABB bounds = {0.0, 0.0, 128.0 + 10.0, 72.0 + 10.0};
        nvSpace_set_SHG(example->space, bounds, 3.0, 3.0);
    }
}


void update(Example *example) {
    if (example->space->bodies->size > MAX_BODIES) return;

    if (example->counter < 4) return;
    else example->counter = 0;

    nvMaterial basic_material = {
        .density = 1.0,
        .restitution = 0.1,
        .friction = 0.1
    };

    nvBody *body;
    nv_float n = 8;
    nv_float size = 2.2;

    for (size_t x = 0; x < n; x++) {

        int r = x % 4;

        // Circle
        if (r == 0) {
            body = nv_Circle_new(
                nvBodyType_DYNAMIC,
                NV_VEC2(
                    64.0 - (n * size) / 2.0 + size / 2.0 + size * x,
                    10.0
                ),
                0.0,
                basic_material,
                size / 2.0 + 0.03
            );
        }
        // Box
        else if (r == 1) {
            body = nv_Rect_new(
                nvBodyType_DYNAMIC,
                NV_VEC2(
                    64.0 - (n * size) / 2.0 + size / 2.0 + size* x,
                    10.0
                ),
                0.0,
                basic_material,
                size, size
            );
        }
        // Pentagon
        else if (r == 2) {
            body = nvBody_new(
                nvBodyType_DYNAMIC,
                nvShapeFactory_NGon(6, size),
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
                nvShapeFactory_NGon(3, size),
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

        nvSpace_add(example->space, body);

        // Apply downward force
        nv_float strength = 10.0 * 1e3 / 1.0;
        nvBody_apply_force(body, NV_VEC2(0.0, strength));
    }
}


int main(int argc, char *argv[]) {
    // Create example
    Example *example = Example_new(
        1280, 720,
        "Nova Physics  -  Fountain Example",
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