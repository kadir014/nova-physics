/*

  This file is a part of the Nova Physics Engine
  project and distributed under the MIT license.

  Copyright © Kadir Aksoy
  https://github.com/kadir014/nova-physics

*/

#include "novaphysics/distance_joint.h"
#include "novaphysics/space.h"


/**
 * @file distance_joint.c
 * 
 * @brief Distance joint implementation.
 */


nvConstraint *nvDistanceJoint_new(
    nvBody *a,
    nvBody *b,
    nvVector2 anchor_a,
    nvVector2 anchor_b,
    nv_float length
) {
    nvConstraint *cons = NV_NEW(nvConstraint);
    if (!cons) return NULL;

    cons->a = a;
    cons->b = b;
    cons->type = nvConstraintType_DISTANCEJOINT;

    cons->def = (void *)NV_NEW(nvDistanceJoint);
    if (!cons->def) return NULL;
    nvDistanceJoint *dist_joint = (nvDistanceJoint *)cons->def;

    dist_joint->length = length;
    dist_joint->anchor_a = anchor_a;
    dist_joint->anchor_b = anchor_b;

    dist_joint->ra = nvVector2_zero;
    dist_joint->rb = nvVector2_zero;
    dist_joint->normal = nvVector2_zero;
    dist_joint->bias = 0.0;
    dist_joint->mass = 0.0;
    dist_joint->jc = 0.0;

    return cons;
}

void nv_presolve_distance_joint(
    nvSpace *space,
    nvConstraint *cons,
    nv_float inv_dt
) {
    nvDistanceJoint *dist_joint = (nvDistanceJoint *)cons->def;
    nvBody *a = cons->a;
    nvBody *b = cons->b;

    // Transform anchor points
    nvVector2 rpa, rpb;
    nv_float invmass_a, invmass_b, invinertia_a, invinertia_b;

    if (a == NULL) {
        dist_joint->ra = nvVector2_zero;
        rpa = dist_joint->anchor_a;
        invmass_a = invinertia_a = 0.0;
    } else {
        dist_joint->ra = nvVector2_rotate(dist_joint->anchor_a, a->angle);
        rpa = nvVector2_add(dist_joint->ra, a->position);
        invmass_a = a->invmass;
        invinertia_a = a->invinertia;
    }

    if (b == NULL) {
        dist_joint->rb = nvVector2_zero;
        rpb = dist_joint->anchor_b;
        invmass_b = invinertia_b = 0.0;
    } else {
        dist_joint->rb = nvVector2_rotate(dist_joint->anchor_b, b->angle);
        rpb = nvVector2_add(dist_joint->rb, b->position);
        invmass_b = b->invmass;
        invinertia_b = b->invinertia;
    }

    nvVector2 delta = nvVector2_sub(rpb, rpa);
    dist_joint->normal = nvVector2_normalize(delta);
    nv_float offset = nvVector2_len(delta) - dist_joint->length;

    // Baumgarte position correction bias
    dist_joint->bias = -space->baumgarte * inv_dt * offset;

    // Constraint effective mass
    dist_joint->mass = 1.0 / nv_calc_mass_k(
        dist_joint->normal,
        dist_joint->ra, dist_joint->rb,
        invmass_a, invmass_b,
        invinertia_a, invinertia_b
    );

    if (space->warmstarting) {
        nvVector2 impulse = nvVector2_mul(dist_joint->normal, dist_joint->jc);

        if (a) nvBody_apply_impulse(cons->a, nvVector2_neg(impulse), dist_joint->ra);
        if (b) nvBody_apply_impulse(cons->b, impulse, dist_joint->rb);
    }
}

void nv_solve_distance_joint(nvConstraint *cons) {
    nvDistanceJoint *dist_joint = (nvDistanceJoint *)cons->def;
    nvBody *a = cons->a;
    nvBody *b = cons->b;

    nvVector2 linear_velocity_a, linear_velocity_b;
    nv_float angular_velocity_a, angular_velocity_b;

    if (a == NULL) {
        linear_velocity_a = nvVector2_zero;
        angular_velocity_a = 0.0;
    } else {
        linear_velocity_a = a->linear_velocity;
        angular_velocity_a = a->angular_velocity;
    }

    if (b == NULL) {
        linear_velocity_b = nvVector2_zero;
        angular_velocity_b = 0.0;
    } else {
        linear_velocity_b = b->linear_velocity;
        angular_velocity_b = b->angular_velocity;
    }

    nvVector2 rv = nv_calc_relative_velocity(
        linear_velocity_a, angular_velocity_a, dist_joint->ra,
        linear_velocity_b, angular_velocity_b, dist_joint->rb
    );

    nv_float rn = nvVector2_dot(rv, dist_joint->normal);

    // Normal constraint lambda (impulse magnitude)
    nv_float jc = (dist_joint->bias - rn) * dist_joint->mass;

    // Accumulate impulse
    nv_float jc_max = NV_INF;//5000 * (1.0 / 60.0);

    nv_float jc0 = dist_joint->jc;
    dist_joint->jc = nv_fclamp(jc0 + jc, -jc_max, jc_max);
    jc = dist_joint->jc - jc0;

    nvVector2 impulse = nvVector2_mul(dist_joint->normal, jc);

    // Apply constraint impulse
    if (a != NULL) nvBody_apply_impulse(a, nvVector2_neg(impulse), dist_joint->ra);
    if (b != NULL) nvBody_apply_impulse(b, impulse, dist_joint->rb);
}