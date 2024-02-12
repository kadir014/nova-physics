/*

  This file is a part of the Nova Physics Engine
  project and distributed under the MIT license.

  Copyright © Kadir Aksoy
  https://github.com/kadir014/nova-physics

*/

#include <math.h>
#include "novaphysics/internal.h"
#include "novaphysics/contact_solver.h"
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
 * @file contact_solver.c
 * 
 * @brief Contact solver functions.
 */


void nv_presolve_contact(
    nvSpace *space,
    nvResolution *res,
    nv_float inv_dt
) {
    NV_TRACY_ZONE_START;

    nvBody *a = res->a;
    nvBody *b = res->b;
    nvVector2 normal = res->normal;
    nvVector2 tangent = nvVector2_perpr(normal);

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
        nvContact *contact = &res->contacts[i];

        contact->ra = nvVector2_sub(contact->position, a->position);
        contact->rb = nvVector2_sub(contact->position, b->position);

        // Relative velocity at contact
        nvVector2 rv = nv_calc_relative_velocity(
            a->linear_velocity, a->angular_velocity, contact->ra,
            b->linear_velocity, b->angular_velocity, contact->rb
        );

        // Restitution * normal velocity at first impact
        nv_float cn = nvVector2_dot(rv, normal);

        // Restitution bias
        contact->velocity_bias = 0.0;
        if (cn < -1.0) {
            contact->velocity_bias = e * cn;
        }

        contact->mass_normal = 1.0 / nv_calc_mass_k(
            normal,
            contact->ra, contact->rb,
            a->invmass, b->invmass,
            a->invinertia, b->invinertia
        );

        contact->mass_tangent = 1.0 / nv_calc_mass_k(
            tangent,
            contact->ra, contact->rb,
            a->invmass, b->invmass,
            a->invinertia, b->invinertia
        );

        if (space->position_correction == nvPositionCorrection_BAUMGARTE) {
            // Position error is fed back to the velocity constraint as a bias 
            // value in the Baumgarte stabilization method.
            nv_float correction = nv_fmin(-res->depth + NV_POSITION_CORRECTION_SLOP, 0.0);
            contact->position_bias = NV_BAUMGARTE * correction * inv_dt;
        }
        else if (space->position_correction == nvPositionCorrection_NGS) {
            contact->position_bias = res->depth > 0.0f ? 1.0f : 0.0f;
            contact->a_angle0 = a->angle;
            contact->b_angle0 = b->angle;
            contact->adjusted_depth = res->depth - nvVector2_dot(nvVector2_sub(contact->rb, contact->ra), normal);
        }
    }

    NV_TRACY_ZONE_END;
}

void nv_warmstart(nvSpace *space, nvResolution *res) {
    NV_TRACY_ZONE_START;

    nvBody *a = res->a;
    nvBody *b = res->b;
    nvVector2 normal = res->normal;
    nvVector2 tangent = nvVector2_perpr(normal);

    for (size_t i = 0; i < res->contact_count; i++) {
        nvContact *contact = &res->contacts[i];

        if (space->warmstarting && res->state == nvResolutionState_NORMAL) {
            nvVector2 impulse = nvVector2_add(
                nvVector2_mul(normal, contact->jn),
                nvVector2_mul(tangent, contact->jt)
            );
            
            nvBody_apply_impulse(a, nvVector2_neg(impulse), contact->ra);
            nvBody_apply_impulse(b, impulse, contact->rb);
        }

        if (!space->warmstarting) {
            contact->jn = 0.0;
            contact->jt = 0.0;
        }
    }

    NV_TRACY_ZONE_END;
}

