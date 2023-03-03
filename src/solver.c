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


/**
 * @file solver.c
 * 
 * @details Collision and constraint solver functions
 *          
 *          Function documentations are in novaphysics/solver.h
 */


void nv_prestep_collision(
    nv_Space *space,
    nv_Resolution *res,
    nv_float inv_dt
) {
    nv_Body *a = res->a;
    nv_Body *b = res->b;
    nv_Vector2 normal = res->normal;

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

        res->restitution[i] = cn < 0.0 ? -cn * e : 0.0;

        // Effective normal mass
        res->mass_normal = nv_calc_mass_k(
            normal,
            ra, rb,
            a->invmass, b->invmass,
            a->invinertia, b->invinertia
        );

        nv_Vector2 tangent = nv_Vector2_perpr(normal);

        nv_Vector2 impulse_fric = nv_Vector2_zero;

        // Effective tangential mass
        res->mass_tangent = nv_calc_mass_k(
            tangent,
            ra, rb,
            a->invmass, b->invmass,
            a->invinertia, b->invinertia
        );

        // Pseudo-velocity steering position correction bias
        nv_float correction = nv_fmin(-res->depth + NV_CORRECTION_SLOP, 0.0);
        res->bias = -space->baumgarte * inv_dt * correction;
        res->jb[i] = 0.0;

        // Warm-starting
        if (space->warmstarting) {
            nv_Vector2 impulse = nv_Vector2_add(
                nv_Vector2_muls(normal, res->jn[i]),
                nv_Vector2_muls(tangent, res->jt[i])
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
        nv_float jb = (res->bias - cn) / res->mass_normal;

        // Accumulate pseudo-impulse
        nv_float jb0 = res->jb[i];
        res->jb[i] = nv_fmax(jb0 + jb, 0.0);
        jb = res->jb[i] - jb0;

        nv_Vector2 impulse = nv_Vector2_muls(normal, jb);

        // Apply pseudo-impulse
        nv_Body_apply_pseudo_impulse(a, nv_Vector2_neg(impulse), ra);
        nv_Body_apply_pseudo_impulse(b, impulse, rb);
    }
}

void nv_solve_velocity(nv_Resolution *res) {
    nv_Body *a = res->a;
    nv_Body *b = res->b;
    nv_Vector2 normal = res->normal;

    for (size_t i = 0; i < res->contact_count; i++) {
        nv_Vector2 contact = res->contacts[i];

        // Calculate normal impulse

        nv_Vector2 ra = nv_Vector2_sub(contact, a->position);
        nv_Vector2 rb = nv_Vector2_sub(contact, b->position);

        // Relative velocity at contact
        nv_Vector2 rv = nv_calc_relative_velocity(
            a->linear_velocity, a->angular_velocity, ra,
            b->linear_velocity, b->angular_velocity, rb
        );

        nv_float cn = nv_Vector2_dot(rv, normal);

        // Normal lambda (normal impulse magnitude)
        nv_float jn = (-cn + res->restitution[i]) / res->mass_normal;

        // Accumulate normal impulse
        nv_float jn0 = res->jn[i];
        // Clamp normal lambda, we only have to push objects
        res->jn[i] = nv_fmax(jn0 + jn, 0.0);
        jn = res->jn[i] - jn0;

        nv_Vector2 impulse = nv_Vector2_muls(normal, jn);

        // Apply normal impulse
        nv_Body_apply_impulse(a, nv_Vector2_neg(impulse), ra);
        nv_Body_apply_impulse(b, impulse, rb);


        // Calculate tangential impulse

        // Don't bother calculating friction if friction coefficent is 0
        if (res->friction == 0.0) continue;

        // Relative velocity at contact
        rv = nv_calc_relative_velocity(
            a->linear_velocity, a->angular_velocity, ra,
            b->linear_velocity, b->angular_velocity, rb
        );

        nv_Vector2 tangent = nv_Vector2_perpr(normal);

        // Tangential lambda (tangential impulse magnitude)
        nv_float jt = -nv_Vector2_dot(rv, tangent) / res->mass_tangent;

        // Accumulate impulse
        nv_float f = res->jn[i] * res->friction;
        nv_float jt0 = res->jt[i];
        // Clamp lambda between friction limits
        res->jt[i] = nv_fmax(-f, nv_fmin(jt0 + jt, f));
        jt = res->jt[i] - jt0;

        impulse = nv_Vector2_muls(tangent, jt);

        // Apply tangential impulse
        nv_Body_apply_impulse(a, nv_Vector2_neg(impulse), ra);
        nv_Body_apply_impulse(b, impulse, rb);
    }
}


void nv_prestep_constraint(
    nv_Constraint *cons,
    nv_float inv_dt,
    nv_float baumgarte
) {
    switch (cons->type) {
        // Spring constraint
        case nv_ConstraintType_SPRING:
            nv_prestep_spring(cons, inv_dt, baumgarte);
            break;

        // Distance joint constraint
        case nv_ConstraintType_DISTANCEJOINT:
            nv_prestep_distance_joint(cons, inv_dt, baumgarte);
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


void nv_prestep_spring(nv_Constraint *cons, nv_float inv_dt, nv_float baumgarte) {
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
    cons->bias = baumgarte * inv_dt * force;

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
    nv_float dist = nv_Vector2_len(delta);
    nv_float offset = dist - spring->length;

    // Relative velocity
    nv_Vector2 rv = nv_calc_relative_velocity(
        a->linear_velocity, a->angular_velocity, ra,
        b->linear_velocity, b->angular_velocity, rb
    );

    nv_float rn = nv_Vector2_dot(rv, dir);
    nv_float damping = rn * spring->damping;

    nv_float lambda = (cons->bias - damping) / cons->mass;

    nv_Vector2 impulse = nv_Vector2_muls(dir, lambda);

    // Apply spring impulse
    nv_Body_apply_impulse(a, nv_Vector2_neg(impulse), ra);
    nv_Body_apply_impulse(b, impulse, rb);
}


void nv_prestep_distance_joint(
    nv_Constraint *cons,
    nv_float inv_dt,
    nv_float baumgarte
) {
    /*
        Distance Joint Jacobian

        [
          -dir
          -ra x dir
           dir
           rb x dir
        ]
    */

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

    nv_Vector2 rv = nv_calc_relative_velocity(
        a->linear_velocity, a->angular_velocity, ra,
        b->linear_velocity, b->angular_velocity, rb
    );

    // Baumgarte stabilization
    cons->bias = -baumgarte * inv_dt * offset;

    // Constraint effective mass
    cons->mass = nv_calc_mass_k(
        dir,
        ra, rb,
        a->invmass, b->invmass,
        a->invinertia, b->invinertia
    );

    // Warm-starting
    nv_Vector2 impulse = nv_Vector2_muls(dir, cons->jc);

    nv_Body_apply_impulse(a, nv_Vector2_neg(impulse), ra);
    nv_Body_apply_impulse(b, impulse, rb);
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
    nv_float lambda = (cons->bias - rn) / (cons->mass);

    // Accumulate impulse max_force * dt
    // nv_float max_force = 10000.0 * (1.0 / 60.0);
    // nv_float jc0 = cons->jc;
    // // Clamp lambda between maximum constraint force limits
    // cons->jc = nv_fmax(-max_force, nv_fmin(jc0 + lambda, max_force));
    // lambda = cons->jc - jc0;

    cons->jc += lambda;

    nv_Vector2 impulse = nv_Vector2_muls(dir, lambda);

    // Apply constraint impulse
    nv_Body_apply_impulse(a, nv_Vector2_neg(impulse), ra);
    nv_Body_apply_impulse(b, impulse, rb);
}