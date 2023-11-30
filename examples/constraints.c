/*

  This file is a part of the Nova Physics Engine
  project and distributed under the MIT license.

  Copyright © Kadir Aksoy
  https://github.com/kadir014/nova-physics

*/

#include "example_base.h"


void setup(Example *example) {
    nv_Body *ground = nv_Rect_new(
        nv_BodyType_STATIC,
        NV_VEC2(64.0, 72.0),
        0.0,
        nv_Material_CONCRETE,
        128.0, 5.0
    );
    nv_Space_add(example->space, ground);

    nv_Body *wall0 = nv_Rect_new(
        nv_BodyType_STATIC,
        NV_VEC2(128.0 / 3.0, 36.0),
        0.0,
        nv_Material_CONCRETE,
        0.5, 72.0
    );
    nv_Space_add(example->space, wall0);

    nv_Body *wall1 = nv_Rect_new(
        nv_BodyType_STATIC,
        NV_VEC2(128.0 / 3.0 * 2.0, 36.0),
        0.0,
        nv_Material_CONCRETE,
        0.5, 72.0
    );
    nv_Space_add(example->space, wall1);


    /* Spring Constraint */

    nv_Body *circle = nv_Rect_new(
        nv_BodyType_DYNAMIC,
        NV_VEC2(128.0 / 6.0, 17.0),
        0.0,
        nv_Material_BASIC,
        4.0, 4.0
    );
    nv_Space_add(example->space, circle);

    nv_Constraint *spring = nv_Spring_new(
        NULL, circle,
        NV_VEC2(128.0 / 6.0, 5.0), NV_VEC2(0.0, -2.0),
        10.0, 100.0, 5.0
    );
    nv_Space_add_constraint(example->space, spring);

    nv_Body *bridge0 = nv_Circle_new(
        nv_BodyType_DYNAMIC,
        NV_VEC2(128.0 / 6.0 - 5.0, 36.0),
        0.0,
        nv_Material_BASIC,
        1.2
    );
    nv_Space_add(example->space, bridge0);

    nv_Body *bridge1 = nv_Circle_new(
        nv_BodyType_DYNAMIC,
        NV_VEC2(128.0 / 6.0, 36.0),
        0.0,
        nv_Material_BASIC,
        1.2
    );
    nv_Space_add(example->space, bridge1);

    nv_Body *bridge2 = nv_Circle_new(
        nv_BodyType_DYNAMIC,
        NV_VEC2(128.0 / 6.0 + 5.0, 36.0),
        0.0,
        nv_Material_BASIC,
        1.2
    );
    nv_Space_add(example->space, bridge2);

    nv_Constraint *bridge_spring0 = nv_Spring_new(
        NULL, bridge0,
        NV_VEC2(128.0 / 6.0 - 10.0, 36.0), NV_VEC2(0.0, 0.0),
        6.0, 300.0, 20.0
    );
    nv_Space_add_constraint(example->space, bridge_spring0);

    nv_Constraint *bridge_spring1 = nv_Spring_new(
        bridge0, bridge1,
        NV_VEC2(0.0, 0.0), NV_VEC2(0.0, 0.0),
        6.0, 300.0, 20.0
    );
    nv_Space_add_constraint(example->space, bridge_spring1);

    nv_Constraint *bridge_spring2 = nv_Spring_new(
        bridge1, bridge2,
        NV_VEC2(0.0, 0.0), NV_VEC2(0.0, 0.0),
        6.0, 300.0, 20.0
    );
    nv_Space_add_constraint(example->space, bridge_spring2);

    nv_Constraint *bridge_spring3 = nv_Spring_new(
        NULL, bridge2,
        NV_VEC2(128.0 / 6.0 + 10.0, 36.0), NV_VEC2(0.0, 0.0),
        6.0, 300.0, 20.0
    );
    nv_Space_add_constraint(example->space, bridge_spring3);

    nv_Body *triangle = nv_Body_new(
        nv_BodyType_DYNAMIC,
        nv_ShapeFactory_NGon(3, 5.5),
        NV_VEC2(128.0 / 6.0 - 10.0, 60.0),
        0.0,
        nv_Material_BASIC
    );
    nv_Space_add(example->space, triangle);

    nv_Body *hexagon = nv_Body_new(
        nv_BodyType_DYNAMIC,
        nv_ShapeFactory_NGon(6, 5.5),
        NV_VEC2(128.0 / 6.0 + 10.0, 60.0),
        0.0,
        nv_Material_BASIC
    );
    nv_Space_add(example->space, hexagon);

    nv_Constraint *shape_spring = nv_Spring_new(
        triangle, hexagon,
        NV_VEC2(1.7, 0.0), NV_VEC2(-1.7, 0.0),
        10.0, 200.0, 25.0
    );
    nv_Space_add_constraint(example->space, shape_spring);


    /* Distance Joint Constraint */

    circle = nv_Rect_new(
        nv_BodyType_DYNAMIC,
        NV_VEC2(128.0 / 2.0, 17.0),
        0.0,
        nv_Material_BASIC,
        4.0, 4.0
    );
    nv_Space_add(example->space, circle);

    nv_Constraint *dist_joint = nv_DistanceJoint_new(
        NULL, circle,
        NV_VEC2(128.0 / 2.0, 5.0), NV_VEC2(0.0, -2.0),
        10.0
    );
    nv_Space_add_constraint(example->space, dist_joint);

    bridge0 = nv_Circle_new(
        nv_BodyType_DYNAMIC,
        NV_VEC2(128.0 / 2.0 - 5.0, 36.0),
        0.0,
        nv_Material_BASIC,
        1.2
    );
    nv_Space_add(example->space, bridge0);

    bridge1 = nv_Circle_new(
        nv_BodyType_DYNAMIC,
        NV_VEC2(128.0 / 2.0, 36.0),
        0.0,
        nv_Material_BASIC,
        1.2
    );
    nv_Space_add(example->space, bridge1);

    bridge2 = nv_Circle_new(
        nv_BodyType_DYNAMIC,
        NV_VEC2(128.0 / 2.0 + 5.0, 36.0),
        0.0,
        nv_Material_BASIC,
        1.2
    );
    nv_Space_add(example->space, bridge2);

    nv_Constraint *bridge_dist0 = nv_DistanceJoint_new(
        NULL, bridge0,
        NV_VEC2(128.0 / 2.0 - 10.0, 36.0), NV_VEC2(0.0, 0.0),
        6.0
    );
    nv_Space_add_constraint(example->space, bridge_dist0);

    nv_Constraint *bridge_dist1 = nv_DistanceJoint_new(
        bridge0, bridge1,
        NV_VEC2(0.0, 0.0), NV_VEC2(0.0, 0.0),
        6.0
    );
    nv_Space_add_constraint(example->space, bridge_dist1);

    nv_Constraint *bridge_dist2 = nv_DistanceJoint_new(
        bridge1, bridge2,
        NV_VEC2(0.0, 0.0), NV_VEC2(0.0, 0.0),
        6.0
    );
    nv_Space_add_constraint(example->space, bridge_dist2);

    nv_Constraint *bridge_dist3 = nv_DistanceJoint_new(
        NULL, bridge2,
        NV_VEC2(128.0 / 2.0 + 10.0, 36.0), NV_VEC2(0.0, 0.0),
        6.0
    );
    nv_Space_add_constraint(example->space, bridge_dist3);

    triangle = nv_Body_new(
        nv_BodyType_DYNAMIC,
        nv_ShapeFactory_NGon(3, 5.5),
        NV_VEC2(128.0 / 2.0 - 10.0, 60.0),
        0.0,
        nv_Material_BASIC
    );
    nv_Space_add(example->space, triangle);

    hexagon = nv_Body_new(
        nv_BodyType_DYNAMIC,
        nv_ShapeFactory_NGon(6, 5.5),
        NV_VEC2(128.0 / 2.0 + 10.0, 60.0),
        0.0,
        nv_Material_BASIC
    );
    nv_Space_add(example->space, hexagon);

    nv_Constraint *shape_dist = nv_DistanceJoint_new(
        triangle, hexagon,
        NV_VEC2(1.7, 0.0), NV_VEC2(-1.7, 0.0),
        10.0
    );
    nv_Space_add_constraint(example->space, shape_dist);


    /* Hinge Joint Constraint */

    circle = nv_Rect_new(
        nv_BodyType_DYNAMIC,
        NV_VEC2(128.0 / 1.2, 17.0),
        0.0,
        nv_Material_BASIC,
        4.0, 4.0
    );
    nv_Space_add(example->space, circle);

    nv_Constraint *hinge_joint = nv_HingeJoint_new(
        NULL, circle,
        NV_VEC2(128.0 / 1.2, 5.0)
    );
    nv_Space_add_constraint(example->space, hinge_joint);

    bridge0 = nv_Circle_new(
        nv_BodyType_DYNAMIC,
        NV_VEC2(128.0 / 1.2 - 2.4, 36.0),
        0.0,
        nv_Material_BASIC,
        1.2
    );
    nv_Space_add(example->space, bridge0);

    bridge1 = nv_Circle_new(
        nv_BodyType_DYNAMIC,
        NV_VEC2(128.0 / 1.2, 36.0),
        0.0,
        nv_Material_BASIC,
        1.2
    );
    nv_Space_add(example->space, bridge1);

    bridge2 = nv_Circle_new(
        nv_BodyType_DYNAMIC,
        NV_VEC2(128.0 / 1.2 + 2.4, 36.0),
        0.0,
        nv_Material_BASIC,
        1.2
    );
    nv_Space_add(example->space, bridge2);

    nv_Body *bridge3 = nv_Circle_new(
        nv_BodyType_DYNAMIC,
        NV_VEC2(128.0 / 1.2 - 1.2*4.0, 36.0),
        0.0,
        nv_Material_BASIC,
        1.2
    );
    nv_Space_add(example->space, bridge3);

    nv_Body *bridge4 = nv_Circle_new(
        nv_BodyType_DYNAMIC,
        NV_VEC2(128.0 / 1.2 + 1.2*4.0, 36.0),
        0.0,
        nv_Material_BASIC,
        1.2
    );
    nv_Space_add(example->space, bridge4);

    nv_Constraint *bridge_hingea = nv_HingeJoint_new(
        NULL, bridge3,
        NV_VEC2(128.0 / 1.2 - 1.2 * 5.0, 36.0)
    );
    nv_Space_add_constraint(example->space, bridge_hingea);

    nv_Constraint *bridge_hinge0 = nv_HingeJoint_new(
        bridge3, bridge0,
        NV_VEC2(128.0 / 1.2 - 1.2 * 3.0, 36.0)
    );
    nv_Space_add_constraint(example->space, bridge_hinge0);

    nv_Constraint *bridge_hinge1 = nv_HingeJoint_new(
        bridge0, bridge1,
        NV_VEC2(128.0 / 1.2 - 1.2 * 1.0, 36.0)
    );
    nv_Space_add_constraint(example->space, bridge_hinge1);

    nv_Constraint *bridge_hinge2 = nv_HingeJoint_new(
        bridge1, bridge2,
        NV_VEC2(128.0 / 1.2 + 1.2 * 1.0, 36.0)
    );
    nv_Space_add_constraint(example->space, bridge_hinge2);

    nv_Constraint *bridge_hinge3 = nv_HingeJoint_new(
        bridge4, bridge2,
        NV_VEC2(128.0 / 1.2 + 1.2 * 3.0, 36.0)
    );
    nv_Space_add_constraint(example->space, bridge_hinge3);

    nv_Constraint *bridge_hingeb = nv_HingeJoint_new(
        bridge4, NULL,
        NV_VEC2(128.0 / 1.2 + 1.2 * 5.0, 36.0)
    );
    nv_Space_add_constraint(example->space, bridge_hingeb);

    triangle = nv_Body_new(
        nv_BodyType_DYNAMIC,
        nv_ShapeFactory_NGon(3, 5.5),
        NV_VEC2(128.0 / 1.2 - 10.0, 60.0),
        0.0,
        nv_Material_BASIC
    );
    nv_Space_add(example->space, triangle);

    hexagon = nv_Body_new(
        nv_BodyType_DYNAMIC,
        nv_ShapeFactory_NGon(6, 5.5),
        NV_VEC2(128.0 / 1.2 + 10.0, 60.0),
        0.0,
        nv_Material_BASIC
    );
    nv_Space_add(example->space, hexagon);

    nv_Constraint *shape_hinge = nv_HingeJoint_new(
        triangle, hexagon,
        NV_VEC2(128.0 / 1.2, 60.0)
    );
    nv_Space_add_constraint(example->space, shape_hinge);
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