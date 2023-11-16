/*

  This file is a part of the Nova Physics Engine
  project and distributed under the MIT license.

  Copyright © Kadir Aksoy
  https://github.com/kadir014/nova-physics

*/

#include "novaphysics/hinge_joint.h"
#include "novaphysics/space.h"


/**
 * @file hinge_joint.c
 * 
 * @brief Hinge joint implementation.
 */


nv_Constraint *nv_HingeJoint_new(
    nv_Body *a,
    nv_Body *b,
    nv_Vector2 anchor
) {
    nv_Constraint *cons = NV_NEW(nv_Constraint);
    if (!cons) return NULL;

    cons->a = a;
    cons->b = b;
    cons->type = nv_ConstraintType_HINGEJOINT;

    cons->def = (void *)NV_NEW(nv_HingeJoint);
    if (!cons->def) return NULL;
    nv_HingeJoint *hinge_joint = (nv_HingeJoint *)cons->def;

    hinge_joint->enable_limits = true;
    hinge_joint->lower_limit = -NV_PI / 2.0;
    hinge_joint->upper_limit = 0.0;
    hinge_joint->angle = 0.0;

    hinge_joint->anchor = anchor;
    nv_float angle_a, angle_b;
    if (a) {
        hinge_joint->anchor_a = nv_Vector2_sub(anchor, a->position);
        angle_a = a->angle;
    }
    else {
        hinge_joint->anchor_a = anchor;
        angle_a = 0.0;
    }
    if (b) {
        hinge_joint->anchor_b = nv_Vector2_sub(anchor, b->position);
        angle_b = b->angle;
    }
    else {
        hinge_joint->anchor_b = anchor;
        angle_b = 0.0;
    }

    hinge_joint->reference_angle = angle_b - angle_a;
    hinge_joint->lower_impulse = 0.0;
    hinge_joint->upper_impulse = 0.0;
    hinge_joint->axial_mass = 0.0;
    hinge_joint->ra = nv_Vector2_zero;
    hinge_joint->rb = nv_Vector2_zero;
    hinge_joint->normal = nv_Vector2_zero;
    hinge_joint->bias = 0.0;
    hinge_joint->mass = 0.0;
    hinge_joint->jc = 0.0;

    return cons;
}

void nv_presolve_hinge_joint(
    nv_Space *space,
    nv_Constraint *cons,
    nv_float inv_dt
) {
    nv_HingeJoint *hinge_joint = (nv_HingeJoint *)cons->def;
    nv_Body *a = cons->a;
    nv_Body *b = cons->b;

    // Transform anchor points
    nv_Vector2 rpa, rpb;
    nv_float invmass_a, invmass_b, invinertia_a, invinertia_b;

    if (a == NULL) {
        hinge_joint->ra = nv_Vector2_zero;
        rpa = hinge_joint->anchor_a;
        invmass_a = invinertia_a = 0.0;
    } else {
        hinge_joint->ra = nv_Vector2_rotate(hinge_joint->anchor_a, a->angle);
        rpa = nv_Vector2_add(hinge_joint->ra, a->position);
        invmass_a = a->invmass;
        invinertia_a = a->invinertia;
    }

    if (b == NULL) {
        hinge_joint->rb = nv_Vector2_zero;
        rpb = hinge_joint->anchor_b;
        invmass_b = invinertia_b = 0.0;
    } else {
        hinge_joint->rb = nv_Vector2_rotate(hinge_joint->anchor_b, b->angle);
        rpb = nv_Vector2_add(hinge_joint->rb, b->position);
        invmass_b = b->invmass;
        invinertia_b = b->invinertia;
    }

    nv_Vector2 delta = nv_Vector2_sub(rpb, rpa);
    if (nv_Vector2_len2(delta) == 0.0) hinge_joint->normal = nv_Vector2_zero;
    else hinge_joint->normal = nv_Vector2_normalize(delta);
    nv_float offset = nv_Vector2_len(delta);

    // Baumgarte position correction bias
    hinge_joint->bias = -space->baumgarte * inv_dt * offset;

    // Distance constraint effective mass
    hinge_joint->mass = 1.0 / nv_calc_mass_k(
        hinge_joint->normal,
        hinge_joint->ra, hinge_joint->rb,
        invmass_a, invmass_b,
        invinertia_a, invinertia_b
    );

    hinge_joint->axial_mass = 1.0 / (invinertia_a + invinertia_b);

    nv_float angle_a, angle_b;
    if (a) angle_a = a->angle;
    else angle_a = 0.0;
    if (b) angle_b = b->angle;
    else angle_b = 0.0;

    hinge_joint->angle = angle_b - angle_a - hinge_joint->reference_angle;

    if (space->warmstarting) {
        nv_Vector2 impulse = nv_Vector2_mul(hinge_joint->normal, hinge_joint->jc);
        nv_float axial_impulse = hinge_joint->lower_impulse - hinge_joint->upper_impulse;

        if (a) {
            nv_Body_apply_impulse(a, nv_Vector2_neg(impulse), hinge_joint->ra);
            a->angular_velocity -= a->invinertia * axial_impulse;
        } 
        if (b) {
            nv_Body_apply_impulse(b, impulse, hinge_joint->rb);
            b->angular_velocity += b->invinertia * axial_impulse;
        }
        
    }
    else {
        hinge_joint->upper_impulse = 0.0;
        hinge_joint->lower_impulse = 0.0;
    }
}

