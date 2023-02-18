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
#include "novaphysics/spring.h"


/**
 * solver.c
 * 
 * Collision and constraint solver functions
 */


void nv_positional_correction(nv_Resolution res) {
    nv_Body *a = res.a;
    nv_Body *b = res.b;
    nv_Vector2 normal = res.normal;


    double mass = (a->invmass + b->invmass);

    nv_Vector2 correction = nv_Vector2_muls(
        nv_Vector2_muls(
                normal,
                nv_maxf(res.depth - NV_CORRECTION_SLOP, 0.0) / mass
            ),
            NV_CORRECTION_PERCENT
        );
    
    if (a->type != nv_BodyType_STATIC)
        a->position = nv_Vector2_sub(
            a->position, nv_Vector2_muls(correction, a->invmass));

    if (b->type != nv_BodyType_STATIC)
        b->position = nv_Vector2_add(
            b->position, nv_Vector2_muls(correction, b->invmass));
}


void nv_resolve_collision(nv_Resolution res) {
    nv_Body *a = res.a;
    nv_Body *b = res.b;
    nv_Vector2 normal = res.normal;

    // Restitution of collision
    double e = nv_minf(a->material.restitution, b->material.restitution);

    double sf = sqrt(a->material.static_friction * b->material.static_friction);
    double df = sqrt(a->material.dynamic_friction * b->material.dynamic_friction);

    nv_Vector2 contact;

    // Take the midpoint of contacts
    if (res.contact_count == 2)
        contact = nv_Vector2_divs(
            nv_Vector2_add(res.contacts[0], res.contacts[1]), 2.0);
    else
        contact = res.contacts[0];


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

    nv_Vector2 ra_perp = nv_Vector2_perp(ra);
    nv_Vector2 rb_perp = nv_Vector2_perp(rb);

    // Relative velocity
    nv_Vector2 rv = nv_calc_relative_velocity(
        a->linear_velocity, a->angular_velocity, ra,
        b->linear_velocity, b->angular_velocity, rb
    );

    double cn = nv_Vector2_dot(rv, normal);

    // Velocities are seperating
    if (cn > 0.0) return;

    double ran = nv_Vector2_dot(ra_perp, normal);
    double rbn = nv_Vector2_dot(rb_perp, normal);

    double numer = -(1.0 + e) * cn;
    double denom = (a->invmass + b->invmass) +
                    ((ran*ran * a->invinertia) + (rbn*rbn * b->invinertia));

    double j = numer / denom;

    nv_Vector2 impulse = nv_Vector2_muls(normal, j);

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

    // Relative velocity
    rv = nv_calc_relative_velocity(
        a->linear_velocity, a->angular_velocity, ra,
        b->linear_velocity, b->angular_velocity, rb
    );

    nv_Vector2 tangent = nv_Vector2_sub(
        rv, nv_Vector2_muls(normal, nv_Vector2_dot(rv, normal)));

    if (nv_nearly_eqv(tangent, nv_Vector2_zero)) return;
    tangent = nv_Vector2_normalize(tangent);

    double rat = nv_Vector2_dot(ra_perp, tangent);
    double rbt = nv_Vector2_dot(rb_perp, tangent);

    numer = -nv_Vector2_dot(rv, tangent);
    denom = a->invmass + b->invmass +
                    ((rat*rat * a->invinertia) + (rbt*rbt * b->invinertia));

    double jt = numer / denom;

    // Don't apply tiny tangential impulses
    if (nv_nearly_eq(jt, 0.0)) return;

    // Coulomb's law
    // Ff <= u * Fn
    nv_Vector2 impulse_fric;
    if (fabs(jt) <= j * sf)
        impulse_fric = nv_Vector2_muls(tangent, jt);
    else
        impulse_fric = nv_Vector2_muls(nv_Vector2_muls(tangent, -j), df);

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