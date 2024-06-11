/*

  This file is a part of the Nova Physics Engine
  project and distributed under the MIT license.

  Copyright © Kadir Aksoy
  https://github.com/kadir014/nova-physics

*/

#include "novaphysics/constraints/hinge_constraint.h"
#include "novaphysics/space.h"


/**
 * @file constraints/hinge_constraint.c
 * 
 * @brief Hinge constraint solver.
 */


nvConstraint *nvHingeConstraint_new(nvHingeConstraintInitializer init) {
    nvConstraint *cons = NV_NEW(nvConstraint);
    NV_MEM_CHECK(cons);

    if (!init.a && !init.b) {
        nv_set_error("Both bodies can't be NULL.");
        free(cons);
        return NULL;
    }

    cons->a = init.a;
    cons->b = init.b;
    cons->type = nvConstraintType_HINGE;

    cons->def = NV_NEW(nvHingeJoint);
    if (!cons->def) {
        nv_set_error("Failed to allocate memory.");
        free(cons);
        return NULL; 
    }
    nvHingeJoint *hinge_cons = (nvHingeJoint *)cons->def;

    hinge_cons->anchor = init.anchor;
    hinge_cons->enable_limits = init.enable_limits;
    hinge_cons->lower_limit = init.lower_limit;
    hinge_cons->upper_limit = init.upper_limit;
    hinge_cons->max_force = init.max_force;
    hinge_cons->angle = 0.0;

    nv_float angle_a, angle_b;
    if (init.a) {
        hinge_cons->anchor_a = nvVector2_sub(init.anchor, init.a->position);
        angle_a = init.a->angle;
    }
    else {
        hinge_cons->anchor_a = init.anchor;
        angle_a = 0.0;
    }
    if (init.b) {
        hinge_cons->anchor_b = nvVector2_sub(init.anchor, init.b->position);
        angle_b = init.b->angle;
    }
    else {
        hinge_cons->anchor_b = init.anchor;
        angle_b = 0.0;
    }

    hinge_cons->reference_angle = angle_b - angle_a;
    hinge_cons->lower_impulse = 0.0;
    hinge_cons->upper_impulse = 0.0;
    hinge_cons->axial_mass = 0.0;
    hinge_cons->xanchor_a = nvVector2_zero;
    hinge_cons->xanchor_b = nvVector2_zero;
    hinge_cons->normal = nvVector2_zero;
    hinge_cons->bias = 0.0;
    hinge_cons->mass = 0.0;
    hinge_cons->impulse = 0.0;
    hinge_cons->max_impulse = 0.0;

    return cons;
}

void nvHingeConstraint_set_anchor(nvConstraint *cons, nvVector2 anchor) {
    nvHingeJoint *hinge_cons = (nvHingeJoint *)cons->def;
    hinge_cons->anchor = anchor;

    if (cons->a) {
        hinge_cons->anchor_a = nvVector2_sub(hinge_cons->anchor, cons->a->position);
    }
    else {
        hinge_cons->anchor_a = hinge_cons->anchor;
    }
    if (cons->b) {
        hinge_cons->anchor_b = nvVector2_sub(hinge_cons->anchor, cons->b->position);
    }
    else {
        hinge_cons->anchor_b = hinge_cons->anchor;
    }
}

nvVector2 nvHingeConstraint_get_anchor(const nvConstraint *cons) {
    nvHingeJoint *hinge_cons = (nvHingeJoint *)cons->def;
    return hinge_cons->anchor;
}

void nvHingeConstraint_set_limits(nvConstraint *cons, nv_bool limits) {
    nvHingeJoint *hinge_cons = (nvHingeJoint *)cons->def;
    hinge_cons->enable_limits = limits;
}

nv_bool nvHingeConstraint_get_limits(const nvConstraint *cons) {
    nvHingeJoint *hinge_cons = (nvHingeJoint *)cons->def;
    return hinge_cons->enable_limits;
}

void nvHingeConstraint_set_upper_limit(nvConstraint *cons, nv_float upper_limit) {
    nvHingeJoint *hinge_cons = (nvHingeJoint *)cons->def;
    hinge_cons->upper_limit = upper_limit;
}

