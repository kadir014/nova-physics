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
 * solver.c
 * 
 * Collision and constraint solver functions
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
    res->restitution = nv_mix_coefficients(
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
        if (!nv_nearly_eqv(tangent, nv_Vector2_zero)) {
            res->mass_tangent = nv_calc_mass_k(
                tangent,
                ra, rb,
                a->invmass, b->invmass,
                a->invinertia, b->invinertia
            );
        }

        impulse_fric = nv_Vector2_muls(tangent, res->jt[i]);

        // Velocity-steering position correction bias
        res->bias = -space->baumgarte * inv_dt * nv_fmin(-res->depth + NV_CORRECTION_SLOP, 0.0);

        // Warm starting
        if (space->accumulate_impulses) {
            nv_Vector2 impulse = nv_Vector2_add(
                nv_Vector2_muls(normal, res->jn[i]),
                impulse_fric
            );

            a->linear_velocity = nv_Vector2_sub(
                a->linear_velocity, nv_Vector2_muls(impulse, a->invmass));

            a->angular_velocity -= nv_Vector2_cross(ra, impulse) * a->invinertia;

            b->linear_velocity = nv_Vector2_add(
                b->linear_velocity, nv_Vector2_muls(impulse, b->invmass));

            b->angular_velocity += nv_Vector2_cross(rb, impulse) * b->invinertia;
        }
    }
}

