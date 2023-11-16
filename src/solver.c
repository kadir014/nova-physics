/*

  This file is a part of the Nova Physics Engine
  project and distributed under the MIT license.

  Copyright © Kadir Aksoy
  https://github.com/kadir014/nova-physics

*/

#include <math.h>
#include "novaphysics/internal.h"
#include "novaphysics/solver.h"
#include "novaphysics/constraint.h"
#include "novaphysics/spring.h"
#include "novaphysics/distance_joint.h"
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
        nv_Contact *contact = &res->contacts[i];

        contact->ra = nv_Vector2_sub(contact->position, a->position);
        contact->rb = nv_Vector2_sub(contact->position, b->position);

        // Relative velocity at contact
        nv_Vector2 rv = nv_calc_relative_velocity(
            a->linear_velocity, a->angular_velocity, contact->ra,
            b->linear_velocity, b->angular_velocity, contact->rb
        );

        // Restitution * normal velocity at first impact
        nv_float cn = nv_Vector2_dot(rv, normal);

        contact->velocity_bias = 0.0;
        if (cn < -1.0) {
            contact->velocity_bias = -e * cn;
        }

        // Effective normal mass
        contact->mass_normal = 1.0 / nv_calc_mass_k(
            normal,
            contact->ra, contact->rb,
            a->invmass, b->invmass,
            a->invinertia, b->invinertia
        );

        // Effective tangential mass
        contact->mass_tangent = 1.0 / nv_calc_mass_k(
            tangent,
            contact->ra, contact->rb,
            a->invmass, b->invmass,
            a->invinertia, b->invinertia
        );

        // Pseudo-velocity steering position correction bias
        nv_float correction = nv_fmin(-res->depth + NV_POSITION_CORRECTION_SLOP, 0.0);
        contact->position_bias = -space->baumgarte * inv_dt * correction;
        contact->jb = 0.0;
    }
}

void nv_warmstart(nv_Space *space, nv_Resolution *res) {
    nv_Body *a = res->a;
    nv_Body *b = res->b;
    nv_Vector2 normal = res->normal;
    nv_Vector2 tangent = nv_Vector2_perpr(normal);

    for (size_t i = 0; i < res->contact_count; i++) {
        if (space->warmstarting && res->state == nv_ResolutionState_NORMAL) {
            nv_Contact *contact = &res->contacts[i];

            nv_Vector2 impulse = nv_Vector2_add(
                nv_Vector2_mul(normal, contact->jn),
                nv_Vector2_mul(tangent, contact->jt)
            );
            
            nv_Body_apply_impulse(a, nv_Vector2_neg(impulse), contact->ra);
            nv_Body_apply_impulse(b, impulse, contact->rb);
        }
    }
}

void nv_solve_position(nv_Resolution *res) {
    nv_Body *a = res->a;
    nv_Body *b = res->b;
    nv_Vector2 normal = res->normal;

    for (size_t i = 0; i < res->contact_count; i++) {
        nv_Contact *contact = &res->contacts[i];

        // Relative velocity at contact
        nv_Vector2 rv = nv_calc_relative_velocity(
            a->linear_pseudo, a->angular_pseudo, contact->ra,
            b->linear_pseudo, b->angular_pseudo, contact->rb
        );

        nv_float cn = nv_Vector2_dot(rv, normal);

        // Normal pseudo-lambda (normal pseudo-impulse magnitude)
        nv_float jb = (contact->position_bias - cn) * contact->mass_normal;

        // Accumulate pseudo-impulse
        nv_float jb0 = contact->jb;
        contact->jb = nv_fmax(jb0 + jb, 0.0);
        jb = contact->jb - jb0;

        nv_Vector2 impulse = nv_Vector2_mul(normal, jb);

        // Apply pseudo-impulse
        nv_Body_apply_pseudo_impulse(a, nv_Vector2_neg(impulse), contact->ra);
        nv_Body_apply_pseudo_impulse(b, impulse, contact->rb);
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

        nv_Contact *contact = &res->contacts[i];

        // Relative velocity at contact
        nv_Vector2 rv = nv_calc_relative_velocity(
            a->linear_velocity, a->angular_velocity, contact->ra,
            b->linear_velocity, b->angular_velocity, contact->rb
        );

        nv_Vector2 tangent = nv_Vector2_perpr(normal);

        // Tangential lambda (tangential impulse magnitude)
        nv_float jt = -nv_Vector2_dot(rv, tangent) * contact->mass_tangent;

        // Accumulate tangential impulse
        nv_float f = contact->jn * res->friction;
        nv_float jt0 = contact->jt;
        // Clamp lambda between friction limits
        contact->jt = nv_fmax(-f, nv_fmin(jt0 + jt, f));
        jt = contact->jt - jt0;

        nv_Vector2 impulse = nv_Vector2_mul(tangent, jt);

        // Apply tangential impulse
        nv_Body_apply_impulse(a, nv_Vector2_neg(impulse), contact->ra);
        nv_Body_apply_impulse(b, impulse, contact->rb);
    }

    // Solve normal impulse
    for (i = 0; i < res->contact_count; i++) {
        nv_Contact *contact = &res->contacts[i];

        // Relative velocity at contact
        nv_Vector2 rv = nv_calc_relative_velocity(
            a->linear_velocity, a->angular_velocity, contact->ra,
            b->linear_velocity, b->angular_velocity, contact->rb
        );

        nv_float cn = nv_Vector2_dot(rv, normal);

        // Normal lambda (normal impulse magnitude)
        nv_float jn = -(cn - contact->velocity_bias) * contact->mass_normal;

        //Accumulate normal impulse
        nv_float jn0 = contact->jn;
        // Clamp lambda because we only want to solve penetration
        contact->jn = nv_fmax(jn0 + jn, 0.0);
        jn = contact->jn - jn0;

        nv_Vector2 impulse = nv_Vector2_mul(normal, jn);

        // Apply normal impulse
        nv_Body_apply_impulse(a, nv_Vector2_neg(impulse), contact->ra);
        nv_Body_apply_impulse(b, impulse, contact->rb);
    }
}


void nv_presolve_constraint(
    nv_Space *space,
    nv_Constraint *cons,
    nv_float inv_dt
) {
    switch (cons->type) {
        case nv_ConstraintType_SPRING:
            nv_presolve_spring(space, cons, inv_dt);
            break;

        case nv_ConstraintType_DISTANCEJOINT:
            nv_presolve_distance_joint(space, cons, inv_dt);
            break;

        case nv_ConstraintType_HINGEJOINT:
            nv_presolve_hinge_joint(space, cons, inv_dt);
            break;
    }
}


void nv_solve_constraint(nv_Constraint *cons) {
    switch (cons->type) {

        case nv_ConstraintType_SPRING:
            nv_solve_spring(cons);
            break;

        case nv_ConstraintType_DISTANCEJOINT:
            nv_solve_distance_joint(cons);
            break;

        case nv_ConstraintType_HINGEJOINT:
            nv_solve_hinge_joint(cons);
            break;
    }
}