nv_float nvHingeConstraint_get_upper_limit(const nvConstraint *cons) {
    nvHingeJoint *hinge_cons = (nvHingeJoint *)cons->def;
    return hinge_cons->upper_limit;
}

void nvHingeConstraint_set_lower_limit(nvConstraint *cons, nv_float lower_limit) {
    nvHingeJoint *hinge_cons = (nvHingeJoint *)cons->def;
    hinge_cons->lower_limit = lower_limit;
}

nv_float nvHingeConstraint_get_lower_limit(const nvConstraint *cons) {
    nvHingeJoint *hinge_cons = (nvHingeJoint *)cons->def;
    return hinge_cons->lower_limit;
}

void nvHingeConstraint_set_max_force(nvConstraint *cons, nv_float max_force) {
    nvHingeJoint *hinge_cons = (nvHingeJoint *)cons->def;
    hinge_cons->max_force = max_force;
}

nv_float nvHingeConstraint_get_max_force(const nvConstraint *cons) {
    nvHingeJoint *hinge_cons = (nvHingeJoint *)cons->def;
    return hinge_cons->max_force;
}

void nvHingeConstraint_presolve(
    nvSpace *space,
    nvConstraint *cons,
    nv_float dt,
    nv_float inv_dt
) {
    nvHingeJoint *hinge_cons = (nvHingeJoint *)cons->def;
    nvRigidBody *a = cons->a;
    nvRigidBody *b = cons->b;

    // Transformed anchor points
    nvVector2 rpa, rpb;
    nv_float invmass_a, invmass_b, invinertia_a, invinertia_b;

    // If a body is NULL count them as static bodies

    if (!a) {
        hinge_cons->xanchor_a = nvVector2_zero;
        rpa = hinge_cons->anchor_a;
        invmass_a = invinertia_a = 0.0;
    } else {
        hinge_cons->xanchor_a = nvVector2_rotate(hinge_cons->anchor_a, a->angle);
        rpa = nvVector2_add(hinge_cons->xanchor_a, a->position);
        invmass_a = a->invmass;
        invinertia_a = a->invinertia;
    }

    if (!b) {
        hinge_cons->xanchor_b = nvVector2_zero;
        rpb = hinge_cons->anchor_b;
        invmass_b = invinertia_b = 0.0;
    } else {
        hinge_cons->xanchor_b = nvVector2_rotate(hinge_cons->anchor_b, b->angle);
        rpb = nvVector2_add(hinge_cons->xanchor_b, b->position);
        invmass_b = b->invmass;
        invinertia_b = b->invinertia;
    }

    // If delta is 0 point constraint is ensured
    nvVector2 delta = nvVector2_sub(rpb, rpa);
    if (nvVector2_len2(delta) == 0.0) hinge_cons->normal = nvVector2_zero;
    else hinge_cons->normal = nvVector2_normalize(delta);
    nv_float offset = nvVector2_len(delta);

    // Baumgarte position correction bias
    hinge_cons->bias = space->settings.baumgarte * inv_dt * offset;

    // Point constraint effective mass
    hinge_cons->mass = 1.0 / nv_calc_mass_k(
        hinge_cons->normal,
        hinge_cons->xanchor_a, hinge_cons->xanchor_b,
        invmass_a, invmass_b,
        invinertia_a, invinertia_b
    );

    hinge_cons->max_impulse = hinge_cons->max_force * dt;

    hinge_cons->axial_mass = 1.0 / (invinertia_a + invinertia_b);

    nv_float angle_a, angle_b;
    if (a) angle_a = a->angle;
    else angle_a = 0.0;
    if (b) angle_b = b->angle;
    else angle_b = 0.0;

    hinge_cons->angle = angle_b - angle_a - hinge_cons->reference_angle;
}

