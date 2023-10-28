/*

  This file is a part of the Nova Physics Engine
  project and distributed under the MIT license.

  Copyright © Kadir Aksoy
  https://github.com/kadir014/nova-physics

*/

#include <math.h>
#include "novaphysics/internal.h"
#include "novaphysics/solver.h"
#include "novaphysics/vector.h"
#include "novaphysics/math.h"
#include "novaphysics/resolution.h"
#include "novaphysics/constants.h"
#include "novaphysics/space.h"
#include "novaphysics/debug.h"


/**
 * @file solver.c
 * 
 * @brief Collision and constraint solver functions.
 */


void nv_presolve_collision(
    nv_Space *space,
    nv_Resolution *res,
    nv_float inv_dt
) {
    nv_Body *a = res->a;
    nv_Body *b = res->b;
    nv_Vector2 normal = res->normal;
    nv_Vector2 tangent = nv_Vector2_perpr(normal);

    // Mixed restitution
    nv_float e = nv_mix_coefficients(
        a->material.restitution,
        b->material.restitution,
        space->mix_restitution
    );

    // Mixed friction
    res->friction = nv_mix_coefficients(
        a->material.friction,
        b->material.friction,
        space->mix_friction
    );

    for (size_t i = 0; i < res->contact_count; i++) {
        nv_Vector2 contact = res->contacts[i];

        nv_Vector2 ra = nv_Vector2_sub(contact, a->position);
        nv_Vector2 rb = nv_Vector2_sub(contact, b->position);

        // Relative velocity at contact
        nv_Vector2 rv = nv_calc_relative_velocity(
            a->linear_velocity, a->angular_velocity, ra,
            b->linear_velocity, b->angular_velocity, rb
        );

        // Restitution * normal velocity at first impact
        nv_float cn = nv_Vector2_dot(rv, normal);

        res->velocity_bias[i] = 0.0;
        if (cn < -1.0) {
            res->velocity_bias[i] = -e * cn;
        }

        // Effective normal mass
        res->mass_normal[i] = 1.0 / nv_calc_mass_k(
            normal,
            ra, rb,
            a->invmass, b->invmass,
            a->invinertia, b->invinertia
        );

        // Effective tangential mass
        res->mass_tangent[i] = 1.0 / nv_calc_mass_k(
            tangent,
            ra, rb,
            a->invmass, b->invmass,
            a->invinertia, b->invinertia
        );

        // Pseudo-velocity steering position correction bias
        nv_float correction = nv_fmin(-res->depth + NV_CORRECTION_SLOP, 0.0);
        res->position_bias[i] = -space->baumgarte * inv_dt * correction;
        res->jb[i] = 0.0;
    }
}

void nv_warmstart(nv_Space *space, nv_Resolution *res) {
    nv_Body *a = res->a;
    nv_Body *b = res->b;
    nv_Vector2 normal = res->normal;
    nv_Vector2 tangent = nv_Vector2_perpr(normal);

    for (size_t i = 0; i < res->contact_count; i++) {
        if (space->warmstarting && res->state == nv_ResolutionState_NORMAL) {
            nv_Vector2 contact = res->contacts[i];

            nv_Vector2 ra = nv_Vector2_sub(contact, a->position);
            nv_Vector2 rb = nv_Vector2_sub(contact, b->position);

            nv_Vector2 impulse = nv_Vector2_add(
                nv_Vector2_mul(normal, res->jn[i]),
                nv_Vector2_mul(tangent, res->jt[i])
            );

            nv_Body_apply_impulse(a, nv_Vector2_neg(impulse), ra);
            nv_Body_apply_impulse(b, impulse, rb);
        }
    }
}

void nv_solve_position(nv_Resolution *res) {
    nv_Body *a = res->a;
    nv_Body *b = res->b;
    nv_Vector2 normal = res->normal;

    for (size_t i = 0; i < res->contact_count; i++) {
        nv_Vector2 contact = res->contacts[i];

        nv_Vector2 ra = nv_Vector2_sub(contact, a->position);
        nv_Vector2 rb = nv_Vector2_sub(contact, b->position);

        // Relative velocity at contact
        nv_Vector2 rv = nv_calc_relative_velocity(
            a->linear_pseudo, a->angular_pseudo, ra,
            b->linear_pseudo, b->angular_pseudo, rb
        );

        nv_float cn = nv_Vector2_dot(rv, normal);

        // Normal pseudo-lambda (normal pseudo-impulse magnitude)
        nv_float jb = (res->position_bias[i] - cn) * res->mass_normal[i];

        // Accumulate pseudo-impulse
        nv_float jb0 = res->jb[i];
        res->jb[i] = nv_fmax(jb0 + jb, 0.0);
        jb = res->jb[i] - jb0;

        nv_Vector2 impulse = nv_Vector2_mul(normal, jb);

        // Apply pseudo-impulse
        nv_Body_apply_pseudo_impulse(a, nv_Vector2_neg(impulse), ra);
        nv_Body_apply_pseudo_impulse(b, impulse, rb);
    }
}