void nv_resolve_collision(nv_Resolution *res, bool accumulate) {
    nv_Body *a = res->a;
    nv_Body *b = res->b;
    nv_Vector2 normal = res->normal;

    for (size_t i = 0; i < res->contact_count; i++) {
        nv_Vector2 contact = res->contacts[i];

        /*
            Calculate normal impulse

                        -(1 + e) · vᴬᴮ
            j = ─────────────────────────────────
                1   1   (r⊥ᴬᴾ · n)²   (r⊥ᴮᴾ · n)²
                ─ + ─ + ─────────── + ───────────
                Mᴬ  Mᴮ      Iᴬ            Iᴮ
        */

        nv_Vector2 ra = nv_Vector2_sub(contact, a->position);
        nv_Vector2 rb = nv_Vector2_sub(contact, b->position);

        //Relative velocity at contact
        nv_Vector2 rv = nv_calc_relative_velocity(
            a->linear_velocity, a->angular_velocity, ra,
            b->linear_velocity, b->angular_velocity, rb
        );

        nv_float cn = nv_Vector2_dot(rv, normal);

        // Normal lambda (normal impulse magnitude)
        nv_float jn = (-cn + res->bias) / res->mass_normal;

        // Accumulate impulse
        if (accumulate) {
            nv_float jn0 = res->jn[i];
            res->jn[i] = nv_fmax(jn0 + jn, 0.0);
            jn = res->jn[i] - jn0;
        }
        else {
            jn = nv_fmax(jn, 0.0);
        }

        nv_Vector2 impulse = nv_Vector2_muls(normal, jn);

        /*
            Apply normal impulse

            vᴬ -= J * (1/Mᴬ)
            wᴬ -= (rᴬᴾ ⨯ J).z * (1/Iᴬ)

            vᴮ += J * (1/Mᴮ)
            wᴮ += (rᴮᴾ ⨯ J).z * (1/Iᴮ)
        */

        a->linear_velocity = nv_Vector2_sub(
            a->linear_velocity, nv_Vector2_muls(impulse, a->invmass));

        a->angular_velocity -= nv_Vector2_cross(ra, impulse) * a->invinertia;

        b->linear_velocity = nv_Vector2_add(
            b->linear_velocity, nv_Vector2_muls(impulse, b->invmass));

        b->angular_velocity += nv_Vector2_cross(rb, impulse) * b->invinertia;

        /*
            Calculate tangential impulse

                            -(vᴬᴮ · t)
            j = ─────────────────────────────────
                1   1   (r⊥ᴬᴾ · t)²   (r⊥ᴮᴾ · t)²
                ─ + ─ + ─────────── + ───────────
                Mᴬ  Mᴮ      Iᴬ            Iᴮ
        */

        // Don't bother calculating friction if friction coefficent is 0
        if (res->friction == 0.0) continue;

        // Relative velocity at contact
        rv = nv_calc_relative_velocity(
            a->linear_velocity, a->angular_velocity, ra,
            b->linear_velocity, b->angular_velocity, rb
        );

        nv_Vector2 tangent = nv_Vector2_perpr(normal);

        // Tangential lambda (tangential impulse magnitude)
        nv_float jt = -nv_Vector2_dot(rv, tangent)/ res->mass_tangent;

        // Accumulate impulse
        if (accumulate) {
            nv_float jt_limit = res->jn[i] * res->friction;

            nv_float jt0 = res->jt[i];
            res->jt[i] = nv_fmax(-jt_limit, nv_fmin(jt0 + jt, jt_limit));
            jt = res->jt[i] - jt0;
        }
        else {
            nv_float jt_limit = jn * res->friction;
            jt = nv_fmax(-jt_limit, nv_fmin(jt, jt_limit));
        }

        nv_Vector2 impulse_fric = nv_Vector2_muls(tangent, jt);

        /*
            Apply tangential impulse

            vᴬ -= J * (1/Mᴬ)
            wᴬ -= (rᴬᴾ ⨯ J).z * (1/Iᴬ)

            vᴮ += J * (1/Mᴮ)
            wᴮ += (rᴮᴾ ⨯ J).z * (1/Iᴮ)
        */

        a->linear_velocity = nv_Vector2_sub(
            a->linear_velocity, nv_Vector2_muls(impulse_fric, a->invmass));

        a->angular_velocity -= nv_Vector2_cross(ra, impulse_fric) * a->invinertia;

        b->linear_velocity = nv_Vector2_add(
            b->linear_velocity, nv_Vector2_muls(impulse_fric, b->invmass));

        b->angular_velocity += nv_Vector2_cross(rb, impulse_fric) * b->invinertia;
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


void nv_resolve_constraint(nv_Constraint *cons) {
    switch (cons->type) {
        // Spring constraint
        case nv_ConstraintType_SPRING:
            nv_resolve_spring(cons);
            break;

        // Distance joint constraint
        case nv_ConstraintType_DISTANCEJOINT:
            nv_resolve_distance_joint(cons);
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

    // Constraint mass
    cons->mass = nv_calc_mass_k(
        dir,
        ra, rb,
        a->invmass, b->invmass,
        a->invinertia, b->invinertia
    );
}

void nv_resolve_spring(nv_Constraint *cons) {
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

    /*
        Apply spring force

        vᴬ -= Fₛ * (1/Mᴬ)
        wᴬ -= (rᴬᴾ ⨯ Fₛ).z * (1/Iᴬ)

        vᴮ += Fₛ * (1/Mᴮ)
        wᴮ += (rᴮᴾ ⨯ Fₛ).z * (1/Iᴮ)
    */
    a->linear_velocity = nv_Vector2_sub(
        a->linear_velocity, nv_Vector2_muls(impulse, a->invmass));

    a->angular_velocity -= nv_Vector2_cross(ra, impulse) * a->invinertia;

    b->linear_velocity = nv_Vector2_add(
        b->linear_velocity, nv_Vector2_muls(impulse, b->invmass));

    b->angular_velocity += nv_Vector2_cross(rb, impulse) * b->invinertia;
}


void nv_prestep_distance_joint(
    nv_Constraint *cons,
    nv_float inv_dt,
    nv_float baumgarte
) {
    /*
        Jacobian (linear angular linear angular)

        J1 = -dir
        J2 = -(ra x dir)
        J3 = dir
        J4 = rb x dir
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
    nv_float offset = dist_joint->length - nv_Vector2_len(delta);

    // Baumgarte stabilization
    cons->bias = -baumgarte * inv_dt * offset;

    // Constraint mass
    cons->mass = nv_calc_mass_k(
        dir,
        ra, rb,
        a->invmass, b->invmass,
        a->invinertia, b->invinertia
    );
}

void nv_resolve_distance_joint(nv_Constraint *cons) {
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

    nv_Vector2 rv = nv_calc_relative_velocity(
        a->linear_velocity, a->angular_velocity, ra,
        b->linear_velocity, b->angular_velocity, rb
    );

    nv_float rn = nv_Vector2_dot(rv, dir);

    nv_float lambda = -(rn + cons->bias) / cons->mass;

    nv_Vector2 impulse = nv_Vector2_muls(dir, lambda);

    // Apply constraint impulse
    a->linear_velocity = nv_Vector2_sub(
        a->linear_velocity, nv_Vector2_muls(impulse, a->invmass));

    a->angular_velocity -= nv_Vector2_cross(ra, impulse) * a->invinertia;

    b->linear_velocity = nv_Vector2_add(
        b->linear_velocity, nv_Vector2_muls(impulse, b->invmass));

    b->angular_velocity += nv_Vector2_cross(rb, impulse) * b->invinertia;
}