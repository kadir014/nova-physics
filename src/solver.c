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
        nv_float correction = nv_fmin(-res->depth + NV_CORRECTION_SLOP, 0.0);
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
    nv_Spring *spring = (nv_Spring *)cons->def;
    nv_Body *a = cons->a;
    nv_Body *b = cons->b;

    // Transform anchor points
    nv_Vector2 rpa, rpb;
    nv_float invmass_a, invmass_b, invinertia_a, invinertia_b;

    if (a == NULL) {
        cons->ra = nv_Vector2_zero;
        rpa = spring->anchor_a;
        invmass_a = invinertia_a = 0.0;
    } else {
        cons->ra = nv_Vector2_rotate(spring->anchor_a, a->angle);
        rpa = nv_Vector2_add(cons->ra, a->position);
        invmass_a = a->invmass;
        invinertia_a = a->invinertia;
    }

    if (b == NULL) {
        cons->rb = nv_Vector2_zero;
        rpb = spring->anchor_b;
        invmass_b = invinertia_b = 0.0;
    } else {
        cons->rb = nv_Vector2_rotate(spring->anchor_b, b->angle);
        rpb = nv_Vector2_add(cons->rb, b->position);
        invmass_b = b->invmass;
        invinertia_b = b->invinertia;
    }

    nv_Vector2 delta = nv_Vector2_sub(rpb, rpa);
    cons->normal = nv_Vector2_normalize(delta);
    nv_float dist = nv_Vector2_len(delta);

    // Constraint effective mass
    nv_float mass_k = nv_calc_mass_k(
        cons->normal,
        cons->ra, cons->rb,
        invmass_a, invmass_b,
        invinertia_a, invinertia_b
    );
    cons->mass = 1.0 / mass_k;
    
    spring->target_rn = 0.0;
    spring->v_coef = 1.0 - nv_exp(-spring->damping / inv_dt * mass_k);

    // Apply spring force
    nv_float spring_force = (spring->length - dist) * spring->stiffness;

    cons->jc = spring_force / inv_dt;
    nv_Vector2 spring_impulse = nv_Vector2_mul(cons->normal, cons->jc);

    if (a != NULL) nv_Body_apply_impulse(a, nv_Vector2_neg(spring_impulse), cons->ra);
    if (b != NULL) nv_Body_apply_impulse(b, spring_impulse, cons->rb);
}

void nv_solve_spring(nv_Constraint *cons) {
    nv_Spring *spring = (nv_Spring *)cons->def;
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

    // Relative velocity
    nv_Vector2 rv = nv_calc_relative_velocity(
        linear_velocity_a, angular_velocity_a, cons->ra,
        linear_velocity_b, angular_velocity_b, cons->rb
    );

    nv_float rn = nv_Vector2_dot(rv, cons->normal);

    // Velocity loss from drag
    nv_float v_damp = (spring->target_rn - rn) * spring->v_coef;
    spring->target_rn = rn + v_damp;

    nv_float jc_damp = v_damp * cons->mass;
    cons->jc += jc_damp;

    nv_Vector2 impulse_damp = nv_Vector2_mul(cons->normal, jc_damp);

    if (a != NULL) nv_Body_apply_impulse(a, nv_Vector2_neg(impulse_damp), cons->ra);
    if (b != NULL) nv_Body_apply_impulse(b, impulse_damp, cons->rb);
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
        cons->ra = nv_Vector2_zero;
        rpa = dist_joint->anchor_a;
        invmass_a = invinertia_a = 0.0;
    } else {
        cons->ra = nv_Vector2_rotate(dist_joint->anchor_a, a->angle);
        rpa = nv_Vector2_add(cons->ra, a->position);
        invmass_a = a->invmass;
        invinertia_a = a->invinertia;
    }

    if (b == NULL) {
        cons->rb = nv_Vector2_zero;
        rpb = dist_joint->anchor_b;
        invmass_b = invinertia_b = 0.0;
    } else {
        cons->rb = nv_Vector2_rotate(dist_joint->anchor_b, b->angle);
        rpb = nv_Vector2_add(cons->rb, b->position);
        invmass_b = b->invmass;
        invinertia_b = b->invinertia;
    }

    nv_Vector2 delta = nv_Vector2_sub(rpb, rpa);
    cons->normal = nv_Vector2_normalize(delta);
    nv_float offset = nv_Vector2_len(delta) - dist_joint->length;

    // Baumgarte stabilization
    cons->bias = -space->baumgarte * inv_dt * offset;

    // Constraint effective mass
    cons->mass = 1.0 / nv_calc_mass_k(
        cons->normal,
        cons->ra, cons->rb,
        invmass_a, invmass_b,
        invinertia_a, invinertia_b
    );

    if (space->warmstarting) {
        nv_Vector2 impulse = nv_Vector2_mul(cons->normal, cons->jc);

        if (cons->a != NULL) nv_Body_apply_impulse(cons->a, nv_Vector2_neg(impulse), cons->ra);
        if (cons->b != NULL) nv_Body_apply_impulse(cons->b, impulse, cons->rb);
    }
}

void nv_solve_distance_joint(nv_Constraint *cons) {
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
        linear_velocity_a, angular_velocity_a, cons->ra,
        linear_velocity_b, angular_velocity_b, cons->rb
    );

    nv_float rn = nv_Vector2_dot(rv, cons->normal);

    // Normal constraint lambda (impulse magnitude)
    nv_float jc = (cons->bias - rn) * cons->mass;

    //Accumulate impulse
    nv_float jc_max = NV_INF;//5000 * (1.0 / 60.0);

    nv_float jc0 = cons->jc;
    cons->jc = nv_fclamp(jc0 + jc, -jc_max, jc_max);
    jc = cons->jc - jc0;

    nv_Vector2 impulse = nv_Vector2_mul(cons->normal, jc);

    // Apply constraint impulse
    if (a != NULL) nv_Body_apply_impulse(a, nv_Vector2_neg(impulse), cons->ra);
    if (b != NULL) nv_Body_apply_impulse(b, impulse, cons->rb);
}