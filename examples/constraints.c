/*

  This file is a part of the Nova Physics Engine
  project and distributed under the MIT license.

  Copyright © Kadir Aksoy
  https://github.com/kadir014/nova-physics

*/

#include "example_base.h"


void setup(Example *example) {
    nvBody *ground = nv_Rect_new(
        nv_BodyType_STATIC,
        NV_VEC2(64.0, 72.0),
        0.0,
        nvMaterial_CONCRETE,
        128.0, 5.0
    );
    nvSpace_add(example->space, ground);

    nvBody *wall0 = nv_Rect_new(
        nv_BodyType_STATIC,
        NV_VEC2(128.0 / 3.0, 36.0),
        0.0,
        nvMaterial_CONCRETE,
        0.5, 72.0
    );
    nvSpace_add(example->space, wall0);

    nvBody *wall1 = nv_Rect_new(
        nv_BodyType_STATIC,
        NV_VEC2(128.0 / 3.0 * 2.0, 36.0),
        0.0,
        nvMaterial_CONCRETE,
        0.5, 72.0
    );
    nvSpace_add(example->space, wall1);


    /* Spring Constraint */

    nvBody *circle = nv_Rect_new(
        nv_BodyType_DYNAMIC,
        NV_VEC2(128.0 / 6.0, 17.0),
        0.0,
        nvMaterial_BASIC,
        4.0, 4.0
    );
    nvSpace_add(example->space, circle);

    nvConstraint *spring = nvSpring_new(
        NULL, circle,
        NV_VEC2(128.0 / 6.0, 5.0), NV_VEC2(0.0, -2.0),
        10.0, 100.0, 5.0
    );
    nvSpace_add_constraint(example->space, spring);

    nvBody *bridge0 = nv_Circle_new(
        nv_BodyType_DYNAMIC,
        NV_VEC2(128.0 / 6.0 - 5.0, 36.0),
        0.0,
        nvMaterial_BASIC,
        1.2
    );
    nvSpace_add(example->space, bridge0);

    nvBody *bridge1 = nv_Circle_new(
        nv_BodyType_DYNAMIC,
        NV_VEC2(128.0 / 6.0, 36.0),
        0.0,
        nvMaterial_BASIC,
        1.2
    );
    nvSpace_add(example->space, bridge1);

    nvBody *bridge2 = nv_Circle_new(
        nv_BodyType_DYNAMIC,
        NV_VEC2(128.0 / 6.0 + 5.0, 36.0),
        0.0,
        nvMaterial_BASIC,
        1.2
    );
    nvSpace_add(example->space, bridge2);

    nvConstraint *bridge_spring0 = nvSpring_new(
        NULL, bridge0,
        NV_VEC2(128.0 / 6.0 - 10.0, 36.0), NV_VEC2(0.0, 0.0),
        6.0, 300.0, 20.0
    );
    nvSpace_add_constraint(example->space, bridge_spring0);

    nvConstraint *bridge_spring1 = nvSpring_new(
        bridge0, bridge1,
        NV_VEC2(0.0, 0.0), NV_VEC2(0.0, 0.0),
        6.0, 300.0, 20.0
    );
    nvSpace_add_constraint(example->space, bridge_spring1);

    nvConstraint *bridge_spring2 = nvSpring_new(
        bridge1, bridge2,
        NV_VEC2(0.0, 0.0), NV_VEC2(0.0, 0.0),
        6.0, 300.0, 20.0
    );
    nvSpace_add_constraint(example->space, bridge_spring2);

    nvConstraint *bridge_spring3 = nvSpring_new(
        NULL, bridge2,
        NV_VEC2(128.0 / 6.0 + 10.0, 36.0), NV_VEC2(0.0, 0.0),
        6.0, 300.0, 20.0
    );
    nvSpace_add_constraint(example->space, bridge_spring3);

    nvBody *triangle = nvBody_new(
        nv_BodyType_DYNAMIC,
        nvShapeFactory_NGon(3, 5.5),
        NV_VEC2(128.0 / 6.0 - 10.0, 60.0),
        0.0,
        nvMaterial_BASIC
    );
    nvSpace_add(example->space, triangle);

    nvBody *hexagon = nvBody_new(
        nv_BodyType_DYNAMIC,
        nvShapeFactory_NGon(6, 5.5),
        NV_VEC2(128.0 / 6.0 + 10.0, 60.0),
        0.0,
        nvMaterial_BASIC
    );
    nvSpace_add(example->space, hexagon);

    nvConstraint *shape_spring = nvSpring_new(
        triangle, hexagon,
        NV_VEC2(1.7, 0.0), NV_VEC2(-1.7, 0.0),
        10.0, 200.0, 25.0
    );
    nvSpace_add_constraint(example->space, shape_spring);


    /* Distance Joint Constraint */

    circle = nv_Rect_new(
        nv_BodyType_DYNAMIC,
        NV_VEC2(128.0 / 2.0, 17.0),
        0.0,
        nvMaterial_BASIC,
        4.0, 4.0
    );
    nvSpace_add(example->space, circle);

    nvConstraint *dist_joint = nvDistanceJoint_new(
        NULL, circle,
        NV_VEC2(128.0 / 2.0, 5.0), NV_VEC2(0.0, -2.0),
        10.0
    );
    nvSpace_add_constraint(example->space, dist_joint);

    bridge0 = nv_Circle_new(
        nv_BodyType_DYNAMIC,
        NV_VEC2(128.0 / 2.0 - 5.0, 36.0),
        0.0,
        nvMaterial_BASIC,
        1.2
    );
    nvSpace_add(example->space, bridge0);

    bridge1 = nv_Circle_new(
        nv_BodyType_DYNAMIC,
        NV_VEC2(128.0 / 2.0, 36.0),
        0.0,
        nvMaterial_BASIC,
        1.2
    );
    nvSpace_add(example->space, bridge1);

    bridge2 = nv_Circle_new(
        nv_BodyType_DYNAMIC,
        NV_VEC2(128.0 / 2.0 + 5.0, 36.0),
        0.0,
        nvMaterial_BASIC,
        1.2
    );
    nvSpace_add(example->space, bridge2);

    nvConstraint *bridge_dist0 = nvDistanceJoint_new(
        NULL, bridge0,
        NV_VEC2(128.0 / 2.0 - 10.0, 36.0), NV_VEC2(0.0, 0.0),
        6.0
    );
    nvSpace_add_constraint(example->space, bridge_dist0);

    nvConstraint *bridge_dist1 = nvDistanceJoint_new(
        bridge0, bridge1,
        NV_VEC2(0.0, 0.0), NV_VEC2(0.0, 0.0),
        6.0
    );
    nvSpace_add_constraint(example->space, bridge_dist1);

    nvConstraint *bridge_dist2 = nvDistanceJoint_new(
        bridge1, bridge2,
        NV_VEC2(0.0, 0.0), NV_VEC2(0.0, 0.0),
        6.0
    );
    nvSpace_add_constraint(example->space, bridge_dist2);

    nvConstraint *bridge_dist3 = nvDistanceJoint_new(
        NULL, bridge2,
        NV_VEC2(128.0 / 2.0 + 10.0, 36.0), NV_VEC2(0.0, 0.0),
        6.0
    );
    nvSpace_add_constraint(example->space, bridge_dist3);

    triangle = nvBody_new(
        nv_BodyType_DYNAMIC,
        nvShapeFactory_NGon(3, 5.5),
        NV_VEC2(128.0 / 2.0 - 10.0, 60.0),
        0.0,
        nvMaterial_BASIC
    );
    nvSpace_add(example->space, triangle);

    hexagon = nvBody_new(
        nv_BodyType_DYNAMIC,
        nvShapeFactory_NGon(6, 5.5),
        NV_VEC2(128.0 / 2.0 + 10.0, 60.0),
        0.0,
        nvMaterial_BASIC
    );
    nvSpace_add(example->space, hexagon);

    nvConstraint *shape_dist = nvDistanceJoint_new(
        triangle, hexagon,
        NV_VEC2(1.7, 0.0), NV_VEC2(-1.7, 0.0),
        10.0
    );
    nvSpace_add_constraint(example->space, shape_dist);


    /* Hinge Joint Constraint */

    circle = nv_Rect_new(
        nv_BodyType_DYNAMIC,
        NV_VEC2(128.0 / 1.2, 17.0),
        0.0,
        nvMaterial_BASIC,
        4.0, 4.0
    );
    nvSpace_add(example->space, circle);

    nvConstraint *hinge_joint = nvHingeJoint_new(
        NULL, circle,
        NV_VEC2(128.0 / 1.2, 5.0)
    );
    nvSpace_add_constraint(example->space, hinge_joint);

    bridge0 = nv_Circle_new(
        nv_BodyType_DYNAMIC,
        NV_VEC2(128.0 / 1.2 - 2.4, 36.0),
        0.0,
        nvMaterial_BASIC,
        1.2
    );
    nvSpace_add(example->space, bridge0);

    bridge1 = nv_Circle_new(
        nv_BodyType_DYNAMIC,
        NV_VEC2(128.0 / 1.2, 36.0),
        0.0,
        nvMaterial_BASIC,
        1.2
    );
    nvSpace_add(example->space, bridge1);

    bridge2 = nv_Circle_new(
        nv_BodyType_DYNAMIC,
        NV_VEC2(128.0 / 1.2 + 2.4, 36.0),
        0.0,
        nvMaterial_BASIC,
        1.2
    );
    nvSpace_add(example->space, bridge2);

    nvBody *bridge3 = nv_Circle_new(
        nv_BodyType_DYNAMIC,
        NV_VEC2(128.0 / 1.2 - 1.2*4.0, 36.0),
        0.0,
        nvMaterial_BASIC,
        1.2
    );
    nvSpace_add(example->space, bridge3);

    nvBody *bridge4 = nv_Circle_new(
        nv_BodyType_DYNAMIC,
        NV_VEC2(128.0 / 1.2 + 1.2*4.0, 36.0),
        0.0,
        nvMaterial_BASIC,
        1.2
    );
    nvSpace_add(example->space, bridge4);

    nvConstraint *bridge_hingea = nvHingeJoint_new(
        NULL, bridge3,
        NV_VEC2(128.0 / 1.2 - 1.2 * 5.0, 36.0)
    );
    nvSpace_add_constraint(example->space, bridge_hingea);

    nvConstraint *bridge_hinge0 = nvHingeJoint_new(
        bridge3, bridge0,
        NV_VEC2(128.0 / 1.2 - 1.2 * 3.0, 36.0)
    );
    nvSpace_add_constraint(example->space, bridge_hinge0);

    nvConstraint *bridge_hinge1 = nvHingeJoint_new(
        bridge0, bridge1,
        NV_VEC2(128.0 / 1.2 - 1.2 * 1.0, 36.0)
    );
    nvSpace_add_constraint(example->space, bridge_hinge1);

    nvConstraint *bridge_hinge2 = nvHingeJoint_new(
        bridge1, bridge2,
        NV_VEC2(128.0 / 1.2 + 1.2 * 1.0, 36.0)
    );
    nvSpace_add_constraint(example->space, bridge_hinge2);

    nvConstraint *bridge_hinge3 = nvHingeJoint_new(
        bridge4, bridge2,
        NV_VEC2(128.0 / 1.2 + 1.2 * 3.0, 36.0)
    );
    nvSpace_add_constraint(example->space, bridge_hinge3);

    nvConstraint *bridge_hingeb = nvHingeJoint_new(
        bridge4, NULL,
        NV_VEC2(128.0 / 1.2 + 1.2 * 5.0, 36.0)
    );
    nvSpace_add_constraint(example->space, bridge_hingeb);

    triangle = nvBody_new(
        nv_BodyType_DYNAMIC,
        nvShapeFactory_NGon(3, 5.5),
        NV_VEC2(128.0 / 1.2 - 10.0, 60.0),
        0.0,
        nvMaterial_BASIC
    );
    nvSpace_add(example->space, triangle);

    hexagon = nvBody_new(
        nv_BodyType_DYNAMIC,
        nvShapeFactory_NGon(6, 5.5),
        NV_VEC2(128.0 / 1.2 + 10.0, 60.0),
        0.0,
        nvMaterial_BASIC
    );
    nvSpace_add(example->space, hexagon);

    nvConstraint *shape_hinge = nvHingeJoint_new(
        triangle, hexagon,
        NV_VEC2(128.0 / 1.2, 60.0)
    );
    nvSpace_add_constraint(example->space, shape_hinge);
}


int main(int argc, char *argv[]) {
    // Create example
    Example *example = Example_new(
        1280, 720,
        "Nova Physics  -  Constraints Example",
        165.0,
        1.0/60.0,
        ExampleTheme_DARK
    );

    // Set callbacks
    example->setup_callback = setup;

    // Run the example
    Example_run(example);

    // Free the space allocated by example
    Example_free(example);

    return 0;
}