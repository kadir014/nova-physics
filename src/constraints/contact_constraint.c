/*

  This file is a part of the Nova Physics Engine
  project and distributed under the MIT license.

  Copyright © Kadir Aksoy
  https://github.com/kadir014/nova-physics

*/

#include "novaphysics/internal.h"
#include "novaphysics/constraints/contact_constraint.h"
#include "novaphysics/vector.h"
#include "novaphysics/math.h"
#include "novaphysics/constants.h"
#include "novaphysics/space.h"


/**
 * @file constraints/contact_constraint.c
 * 
 * @brief Contact constraint solver functions.
 */


void nv_contact_presolve(
    nvSpace *space,
    nvPersistentContactPair *pcp,
    nv_float inv_dt
) {
    NV_TRACY_ZONE_START;

    nvRigidBody *a = pcp->body_a;
    nvRigidBody *b = pcp->body_b;
    nvVector2 normal = pcp->normal;
    nvVector2 tangent = nvVector2_perpr(normal);

    // Mixed restitution
    nv_float e = nv_mix_coefficients(
        a->material.restitution,
        b->material.restitution,
        space->settings.restitution_mix
    );

    // Mixed friction
    nv_float friction = nv_mix_coefficients(
        a->material.friction,
        b->material.friction,
        space->settings.friction_mix
    );

    for (size_t i = 0; i < pcp->contact_count; i++) {
        nvContact *contact = &pcp->contacts[i];
        if (contact->separation > 0.0) continue;
        nvContactSolverInfo *solver_info = &contact->solver_info;

        solver_info->friction = friction;

        // Relative velocity at contact
        nvVector2 rv = nv_calc_relative_velocity(
            a->linear_velocity, a->angular_velocity, contact->anchor_a,
            b->linear_velocity, b->angular_velocity, contact->anchor_b
        );

        // Restitution * normal velocity at first impact
        nv_float vn = nvVector2_dot(rv, normal);

        // Restitution bias
        solver_info->velocity_bias = 0.0;
        if (vn < -1.0) {
            solver_info->velocity_bias = e * vn;
        }

        // Effective masses
        solver_info->mass_normal = 1.0 / nv_calc_mass_k(
            normal,
            contact->anchor_a, contact->anchor_b,
            a->invmass, b->invmass,
            a->invinertia, b->invinertia
        );
        solver_info->mass_tangent = 1.0 / nv_calc_mass_k(
            tangent,
            contact->anchor_a, contact->anchor_b,
            a->invmass, b->invmass,
            a->invinertia, b->invinertia
        );

        if (space->settings.contact_position_correction == nvContactPositionCorrection_BAUMGARTE) {
            // Position error is fed back to the velocity constraint as a bias value
            nv_float correction = nv_fmin(contact->separation + space->settings.penetration_slop, 0.0);
            solver_info->position_bias = space->settings.baumgarte * inv_dt * correction;
        }
        else if (space->settings.contact_position_correction == nvContactPositionCorrection_NGS) {
            // contact->position_bias = res->depth > 0.0f ? 1.0f : 0.0f;
            // contact->a_angle0 = a->angle;
            // contact->b_angle0 = b->angle;
            // contact->adjusted_depth = res->depth - nvVector2_dot(nvVector2_sub(contact->rb, contact->ra), normal);
        }
    }

    NV_TRACY_ZONE_END;
}

void nv_contact_warmstart(nvSpace *space, nvPersistentContactPair *pcp) {
    NV_TRACY_ZONE_START;

    nvRigidBody *a = pcp->body_a;
    nvRigidBody *b = pcp->body_b;
    nvVector2 normal = pcp->normal;
    nvVector2 tangent = nvVector2_perpr(normal);

    for (size_t i = 0; i < pcp->contact_count; i++) {
        nvContact *contact = &pcp->contacts[i];
        if (contact->separation > 0.0) continue;
        // No need to apply warmstarting if this contact is just created
        if (!contact->is_persisted) continue;
        nvContactSolverInfo *solver_info = &contact->solver_info;

        if (space->settings.warmstarting) {
            nvVector2 impulse = nvVector2_add(
                nvVector2_mul(normal, solver_info->normal_impulse),
                nvVector2_mul(tangent, solver_info->tangent_impulse)
            );
            
            nvRigidBody_apply_impulse(a, nvVector2_neg(impulse), contact->anchor_a);
            nvRigidBody_apply_impulse(b, impulse, contact->anchor_b);
        }
        else {
            solver_info->normal_impulse = 0.0;
            solver_info->tangent_impulse = 0.0;
        }
    }

    NV_TRACY_ZONE_END;
}