void nv_solve_velocity(nvResolution *res) {
    NV_TRACY_ZONE_START;

    nvBody *a = res->a;
    nvBody *b = res->b;
    nvVector2 normal = res->normal;
    nvVector2 tangent = nvVector2_perpr(normal);
    size_t i;

    // In an iterative solver what is applied the last affects the result more.
    // So we solve normal impulse after tangential impulse because
    // non-penetration is more important.

    // Solve friction
    for (i = 0; i < res->contact_count; i++) {
        // Don't bother calculating friction if the coefficent is 0
        if (res->friction == 0.0) continue;

        nvContact *contact = &res->contacts[i];

        // Relative velocity at contact
        nvVector2 rv = nv_calc_relative_velocity(
            a->linear_velocity, a->angular_velocity, contact->ra,
            b->linear_velocity, b->angular_velocity, contact->rb
        );

        // Tangential lambda (tangential impulse magnitude)
        nv_float jt = -nvVector2_dot(rv, tangent) * contact->mass_tangent;

        // Accumulate tangential impulse
        nv_float f = contact->jn * res->friction;
        nv_float jt0 = contact->jt;
        // Clamp lambda between friction limits
        contact->jt = nv_fmax(-f, nv_fmin(jt0 + jt, f));
        jt = contact->jt - jt0;

        nvVector2 impulse = nvVector2_mul(tangent, jt);

        // Apply tangential impulse
        nvBody_apply_impulse(a, nvVector2_neg(impulse), contact->ra);
        nvBody_apply_impulse(b, impulse, contact->rb);
    }

    // Solve penetration
    for (i = 0; i < res->contact_count; i++) {
        nvContact *contact = &res->contacts[i];

        // Relative velocity at contact
        nvVector2 rv = nv_calc_relative_velocity(
            a->linear_velocity, a->angular_velocity, contact->ra,
            b->linear_velocity, b->angular_velocity, contact->rb
        );

        nv_float cn = nvVector2_dot(rv, normal);

        // Normal lambda (normal impulse magnitude)
        //nv_float jn = -(cn + contact->velocity_bias + contact->position_bias) * contact->mass_normal;
        
        //-cp->normalMass * (vn + cp->biasCoefficient * cp->separation * inv_dt)
        nv_float jn = -(cn + contact->position_bias * -res->depth) * contact->mass_normal;

        // Accumulate normal impulse
        nv_float jn0 = contact->jn;
        // Clamp lambda because we only want to solve penetration
        contact->jn = nv_fmax(jn0 + jn, 0.0);
        jn = contact->jn - jn0;

        nvVector2 impulse = nvVector2_mul(normal, jn);

        // Apply normal impulse
        nvBody_apply_impulse(a, nvVector2_neg(impulse), contact->ra);
        nvBody_apply_impulse(b, impulse, contact->rb);
    }

    NV_TRACY_ZONE_END;
}

void nv_solve_position(nvResolution *res) {
    // TODO: Finish the NGS iterations early if there is no collision?

    NV_TRACY_ZONE_START;

    nvBody *a = res->a;
    nvBody *b = res->b;

    for (size_t i = 0; i < res->contact_count; i++) {
        nvContact contact = res->contacts[i];

        nvVector2 ra = nvVector2_rotate(contact.ra, a->angle - contact.a_angle0);
        nvVector2 rb = nvVector2_rotate(contact.rb, b->angle - contact.b_angle0);

        // Current separation
        nvVector2 d = nvVector2_add(nvVector2_sub(b->position, a->position), nvVector2_sub(rb, ra));
        nv_float depth = nvVector2_dot(d, res->normal) - res->depth;

        // nv_float mass_normal = nv_calc_mass_k(
        //     res->normal,
        //     ra, rb,
        //     a->invmass, b->invmass,
        //     a->invinertia, b->invinertia
        // );

        // if (mass_normal == 0.0) printf("a\n");

        nv_float rna = nvVector2_cross(ra, res->normal);
        nv_float rnb = nvVector2_cross(rb, res->normal);
        nv_float mass_normal = a->invmass + b->invmass + a->invinertia * rna * rna + b->invinertia * rnb * rnb;

        nv_float correction = nv_fmin(0.0, depth + NV_POSITION_CORRECTION_SLOP);
        nv_float position_bias = -NV_BAUMGARTE * correction;

        // Normal pseudo lambda
        nv_float jp = position_bias / mass_normal;

        nvVector2 impulse = nvVector2_mul(res->normal, jp);

        // Apply pseudo-impulse
        a->position = nvVector2_sub(a->position, nvVector2_mul(impulse, a->invmass));
        a->angle -= nvVector2_cross(ra, impulse) * a->invinertia;

        b->position = nvVector2_add(b->position, nvVector2_mul(impulse, b->invmass));
        b->angle += nvVector2_cross(rb, impulse) * b->invinertia;
    }

    NV_TRACY_ZONE_END;
}