void nv_solve_velocity(nv_Resolution *res) {
    nv_Body *a = res->a;
    nv_Body *b = res->b;
    nv_Vector2 normal = res->normal;
    size_t i;

    // In an iterative solver what is applied the last affects the result more.
    // So we solve normal impulse after tangential impulse because
    // non-penetration is more important.

    // Solve tangential impulse
    for (i = 0; i < res->contact_count; i++) {
        // Don't bother calculating friction if friction coefficent is 0
        if (res->friction == 0.0) continue;

        nv_Vector2 contact = res->contacts[i];

        nv_Vector2 ra = nv_Vector2_sub(contact, a->position);
        nv_Vector2 rb = nv_Vector2_sub(contact, b->position);

        // Relative velocity at contact
        nv_Vector2 rv = nv_calc_relative_velocity(
            a->linear_velocity, a->angular_velocity, ra,
            b->linear_velocity, b->angular_velocity, rb
        );

        nv_Vector2 tangent = nv_Vector2_perpr(normal);

        // Tangential lambda (tangential impulse magnitude)
        nv_float jt = -nv_Vector2_dot(rv, tangent) * res->mass_tangent[i];

        // Accumulate tangential impulse
        nv_float f = res->jn[i] * res->friction;
        nv_float jt0 = res->jt[i];
        // Clamp lambda between friction limits
        res->jt[i] = nv_fmax(-f, nv_fmin(jt0 + jt, f));
        jt = res->jt[i] - jt0;

        nv_Vector2 impulse = nv_Vector2_mul(tangent, jt);

        // Apply tangential impulse
        nv_Body_apply_impulse(a, nv_Vector2_neg(impulse), ra);
        nv_Body_apply_impulse(b, impulse, rb);
    }

    // Solve normal impulse
    for (i = 0; i < res->contact_count; i++) {
        nv_Vector2 contact = res->contacts[i];

        nv_Vector2 ra = nv_Vector2_sub(contact, a->position);
        nv_Vector2 rb = nv_Vector2_sub(contact, b->position);

        // Relative velocity at contact
        nv_Vector2 rv = nv_calc_relative_velocity(
            a->linear_velocity, a->angular_velocity, ra,
            b->linear_velocity, b->angular_velocity, rb
        );

        nv_float cn = nv_Vector2_dot(rv, normal);

        // Normal lambda (normal impulse magnitude)
        nv_float jn = -(cn - res->velocity_bias[i]) * res->mass_normal[i];

        //Accumulate normal impulse
        nv_float jn0 = res->jn[i];
        // Clamp lambda because we only want to solve penetration
        res->jn[i] = nv_fmax(jn0 + jn, 0.0);
        jn = res->jn[i] - jn0;

        nv_Vector2 impulse = nv_Vector2_mul(normal, jn);

        // Apply normal impulse
        nv_Body_apply_impulse(a, nv_Vector2_neg(impulse), ra);
        nv_Body_apply_impulse(b, impulse, rb);
    }
}


void nv_presolve_constraint(
    nv_Space *space,
    nv_Constraint *cons,
    nv_float inv_dt
) {
    switch (cons->type) {
        // Spring constraint
        case nv_ConstraintType_SPRING:
            nv_presolve_spring(space, cons, inv_dt);
            break;

        // Distance joint constraint
        case nv_ConstraintType_DISTANCEJOINT:
            nv_presolve_distance_joint(space, cons, inv_dt);
            break;
    }
}


void nv_solve_constraint(nv_Constraint *cons) {
    switch (cons->type) {
        // Spring constraint
        case nv_ConstraintType_SPRING:
            nv_solve_spring(cons);
            break;

        // Distance joint constraint
        case nv_ConstraintType_DISTANCEJOINT:
            nv_solve_distance_joint(cons);
            break;
    }
}


void nv_presolve_spring(
    nv_Space *space,
    nv_Constraint *cons,
    nv_float inv_dt
) {
    nv_Spring *spring = (nv_Spring *)cons->head;
    nv_Body *a = cons->a;
    nv_Body *b = cons->b;

    // Transform anchor points
    nv_Vector2 ra = nv_Vector2_rotate(spring->anchor_a, a->angle);
    nv_Vector2 rb = nv_Vector2_rotate(spring->anchor_b, b->angle);
    nv_Vector2 rpa = nv_Vector2_add(ra, a->position);
    nv_Vector2 rpb = nv_Vector2_add(rb, b->position);

    nv_Vector2 delta = nv_Vector2_sub(rpb, rpa);
    nv_Vector2 dir = nv_Vector2_normalize(delta);
    nv_float dist = nv_Vector2_len(delta);
    nv_float offset = dist - spring->length;

    /*
        Calculate spring force with Hooke's Law

        Fₛ = -k * x
    */
    nv_float force = -spring->stiffness * offset;
    cons->bias = space->baumgarte * inv_dt * force;

    // Constraint effective mass
    cons->mass = nv_calc_mass_k(
        dir,
        ra, rb,
        a->invmass, b->invmass,
        a->invinertia, b->invinertia
    );
}

