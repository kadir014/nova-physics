/*

  This file is a part of the Nova Physics Engine
  project and distributed under the MIT license.

  Copyright © Kadir Aksoy
  https://github.com/kadir014/nova-physics

*/

#include "example.h"


void create_ragdoll(nvSpace *space, nvVector2 position, nv_float scale, nv_uint32 group) {
    nvBody *torso = nvBody_new(
        nvBodyType_DYNAMIC,
        nvRectShape_new((1.5 * scale) * 2.0, (2.0 * scale) * 2.0),
        position,
        0.0,
        nvMaterial_BASIC
    );
    nvSpace_add(space, torso);
    torso->collision_group = group;

    nvBody *head = nvBody_new(
        nvBodyType_DYNAMIC,
        nvCircleShape_new(1.0 * scale),
        NV_VEC2(position.x, position.y - 2.0 * scale - 1.0 * scale),
        0.0,
        nvMaterial_BASIC
    );
    nvSpace_add(space, head);
    head->collision_group = group;

    nvConstraint *head_link = nvHingeJoint_new(torso, head, NV_VEC2(position.x, position.y - 2.0 * scale));
    nvHingeJoint *head_link_def = (nvHingeJoint *)head_link->def;
    nvSpace_add_constraint(space, head_link);
    head_link_def->enable_limits = true;
    head_link_def->lower_limit = -NV_PI / 4.0;
    head_link_def->upper_limit = NV_PI / 4.0;

    nvBody *larm1 = nvBody_new(
        nvBodyType_DYNAMIC,
        nvRectShape_new((1.5 * scale) * 2.0, (0.5 * scale) * 2.0),
        NV_VEC2(position.x - 1.5 * scale - 1.0 * scale, position.y - 2.0 * scale + 0.5 * scale),
        0.0,
        nvMaterial_BASIC
    );
    nvSpace_add(space, larm1);
    larm1->collision_group = group;

    nvConstraint *larm1_link = nvHingeJoint_new(torso, larm1, NV_VEC2(position.x - 1.5 * scale, position.y - 2.0 * scale + 0.5 * scale));
    nvHingeJoint *larm1_link_def = (nvHingeJoint *)larm1_link->def;
    nvSpace_add_constraint(space, larm1_link);
    larm1_link_def->enable_limits = true;
    larm1_link_def->lower_limit = -NV_PI / 2.0;
    larm1_link_def->upper_limit = NV_PI / 2.0;

    nvBody *larm2 = nvBody_new(
        nvBodyType_DYNAMIC,
        nvRectShape_new((1.5 * scale) * 2.0, (0.5 * scale) * 2.0),
        NV_VEC2(position.x - 1.5 * scale - 2.0 * scale - 1.0 * scale, position.y - 2.0 * scale + 0.5 * scale),
        0.0,
        nvMaterial_BASIC
    );
    nvSpace_add(space, larm2);
    larm2->collision_group = group;

    nvConstraint *larm2_link = nvHingeJoint_new(larm1, larm2, NV_VEC2(position.x - 1.5 * scale - 2.0 * scale, position.y - 2.0 * scale + 0.5 * scale));
    nvHingeJoint *larm2_link_def = (nvHingeJoint *)larm2_link->def;
    nvSpace_add_constraint(space, larm2_link);
    larm2_link_def->enable_limits = true;
    larm2_link_def->lower_limit = -NV_PI / 2.0;
    larm2_link_def->upper_limit = NV_PI / 2.0;

    nvBody *rarm1 = nvBody_new(
        nvBodyType_DYNAMIC,
        nvRectShape_new((1.5 * scale) * 2.0, (0.5 * scale) * 2.0),
        NV_VEC2(position.x + 1.5 * scale + 1.0 * scale, position.y - 2.0 * scale + 0.5 * scale),
        0.0,
        nvMaterial_BASIC
    );
    nvSpace_add(space, rarm1);
    rarm1->collision_group = group;

    nvConstraint *rarm1_link = nvHingeJoint_new(torso, rarm1, NV_VEC2(position.x + 1.5 * scale, position.y - 2.0 * scale + 0.5 * scale));
    nvHingeJoint *rarm1_link_def = (nvHingeJoint *)rarm1_link->def;
    nvSpace_add_constraint(space, rarm1_link);
    rarm1_link_def->enable_limits = true;
    rarm1_link_def->lower_limit = -NV_PI / 2.0;
    rarm1_link_def->upper_limit = NV_PI / 2.0;

    nvBody *rarm2 = nvBody_new(
        nvBodyType_DYNAMIC,
        nvRectShape_new((1.5 * scale) * 2.0, (0.5 * scale) * 2.0),
        NV_VEC2(position.x + 1.5 * scale + 2.0 * scale + 1.0 * scale, position.y - 2.0 * scale + 0.5 * scale),
        0.0,
        nvMaterial_BASIC
    );
    nvSpace_add(space, rarm2);
    rarm2->collision_group = group;

    nvConstraint *rarm2_link = nvHingeJoint_new(rarm1, rarm2, NV_VEC2(position.x + 1.5 * scale + 2.0 * scale, position.y - 2.0 * scale + 0.5 * scale));
    nvHingeJoint *rarm2_link_def = (nvHingeJoint *)rarm2_link->def;
    nvSpace_add_constraint(space, rarm2_link);
    rarm2_link_def->enable_limits = true;
    rarm2_link_def->lower_limit = -NV_PI / 2.0;
    rarm2_link_def->upper_limit = NV_PI / 2.0;

    nvBody *lleg1 = nvBody_new(
        nvBodyType_DYNAMIC,
        nvRectShape_new((0.5 * scale) * 2.0, (1.5 * scale) * 2.0),
        NV_VEC2(position.x - 1.5 * scale + 0.5 * scale, position.y + 2.0 * scale + 1.0 * scale),
        0.0,
        nvMaterial_BASIC
    );
    nvSpace_add(space, lleg1);
    lleg1->collision_group = group;

    nvConstraint *lleg1_link = nvHingeJoint_new(torso, lleg1, NV_VEC2(position.x - 1.5 * scale + 0.5 * scale, position.y + 2.0 * scale));
    nvHingeJoint *lleg1_link_def = (nvHingeJoint *)lleg1_link->def;
    nvSpace_add_constraint(space, lleg1_link);
    lleg1_link_def->enable_limits = true;
    lleg1_link_def->lower_limit = -NV_PI / 2.0 + 0.3;
    lleg1_link_def->upper_limit = NV_PI / 2.0 - 0.3;

    nvBody *lleg2 = nvBody_new(
        nvBodyType_DYNAMIC,
        nvRectShape_new((0.5 * scale) * 2.0, (1.5 * scale) * 2.0),
        NV_VEC2(position.x - 1.5 * scale + 0.5 * scale, position.y + 2.0 * scale + 2.0 * scale + 1.0 * scale),
        0.0,
        nvMaterial_BASIC
    );
    nvSpace_add(space, lleg2);
    lleg2->collision_group = group;

    nvConstraint *lleg2_link = nvHingeJoint_new(lleg1, lleg2, NV_VEC2(position.x - 1.5 * scale + 0.5 * scale, position.y + 2.0 * scale + 2.0 * scale));
    nvHingeJoint *lleg2_link_def = (nvHingeJoint *)lleg2_link->def;
    nvSpace_add_constraint(space, lleg2_link);
    lleg2_link_def->enable_limits = true;
    lleg2_link_def->lower_limit = -NV_PI / 2.0 + 0.0;
    lleg2_link_def->upper_limit = 0.0;

    nvBody *rleg1 = nvBody_new(
        nvBodyType_DYNAMIC,
        nvRectShape_new((0.5 * scale) * 2.0, (1.5 * scale) * 2.0),
        NV_VEC2(position.x + 1.5 * scale - 0.5 * scale, position.y + 2.0 * scale + 1.0 * scale),
        0.0,
        nvMaterial_BASIC
    );
    nvSpace_add(space, rleg1);
    rleg1->collision_group = group;

    nvConstraint *rleg1_link = nvHingeJoint_new(torso, rleg1, NV_VEC2(position.x + 1.5 * scale - 0.5 * scale, position.y + 2.0 * scale));
    nvHingeJoint *rleg1_link_def = (nvHingeJoint *)rleg1_link->def;
    nvSpace_add_constraint(space, rleg1_link);
    rleg1_link_def->enable_limits = true;
    rleg1_link_def->lower_limit = -NV_PI / 2.0 + 0.3;
    rleg1_link_def->upper_limit = NV_PI / 2.0 - 0.3;

    nvBody *rleg2 = nvBody_new(
        nvBodyType_DYNAMIC,
        nvRectShape_new((0.5 * scale) * 2.0, (1.5 * scale) * 2.0),
        NV_VEC2(position.x + 1.5 * scale - 0.5 * scale, position.y + 2.0 * scale + 2.0 * scale + 1.0 * scale),
        0.0,
        nvMaterial_BASIC
    );
    nvSpace_add(space, rleg2);
    rleg2->collision_group = group;

    nvConstraint *rleg2_link = nvHingeJoint_new(rleg1, rleg2, NV_VEC2(position.x + 1.5 * scale - 0.5 * scale, position.y + 2.0 * scale + 2.0 * scale));
    nvHingeJoint *rleg2_link_def = (nvHingeJoint *)rleg2_link->def;
    nvSpace_add_constraint(space, rleg2_link);
    rleg2_link_def->enable_limits = true;
    rleg2_link_def->lower_limit = 0.0;
    rleg2_link_def->upper_limit = NV_PI / 2.0 - 0.3;
}


void RagdollsExample_setup(Example *example) {
    nvSpace *space = example->space;

    nvBody *ground = nvBody_new(
        nvBodyType_STATIC,
        nvRectShape_new(128.0, 5.0),
        NV_VEC2(64.0, 72.0 - 2.5),
        0.0,
        nvMaterial_CONCRETE
    );

    nvSpace_add(space, ground);

    for (size_t i = 0; i < 100; i++) {
        create_ragdoll(example->space, NV_VEC2(64.0 + frand(-30.0, 30.0), 36.0 + frand(-130.0, 15.0)), 1.0, i + 0.6);
    }

    if (space->broadphase_algorithm == nvBroadPhaseAlg_SHG)
        nvSpace_set_SHG(space, space->shg->bounds, 2.0, 2.0);
}