void nvHingeConstraint_warmstart(nvSpace *space, nvConstraint *cons) {
    nvHingeJoint *hinge_cons = (nvHingeJoint *)cons->def;
    nvRigidBody *a = cons->a;
    nvRigidBody *b = cons->b;

    if (space->settings.warmstarting) {
        nvVector2 impulse = nvVector2_mul(hinge_cons->normal, hinge_cons->impulse);
        nv_float axial_impulse = hinge_cons->lower_impulse - hinge_cons->upper_impulse;

        if (a) {
            nvRigidBody_apply_impulse(a, nvVector2_neg(impulse), hinge_cons->xanchor_a);
            a->angular_velocity -= a->invinertia * axial_impulse;
        } 
        if (b) {
            nvRigidBody_apply_impulse(b, impulse, hinge_cons->xanchor_b);
            b->angular_velocity += b->invinertia * axial_impulse;
        }
        
    }
    else {
        hinge_cons->impulse = 0.0;
        hinge_cons->upper_impulse = 0.0;
        hinge_cons->lower_impulse = 0.0;
    }
}

void nvHingeConstraint_solve(nvConstraint *cons, nv_float inv_dt) {
    nvHingeJoint *hinge_cons = (nvHingeJoint *)cons->def;
    nvRigidBody *a = cons->a;
    nvRigidBody *b = cons->b;

    // Solve angular limits
    if (hinge_cons->enable_limits) {
        nv_float c, wr, wa, wb, lambda, lambda0;

        if (a) wa = a->angular_velocity;
        else wa = 0.0;
        if (b) wb = b->angular_velocity;
        else wb = 0.0;

        // Solve lower limit
        c = hinge_cons->angle - hinge_cons->lower_limit;
        wr = wb - wa;
        lambda = -hinge_cons->axial_mass * (wr + nv_fmax(c, 0.0) * inv_dt);

        // Accumulate lower impulse
        lambda0 = hinge_cons->lower_impulse;
        hinge_cons->lower_impulse = nv_fmax(hinge_cons->lower_impulse + lambda, 0.0);
        lambda = hinge_cons->lower_impulse - lambda0;

        // Apply lower impulse
        if (a) a->angular_velocity -= lambda * a->invinertia;
        if (b) b->angular_velocity += lambda * b->invinertia;

        // Solve upper limmit
        c = hinge_cons->upper_limit - hinge_cons->angle;
        wr = wa - wb;
        lambda = -hinge_cons->axial_mass * (wr + nv_fmax(c, 0.0) * inv_dt);

        // Accumulate upper impulse
        lambda0 = hinge_cons->upper_impulse;
        hinge_cons->upper_impulse = nv_fmax(hinge_cons->upper_impulse + lambda, 0.0);
        lambda = hinge_cons->upper_impulse - lambda0;

        // Apply upper impulse
        if (a) a->angular_velocity += lambda * a->invinertia;
        if (b) b->angular_velocity -= lambda * b->invinertia;
    }

    // Solve point constraint

    nvVector2 linear_velocity_a, linear_velocity_b;
    nv_float angular_velocity_a, angular_velocity_b;

    if (!a) {
        linear_velocity_a = nvVector2_zero;
        angular_velocity_a = 0.0;
    } else {
        linear_velocity_a = a->linear_velocity;
        angular_velocity_a = a->angular_velocity;
    }

    if (!b) {
        linear_velocity_b = nvVector2_zero;
        angular_velocity_b = 0.0;
    } else {
        linear_velocity_b = b->linear_velocity;
        angular_velocity_b = b->angular_velocity;
    }

    nvVector2 rv = nv_calc_relative_velocity(
        linear_velocity_a, angular_velocity_a, hinge_cons->xanchor_a,
        linear_velocity_b, angular_velocity_b, hinge_cons->xanchor_b
    );

    nv_float vn = nvVector2_dot(rv, hinge_cons->normal);

    // Point constraint impulse magnitude
    nv_float lambda = -(hinge_cons->bias + vn) * hinge_cons->mass;

    // Accumulate impulse
    nv_float limit = hinge_cons->max_impulse;
    nv_float lambda0 = hinge_cons->impulse;
    hinge_cons->impulse = nv_fclamp(lambda0 + lambda, -limit, limit);
    lambda = hinge_cons->impulse - lambda0;

    nvVector2 impulse = nvVector2_mul(hinge_cons->normal, lambda);

    // Apply point constraint impulse
    if (a) nvRigidBody_apply_impulse(a, nvVector2_neg(impulse), hinge_cons->xanchor_a);
    if (b) nvRigidBody_apply_impulse(b, impulse, hinge_cons->xanchor_b);
}