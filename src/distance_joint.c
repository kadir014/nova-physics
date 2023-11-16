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


nv_Constraint *nv_DistanceJoint_new(
    nv_Body *a,
    nv_Body *b,
    nv_Vector2 anchor_a,
    nv_Vector2 anchor_b,
    nv_float length
) {
    nv_Constraint *cons = NV_NEW(nv_Constraint);
    if (!cons) return NULL;

    cons->a = a;
    cons->b = b;
    cons->type = nv_ConstraintType_DISTANCEJOINT;

    cons->def = (void *)NV_NEW(nv_DistanceJoint);
    if (!cons->def) return NULL;
    nv_DistanceJoint *dist_joint = (nv_DistanceJoint *)cons->def;

    dist_joint->length = length;
    dist_joint->anchor_a = anchor_a;
    dist_joint->anchor_b = anchor_b;

    dist_joint->ra = nv_Vector2_zero;
    dist_joint->rb = nv_Vector2_zero;
    dist_joint->normal = nv_Vector2_zero;
    dist_joint->bias = 0.0;
    dist_joint->mass = 0.0;
    dist_joint->jc = 0.0;

    return cons;
}

void nv_presolve_distance_joint(
    nv_Space *space,
    nv_Constraint *cons,
    nv_float inv_dt
) {
    nv_DistanceJoint *dist_joint = (nv_DistanceJoint *)cons->def;
    nv_Body *a = cons->a;
    nv_Body *b = cons->b;

    // Transform anchor points
    nv_Vector2 rpa, rpb;
    nv_float invmass_a, invmass_b, invinertia_a, invinertia_b;

    if (a == NULL) {
        dist_joint->ra = nv_Vector2_zero;
        rpa = dist_joint->anchor_a;
        invmass_a = invinertia_a = 0.0;
    } else {
        dist_joint->ra = nv_Vector2_rotate(dist_joint->anchor_a, a->angle);
        rpa = nv_Vector2_add(dist_joint->ra, a->position);
        invmass_a = a->invmass;
        invinertia_a = a->invinertia;
    }

    if (b == NULL) {
        dist_joint->rb = nv_Vector2_zero;
        rpb = dist_joint->anchor_b;
        invmass_b = invinertia_b = 0.0;
    } else {
        dist_joint->rb = nv_Vector2_rotate(dist_joint->anchor_b, b->angle);
        rpb = nv_Vector2_add(dist_joint->rb, b->position);
        invmass_b = b->invmass;
        invinertia_b = b->invinertia;
    }

    nv_Vector2 delta = nv_Vector2_sub(rpb, rpa);
    dist_joint->normal = nv_Vector2_normalize(delta);
    nv_float offset = nv_Vector2_len(delta) - dist_joint->length;

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
        nv_Vector2 impulse = nv_Vector2_mul(dist_joint->normal, dist_joint->jc);

        if (a) nv_Body_apply_impulse(cons->a, nv_Vector2_neg(impulse), dist_joint->ra);
        if (b) nv_Body_apply_impulse(cons->b, impulse, dist_joint->rb);
    }
}

void nv_solve_distance_joint(nv_Constraint *cons) {
    nv_DistanceJoint *dist_joint = (nv_DistanceJoint *)cons->def;
    nv_Body *a = cons->a;
    nv_Body *b = cons->b;

    nv_Vector2 linear_velocity_a, linear_velocity_b;
    nv_float angular_velocity_a, angular_velocity_b;

    if (a == NULL) {
        linear_velocity_a = nv_Vector2_zero;
        angular_velocity_a = 0.0;
    } else {
        linear_velocity_a = a->linear_velocity;
        angular_velocity_a = a->angular_velocity;
    }

    if (b == NULL) {
        linear_velocity_b = nv_Vector2_zero;
        angular_velocity_b = 0.0;
    } else {
        linear_velocity_b = b->linear_velocity;
        angular_velocity_b = b->angular_velocity;
    }

    nv_Vector2 rv = nv_calc_relative_velocity(
        linear_velocity_a, angular_velocity_a, dist_joint->ra,
        linear_velocity_b, angular_velocity_b, dist_joint->rb
    );

    nv_float rn = nv_Vector2_dot(rv, dist_joint->normal);

    // Normal constraint lambda (impulse magnitude)
    nv_float jc = (dist_joint->bias - rn) * dist_joint->mass;

    // Accumulate impulse
    nv_float jc_max = NV_INF;//5000 * (1.0 / 60.0);

    nv_float jc0 = dist_joint->jc;
    dist_joint->jc = nv_fclamp(jc0 + jc, -jc_max, jc_max);
    jc = dist_joint->jc - jc0;

    nv_Vector2 impulse = nv_Vector2_mul(dist_joint->normal, jc);

    // Apply constraint impulse
    if (a != NULL) nv_Body_apply_impulse(a, nv_Vector2_neg(impulse), dist_joint->ra);
    if (b != NULL) nv_Body_apply_impulse(b, impulse, dist_joint->rb);
}