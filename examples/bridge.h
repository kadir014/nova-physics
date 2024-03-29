/*

  This file is a part of the Nova Physics Engine
  project and distributed under the MIT license.

  Copyright © Kadir Aksoy
  https://github.com/kadir014/nova-physics

*/

#include "example.h"


void BridgeExample_setup(Example *example) {
    nvSpace *space = example->space;
    
    // Create grounds & bridge

    nvBody *ground_left = nvBody_new(
        nvBodyType_STATIC,
        nvRectShape_new(30.0, 40.0),
        NV_VEC2(10.0, 52.5),
        0.0,
        nvMaterial_CONCRETE
    );

    nvSpace_add(space, ground_left);

    int n = 17; // Parts of the bridge
    double width = 78.0 / (double)n; // Width of one part of the bridge
    double w2 = width / 2.0;

    for (size_t i = 0; i < n; i++) {
        nvBody *bridge_part = nvBody_new(
            nvBodyType_DYNAMIC,
            nvRectShape_new(width, 2.0),
            NV_VEC2(25.0 + w2 + i * width, 33.0),
            0.0,
            nvMaterial_CONCRETE
        );

        nvSpace_add(space, bridge_part);
    }

    nvBody *ground_right = nvBody_new(
        nvBodyType_STATIC,
        nvRectShape_new(30.0, 40.0),
        NV_VEC2(118.0, 52.5),
        0.0,
        nvMaterial_CONCRETE
    );

    nvSpace_add(space, ground_right);


    // Link bridge parts with distance joint constraints
    for (size_t i = 1; i < n + 2; i++) {
        nvVector2 anchor_a;
        nvVector2 anchor_b;

        // Offset anchors by a tiny amount so they don't intersect
        double offset = w2 / 3.0;
        
        // Link to left ground
        if (i == 1) {
            anchor_a = NV_VEC2(15.0, -20.0);
            anchor_b = NV_VEC2(-w2, 0.0);
            offset /= 2.0;
        }
        // Link to right ground
        else if (i == n + 1) {
            anchor_a = NV_VEC2(w2, 0.0);
            anchor_b = NV_VEC2(-15, -20.0);
            offset /= 2.0;
        }
        // Link between bridge parts
        else {
            anchor_a = NV_VEC2(w2 - offset, 0.0);
            anchor_b = NV_VEC2(-w2 + offset, 0.0);
        }

        nvBody *a = (nvBody *)space->bodies->data[i];
        nvBody *b = (nvBody *)space->bodies->data[i + 1];

        nvConstraint *link;

        // Link with a spring to the grounds
        if (i == 1 || i == n + 1) {
            link = nvSpring_new(
                a, b,
                anchor_a, anchor_b,
                offset,
                10000.0,
                500.0
            );
        }
        else {
            link = nvDistanceJoint_new(
                a, b,
                anchor_a, anchor_b,
                offset * 2.0 + 0.25
            );
        }

        nvSpace_add_constraint(space, link);
    }

    //Create boxes on top of the bridge
    for (size_t y = 0; y < 8; y++) {
        for (size_t x = 0; x < 8; x++) {
            nvBody *box = nvBody_new(
                nvBodyType_DYNAMIC,
                nvRectShape_new(2.0, 2.0),
                NV_VEC2(64.0 + x * 2.0 - ((2.0 * 8.0) / 2.0), 10.0 + y * 2.0),
                0.0,
                nvMaterial_WOOD
            );

            nvSpace_add(space, box);
        }
    }
}