void nv_contact_solve_velocity(nvPersistentContactPair *pcp) {
    NV_TRACY_ZONE_START;

    nvRigidBody *a = pcp->body_a;
    nvRigidBody *b = pcp->body_b;
    nvVector2 normal = pcp->normal;
    nvVector2 tangent = nvVector2_perpr(normal);

    /*
        Nova uses Sequential Impulses / PGS for all constraints.
        Check out https://box2d.org/files/ErinCatto_SequentialImpulses_GDC2006.pdf
    */

    /*
        In an iterative solver what is applied the last affects the result more.
        So we solve normal impulse after tangential impulse because
        non-penetration is more important.
    */

    // Solve friction
    for (size_t i = 0; i < pcp->contact_count; i++) {
        nvContact *contact = &pcp->contacts[i];
        //if (contact->separation > 0.0) continue;
        nvContactSolverInfo *solver_info = &contact->solver_info;

        // Don't bother calculating friction if the coefficent is 0
        if (solver_info->friction == 0.0) continue;

        // Relative velocity at contact
        nvVector2 rv = nv_calc_relative_velocity(
            a->linear_velocity, a->angular_velocity, contact->anchor_a,
            b->linear_velocity, b->angular_velocity, contact->anchor_b
        );

        // Tangential impulse magnitude
        nv_float lambda = -nvVector2_dot(rv, tangent) * solver_info->mass_tangent;

        // Accumulate tangential impulse
        nv_float f = solver_info->normal_impulse * solver_info->friction;
        nv_float lambda0 = solver_info->tangent_impulse;
        // Clamp lambda between friction limits
        solver_info->tangent_impulse = nv_fmax(-f, nv_fmin(lambda0 + lambda, f));
        lambda = solver_info->tangent_impulse - lambda0;

        nvVector2 impulse = nvVector2_mul(tangent, lambda);

        // Apply tangential impulse
        nvRigidBody_apply_impulse(a, nvVector2_neg(impulse), contact->anchor_a);
        nvRigidBody_apply_impulse(b, impulse, contact->anchor_b);
    }

    // Solve penetration
    for (size_t i = 0; i < pcp->contact_count; i++) {
        nvContact *contact = &pcp->contacts[i];
        if (contact->separation > 0.0) continue;
        nvContactSolverInfo *solver_info = &contact->solver_info;

        // Relative velocity at contact
        nvVector2 rv = nv_calc_relative_velocity(
            a->linear_velocity, a->angular_velocity, contact->anchor_a,
            b->linear_velocity, b->angular_velocity, contact->anchor_b
        );

        nv_float vn = nvVector2_dot(rv, normal);

        // Normal impulse magnitude
        nv_float lambda = -(vn + solver_info->velocity_bias + solver_info->position_bias);
        lambda *= solver_info->mass_normal;

        // Accumulate normal impulse
        nv_float lambda0 = solver_info->normal_impulse;
        // Clamp lambda because we only want to solve penetration
        solver_info->normal_impulse = nv_fmax(lambda0 + lambda, 0.0);
        lambda = solver_info->normal_impulse - lambda0;

        nvVector2 impulse = nvVector2_mul(normal, lambda);

        // Apply normal impulse
        nvRigidBody_apply_impulse(a, nvVector2_neg(impulse), contact->anchor_a);
        nvRigidBody_apply_impulse(b, impulse, contact->anchor_b);
    }

    NV_TRACY_ZONE_END;
}

void nv_contact_solve_position(nvPersistentContactPair *pcp) {
    // TODO: Finish the NGS iterations early if there is no collision?

    NV_TRACY_ZONE_START;

    // nvRigidBody *a = res->a;
    // nvRigidBody *b = res->b;

    // for (size_t i = 0; i < res->contact_count; i++) {
    //     nvContact contact = res->contacts[i];

    //     nvVector2 ra = nvVector2_rotate(contact.ra, a->angle - contact.a_angle0);
    //     nvVector2 rb = nvVector2_rotate(contact.rb, b->angle - contact.b_angle0);

    //     // Current separation
    //     nvVector2 d = nvVector2_add(nvVector2_sub(b->position, a->position), nvVector2_sub(rb, ra));
    //     nv_float depth = nvVector2_dot(d, res->normal) - res->depth;

    //     // nv_float mass_normal = nv_calc_mass_k(
    //     //     res->normal,
    //     //     ra, rb,
    //     //     a->invmass, b->invmass,
    //     //     a->invinertia, b->invinertia
    //     // );

    //     // if (mass_normal == 0.0) printf("a\n");

    //     nv_float rna = nvVector2_cross(ra, res->normal);
    //     nv_float rnb = nvVector2_cross(rb, res->normal);
    //     nv_float mass_normal = a->invmass + b->invmass + a->invinertia * rna * rna + b->invinertia * rnb * rnb;

    //     nv_float correction = nv_fmin(0.0, depth + NV_POSITION_CORRECTION_SLOP);
    //     nv_float position_bias = -NV_BAUMGARTE * correction;

    //     // Normal pseudo lambda
    //     nv_float jp = position_bias / mass_normal;

    //     nvVector2 impulse = nvVector2_mul(res->normal, jp);

    //     // Apply pseudo-impulse
    //     a->position = nvVector2_sub(a->position, nvVector2_mul(impulse, a->invmass));
    //     a->angle -= nvVector2_cross(ra, impulse) * a->invinertia;

    //     b->position = nvVector2_add(b->position, nvVector2_mul(impulse, b->invmass));
    //     b->angle += nvVector2_cross(rb, impulse) * b->invinertia;
    // }

    NV_TRACY_ZONE_END;
}