void nv_solve_hinge_joint(nv_Constraint *cons) {
    nv_HingeJoint *hinge_joint = (nv_HingeJoint *)cons->def;
    nv_Body *a = cons->a;
    nv_Body *b = cons->b;

    nv_float inv_dt = 60.0;

    // Solve angular limits
    if (hinge_joint->enable_limits) {
        nv_float c, wr, wa, wb, impulse, impulse0;

        if (a) wa = a->angular_velocity;
        else wa = 0.0;
        if (b) wb = b->angular_velocity;
        else wb = 0.0;

        // Solve lower limit

        c = hinge_joint->angle - hinge_joint->lower_limit;
        wr = wb - wa;
        impulse = -hinge_joint->axial_mass * (wr + nv_fmax(c, 0.0) * inv_dt);

        // Accumulate lower impulse
        impulse0 = hinge_joint->lower_impulse;
        hinge_joint->lower_impulse = nv_fmax(hinge_joint->lower_impulse + impulse, 0.0);
        impulse = hinge_joint->lower_impulse - impulse0;

        // Apply lower impulse
        if (a) a->angular_velocity -= impulse * a->invinertia;
        if (b) b->angular_velocity += impulse * b->invinertia;

        // Solve upper limmit

        c = hinge_joint->upper_limit - hinge_joint->angle;
        wr = wa - wb;
        impulse = -hinge_joint->axial_mass * (wr + nv_fmax(c, 0.0) * inv_dt);

        // Accumulate upper impulse
        impulse0 = hinge_joint->upper_impulse;
        hinge_joint->upper_impulse = nv_fmax(hinge_joint->upper_impulse + impulse, 0.0);
        impulse = hinge_joint->upper_impulse - impulse0;

        // Apply upper impulse
        if (a) a->angular_velocity += impulse * a->invinertia;
        if (b) b->angular_velocity -= impulse * b->invinertia;
    }

    // Solve distance constraintt

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
        linear_velocity_a, angular_velocity_a, hinge_joint->ra,
        linear_velocity_b, angular_velocity_b, hinge_joint->rb
    );

    nv_float rn = nv_Vector2_dot(rv, hinge_joint->normal);

    // Normal position constraint lambda (impulse magnitude)
    nv_float jc = (hinge_joint->bias - rn) * hinge_joint->mass;

    // Accumulate impulse
    nv_float jc_max = NV_INF;//5000 * (1.0 / 60.0);

    nv_float jc0 = hinge_joint->jc;
    hinge_joint->jc = nv_fclamp(jc0 + jc, -jc_max, jc_max);
    jc = hinge_joint->jc - jc0;

    nv_Vector2 impulse = nv_Vector2_mul(hinge_joint->normal, jc);

    // Apply position impulse
    if (a != NULL) nv_Body_apply_impulse(a, nv_Vector2_neg(impulse), hinge_joint->ra);
    if (b != NULL) nv_Body_apply_impulse(b, impulse, hinge_joint->rb);
}