void nv_solve_spring(nv_Constraint *cons) {
    nv_Spring *spring = (nv_Spring *)cons->head;
    nv_Body *a = cons->a;
    nv_Body *b = cons->b;

    // Transform anchor points
    nv_Vector2 ra = nv_Vector2_rotate(spring->anchor_a, a->angle);
    nv_Vector2 rb = nv_Vector2_rotate(spring->anchor_b, b->angle);
    nv_Vector2 rpa = nv_Vector2_add(ra, a->position);
    nv_Vector2 rpb = nv_Vector2_add(rb, b->position);

    nv_Vector2 delta = nv_Vector2_sub(rpb, rpa);
    nv_Vector2 dir = nv_Vector2_normalize(delta);

    // Relative velocity
    nv_Vector2 rv = nv_calc_relative_velocity(
        a->linear_velocity, a->angular_velocity, ra,
        b->linear_velocity, b->angular_velocity, rb
    );

    nv_float rn = nv_Vector2_dot(rv, dir);
    nv_float damping = rn * spring->damping;

    nv_float lambda = (cons->bias - damping) / cons->mass;

    nv_Vector2 impulse = nv_Vector2_mul(dir, lambda);

    // Apply spring impulse
    nv_Body_apply_impulse(a, nv_Vector2_neg(impulse), ra);
    nv_Body_apply_impulse(b, impulse, rb);
}


void nv_presolve_distance_joint(
    nv_Space *space,
    nv_Constraint *cons,
    nv_float inv_dt
) {
    nv_DistanceJoint *dist_joint = (nv_DistanceJoint *)cons->head;
    nv_Body *a = cons->a;
    nv_Body *b = cons->b;

    // Transform anchor points
    nv_Vector2 ra = nv_Vector2_rotate(dist_joint->anchor_a, a->angle);
    nv_Vector2 rb = nv_Vector2_rotate(dist_joint->anchor_b, b->angle);
    nv_Vector2 rpa = nv_Vector2_add(ra, a->position);
    nv_Vector2 rpb = nv_Vector2_add(rb, b->position);

    nv_Vector2 delta = nv_Vector2_sub(rpb, rpa);
    nv_Vector2 dir = nv_Vector2_normalize(delta);
    nv_float offset = nv_Vector2_len(delta) - dist_joint->length;

    // Baumgarte stabilization
    cons->bias = -space->baumgarte * inv_dt * offset;

    // 0.9 ^ (1 / dt) ?
    // nv_float error_bias = nv_pow(0.9, 60.0);
    // nv_float dt = 1.0 / 60.0;
    // nv_float bias_coef = 1.0 - nv_pow(error_bias, dt);
    // //printf("bias_coef: %f\n", bias_coef);
    // cons->bias = -bias_coef * offset / dt;

    // Constraint effective mass
    cons->mass = 1.0 / nv_calc_mass_k(
        dir,
        ra, rb,
        a->invmass, b->invmass,
        a->invinertia, b->invinertia
    );

    // Warm-starting
    if (space->warmstarting) {
        nv_Vector2 impulse = nv_Vector2_mul(dir, cons->jc);

        nv_Body_apply_impulse(a, nv_Vector2_neg(impulse), ra);
        nv_Body_apply_impulse(b, impulse, rb);
    }
}

void nv_solve_distance_joint(nv_Constraint *cons) {
    nv_DistanceJoint *dist_joint = (nv_DistanceJoint *)cons->head;
    nv_Body *a = cons->a;
    nv_Body *b = cons->b;

    // Transform local anchor points to world
    nv_Vector2 ra = nv_Vector2_rotate(dist_joint->anchor_a, a->angle);
    nv_Vector2 rb = nv_Vector2_rotate(dist_joint->anchor_b, b->angle);
    nv_Vector2 rpa = nv_Vector2_add(ra, a->position);
    nv_Vector2 rpb = nv_Vector2_add(rb, b->position);

    nv_Vector2 delta = nv_Vector2_sub(rpb, rpa);
    nv_Vector2 dir = nv_Vector2_normalize(delta);

    nv_Vector2 rv = nv_calc_relative_velocity(
        a->linear_velocity, a->angular_velocity, ra,
        b->linear_velocity, b->angular_velocity, rb
    );

    nv_float rn = nv_Vector2_dot(rv, dir);

    // Normal constraint lambda (impulse magnitude)
    nv_float lambda = (cons->bias - rn) * cons->mass;

    //Accumulate impulse
    //cons->jc += lambda;

    nv_float jc_max = 5000 * (1.0 / 60.0);

    nv_float jc0 = cons->jc;
    // Clamp lambda because we only want to solve penetration
    //cons->jc = nv_fmax(jc0 + lambda, 0.0);
    cons->jc = nv_fclamp(jc0 + lambda, -jc_max, jc_max);
    lambda = cons->jc - jc0;

    nv_Vector2 impulse = nv_Vector2_mul(dir, lambda);

    // Apply constraint impulse
    nv_Body_apply_impulse(a, nv_Vector2_neg(impulse), ra);
    nv_Body_apply_impulse(b, impulse, rb);
}