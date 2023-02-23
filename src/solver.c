/*

  This file is a part of the Nova Physics Engine
  project and distributed under the MIT license.

  Copyright © Kadir Aksoy
  https://github.com/kadir014/nova-physics

*/

#include <math.h>
#include "novaphysics/solver.h"
#include "novaphysics/vector.h"
#include "novaphysics/math.h"
#include "novaphysics/resolution.h"
#include "novaphysics/constants.h"


/**
 * solver.c
 * 
 * Collision and constraint solver functions
 */

void nv_prestep_collision(
    nv_Resolution *res,
    double inv_dt,
    bool accumulate,
    double correction_factor
) {
    nv_Body *a = res->a;
    nv_Body *b = res->b;
    nv_Vector2 normal = res->normal;

    // Mixed restitution
    res->e = fmin(a->material.restitution, b->material.restitution);

    // Mixed friction
    res->sf = sqrt(a->material.static_friction * b->material.static_friction);
    res->df = sqrt(a->material.dynamic_friction * b->material.dynamic_friction);

    for (size_t i = 0; i < res->contact_count; i++) {
        nv_Vector2 contact = res->contacts[i];

        nv_Vector2 ra = nv_Vector2_sub(contact, a->position);
        nv_Vector2 rb = nv_Vector2_sub(contact, b->position);

        nv_Vector2 rv = nv_calc_relative_velocity(
            a->linear_velocity, a->angular_velocity, ra,
            b->linear_velocity, b->angular_velocity, rb
        );

        res->mass_normal = nv_calc_mass_k(
            normal,
            ra, rb,
            a->invmass, b->invmass,
            a->invinertia, b->invinertia
        );

        nv_Vector2 tangent = nv_Vector2_perpr(normal);

        nv_Vector2 impulse_fric = nv_Vector2_zero;

        if (!nv_nearly_eqv(tangent, nv_Vector2_zero)) {
            res->mass_tangent = nv_calc_mass_k(
                tangent,
                ra, rb,
                a->invmass, b->invmass,
                a->invinertia, b->invinertia
            );
        }

        impulse_fric = nv_Vector2_muls(tangent, res->jt[i]);

        res->bias = -correction_factor * inv_dt * fmin(-res->depth + NV_CORRECTION_SLOP, 0.0);

        if (accumulate) {
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

        // Relative velocity at contact
        nv_Vector2 rv = nv_calc_relative_velocity(
            a->linear_velocity, a->angular_velocity, ra,
            b->linear_velocity, b->angular_velocity, rb
        );

        double cn = nv_Vector2_dot(rv, normal);

        double numer = -(1.0 + res->e) * cn + res->bias;

        double jn = numer / res->mass_normal;

        // Accumulate impulse
        if (accumulate) {
            double jn0 = res->jn[i];
            res->jn[i] = fmax(jn0 + jn, 0.0);
            jn = res->jn[i] - jn0;
        }
        else {
            jn = fmax(jn, 0.0);
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

        // Don't bother calculating friction if both fric. coefficents are 0
        if (res->sf == 0.0 && res->df == 0.0) continue;

        // Relative velocity at contact
        rv = nv_calc_relative_velocity(
            a->linear_velocity, a->angular_velocity, ra,
            b->linear_velocity, b->angular_velocity, rb
        );

        nv_Vector2 tangent = nv_Vector2_perpr(normal);

        numer = -nv_Vector2_dot(rv, tangent);

        double jt = numer / res->mass_tangent;

        // TODO: Reimplement Coulomb's friction law
        // Coulomb's law
        // Ff <= u * Fn
        // if (fabs(jt) <= jn * res->sf)
        //     jt = -jn * res->df;

        // Accumulate impulse
        if (accumulate) {
            double jt_limit = res->jn[i] * res->df;

            double jt0 = res->jt[i];
            res->jt[i] = fmax(-jt_limit, fmin(jt0 + jt, jt_limit));
            jt = res->jt[i] - jt0;
        }
        else {
            double jt_limit = jn * res->df;
            jt = fmax(-jt_limit, fmin(jt, jt_limit));
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


void nv_resolve_constraint(nv_Constraint *cons) {
    switch (cons->type) {
        // Spring constraint
        case nv_ConstraintType_SPRING:
            nv_resolve_spring(cons);
            break;
    }
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
    double dist = nv_Vector2_len(delta);
    double offset = dist - spring->length;

    // Relative velocity
    nv_Vector2 rv = nv_calc_relative_velocity(
        a->linear_velocity, a->angular_velocity, ra,
        b->linear_velocity, b->angular_velocity, rb
    );

    double rn = nv_Vector2_dot(rv, dir);

    double damping = rn * spring->damping;

    /*
        Calculate spring force with Hooke's Law

        Fₛ = -k * x [- damping]
    */
    nv_Vector2 force = nv_Vector2_muls(dir, -spring->stiffness * offset - damping);

    /*
        Apply spring force

        vᴬ -= Fₛ * (1/Mᴬ)
        wᴬ -= (rᴬᴾ ⨯ Fₛ).z * (1/Iᴬ)

        vᴮ += Fₛ * (1/Mᴮ)
        wᴮ += (rᴮᴾ ⨯ Fₛ).z * (1/Iᴮ)
    */
    if (a->type != nv_BodyType_STATIC) {
        a->linear_velocity = nv_Vector2_sub(
            a->linear_velocity, nv_Vector2_muls(force, a->invmass));

        a->angular_velocity -= nv_Vector2_cross(ra, force) * a->invinertia;
    }
    if (b->type != nv_BodyType_STATIC) {
        b->linear_velocity = nv_Vector2_add(
            b->linear_velocity, nv_Vector2_muls(force, b->invmass));

        b->angular_velocity += nv_Vector2_cross(rb, force) * b->invinertia;
    }
}