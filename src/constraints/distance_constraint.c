/*

  This file is a part of the Nova Physics Engine
  project and distributed under the MIT license.

  Copyright © Kadir Aksoy
  https://github.com/kadir014/nova-physics

*/

#include "novaphysics/constraints/distance_constraint.h"
#include "novaphysics/space.h"


/**
 * @file constraints/distance_constraint.c
 * 
 * @brief Distance constraint solver.
 */


nvConstraint *nvDistanceConstraint_new(nvDistanceConstraintInitializer init) {
    if (init.length < 0.0) {
        nv_set_error("Distance constraint length can't be negative.");
        return NULL;
    }

    nvConstraint *cons = NV_NEW(nvConstraint);
    NV_MEM_CHECK(cons);

    if (!init.a && !init.b) {
        nv_set_error("Both bodies can't be NULL.");
        NV_FREE(cons);
        return NULL;
    }

    cons->a = init.a;
    cons->b = init.b;
    cons->type = nvConstraintType_DISTANCE;
    cons->ignore_collision = false;

    cons->def = NV_NEW(nvDistanceConstraint);
    if (!cons->def) {
        nv_set_error("Failed to allocate memory.");
        NV_FREE(cons);
        return NULL; 
    }
    nvDistanceConstraint *dist_cons = (nvDistanceConstraint *)cons->def;

    dist_cons->length = init.length;
    dist_cons->anchor_a = init.anchor_a;
    dist_cons->anchor_b = init.anchor_b;
    dist_cons->max_force = init.max_force;
    dist_cons->spring = init.spring;
    dist_cons->hertz = init.hertz;
    dist_cons->damping = init.damping;

    dist_cons->xanchor_a = nvVector2_zero;
    dist_cons->xanchor_b = nvVector2_zero;
    dist_cons->normal = nvVector2_zero;
    dist_cons->bias = 0.0;
    dist_cons->mass = 0.0;
    dist_cons->impulse = 0.0;
    dist_cons->max_impulse = 0.0;
    dist_cons->bias_rate = 0.0;
    dist_cons->mass_coeff = 0.0;
    dist_cons->impulse_coeff = 0.0;

    return cons;
}

nvRigidBody *nvDistanceConstraint_get_body_a(const nvConstraint *cons) {
    return cons->a;
}

nvRigidBody *nvDistanceConstraint_get_body_b(const nvConstraint *cons) {
    return cons->b;
}

void nvDistanceConstraint_set_length(nvConstraint *cons, nv_float length) {
    nvDistanceConstraint *dist_cons = (nvDistanceConstraint *)cons->def;
    dist_cons->length = length;
}

nv_float nvDistanceConstraint_get_length(const nvConstraint *cons) {
    nvDistanceConstraint *dist_cons = (nvDistanceConstraint *)cons->def;
    return dist_cons->length;
}

void nvDistanceConstraint_set_anchor_a(nvConstraint *cons, nvVector2 anchor_a) {
    nvDistanceConstraint *dist_cons = (nvDistanceConstraint *)cons->def;
    dist_cons->anchor_a = anchor_a;
}

nvVector2 nvDistanceConstraint_get_anchor_a(const nvConstraint *cons) {
    nvDistanceConstraint *dist_cons = (nvDistanceConstraint *)cons->def;
    return dist_cons->anchor_a;
}

void nvDistanceConstraint_set_anchor_b(nvConstraint *cons, nvVector2 anchor_b) {
    nvDistanceConstraint *dist_cons = (nvDistanceConstraint *)cons->def;
    dist_cons->anchor_b = anchor_b;
}

nvVector2 nvDistanceConstraint_get_anchor_b(const nvConstraint *cons) {
    nvDistanceConstraint *dist_cons = (nvDistanceConstraint *)cons->def;
    return dist_cons->anchor_b;
}

void nvDistanceConstraint_set_max_force(nvConstraint *cons, nv_float max_force) {
    nvDistanceConstraint *dist_cons = (nvDistanceConstraint *)cons->def;
    dist_cons->max_force = max_force;
}

nv_float nvDistanceConstraint_get_max_force(const nvConstraint *cons) {
    nvDistanceConstraint *dist_cons = (nvDistanceConstraint *)cons->def;
    return dist_cons->max_force;
}

void nvDistanceConstraint_set_spring(nvConstraint *cons, nv_bool spring) {
    nvDistanceConstraint *dist_cons = (nvDistanceConstraint *)cons->def;
    dist_cons->spring = spring;
}

nv_bool nvDistanceConstraint_get_spring(const nvConstraint *cons) {
    nvDistanceConstraint *dist_cons = (nvDistanceConstraint *)cons->def;
    return dist_cons->spring;
}

void nvDistanceConstraint_set_hertz(nvConstraint *cons, nv_float hertz) {
    nvDistanceConstraint *dist_cons = (nvDistanceConstraint *)cons->def;
    dist_cons->hertz = hertz;
}

nv_float nvDistanceConstraint_get_hertz(const nvConstraint *cons) {
    nvDistanceConstraint *dist_cons = (nvDistanceConstraint *)cons->def;
    return dist_cons->hertz;
}

void nvDistanceConstraint_set_damping(nvConstraint *cons, nv_float damping) {
    nvDistanceConstraint *dist_cons = (nvDistanceConstraint *)cons->def;
    dist_cons->damping = damping;
}

nv_float nvDistanceConstraint_get_damping(const nvConstraint *cons) {
    nvDistanceConstraint *dist_cons = (nvDistanceConstraint *)cons->def;
    return dist_cons->damping;
}

void nvDistanceConstraint_presolve(
    nvSpace *space,
    nvConstraint *cons,
    nv_float dt,
    nv_float inv_dt
) {
    nvDistanceConstraint *dist_cons = (nvDistanceConstraint *)cons->def;
    nvRigidBody *a = cons->a;
    nvRigidBody *b = cons->b;

    // Transformed anchor points
    nvVector2 rpa, rpb;
    nv_float invmass_a, invmass_b, invinertia_a, invinertia_b;

    // If a body is NULL count them as static bodies

    if (!a) {
        dist_cons->xanchor_a = nvVector2_zero;
        rpa = dist_cons->anchor_a;
        invmass_a = invinertia_a = 0.0;
    } else {
        dist_cons->xanchor_a = nvVector2_rotate(dist_cons->anchor_a, a->angle);
        rpa = nvVector2_add(dist_cons->xanchor_a, a->position);
        invmass_a = a->invmass;
        invinertia_a = a->invinertia;
    }

    if (!b) {
        dist_cons->xanchor_b = nvVector2_zero;
        rpb = dist_cons->anchor_b;
        invmass_b = invinertia_b = 0.0;
    } else {
        dist_cons->xanchor_b = nvVector2_rotate(dist_cons->anchor_b, b->angle);
        rpb = nvVector2_add(dist_cons->xanchor_b, b->position);
        invmass_b = b->invmass;
        invinertia_b = b->invinertia;
    }

    nvVector2 delta = nvVector2_sub(rpb, rpa);
    dist_cons->normal = nvVector2_normalize(delta);
    nv_float offset = nvVector2_len(delta) - dist_cons->length;

    // Baumgarte stabilization bias
    dist_cons->bias = space->settings.baumgarte * inv_dt * offset;

    // Constraint effective mass
    dist_cons->mass = 1.0 / nv_calc_mass_k(
        dist_cons->normal,
        dist_cons->xanchor_a, dist_cons->xanchor_b,
        invmass_a, invmass_b,
        invinertia_a, invinertia_b
    );

    dist_cons->max_impulse = dist_cons->max_force * dt;

    /*
        Soft-constraint formulation
        https://box2d.org/files/ErinCatto_SoftConstraints_GDC2011.pdf
        https://box2d.org/posts/2024/02/solver2d/
    */
    if (dist_cons->spring) {
        nv_float zeta = dist_cons->damping;
        nv_float omega = 2.0 * NV_PI * dist_cons->hertz;
        nv_float a1 = 2.0 * zeta + omega * (1.0 / inv_dt);
        nv_float a2 = (1.0 / inv_dt) * omega * a1;
        nv_float a3 = 1.0 / (1.0 + a2);
        dist_cons->bias_rate = omega / a1;
        dist_cons->mass_coeff = a2 * a3;
        dist_cons->impulse_coeff = a3;
    }
    else {
        dist_cons->bias_rate = 1.0;
        dist_cons->mass_coeff = 1.0;
        dist_cons->impulse_coeff = 0.0;
    }
}

void nvDistanceConstraint_warmstart(nvSpace *space, nvConstraint *cons) {
    nvDistanceConstraint *dist_cons = (nvDistanceConstraint *)cons->def;
    nvRigidBody *a = cons->a;
    nvRigidBody *b = cons->b;

    if (space->settings.warmstarting) {
        nvVector2 impulse = nvVector2_mul(dist_cons->normal, dist_cons->impulse);

        if (a) nvRigidBody_apply_impulse(cons->a, nvVector2_neg(impulse), dist_cons->xanchor_a);
        if (b) nvRigidBody_apply_impulse(cons->b, impulse, dist_cons->xanchor_b);
    }
    else {
        dist_cons->impulse = 0.0;
    }
}

void nvDistanceConstraint_solve(nvConstraint *cons) {
    nvDistanceConstraint *dist_cons = (nvDistanceConstraint *)cons->def;
    nvRigidBody *a = cons->a;
    nvRigidBody *b = cons->b;

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
        linear_velocity_a, angular_velocity_a, dist_cons->xanchor_a,
        linear_velocity_b, angular_velocity_b, dist_cons->xanchor_b
    );

    nv_float vn = nvVector2_dot(rv, dist_cons->normal);

    // Constraint impulse magnitude
    nv_float lambda = (dist_cons->bias * dist_cons->bias_rate + vn);
    lambda *= dist_cons->mass * -dist_cons->mass_coeff;
    lambda -= dist_cons->impulse_coeff * dist_cons->impulse;

    // Accumulate impulse
    nv_float limit = dist_cons->max_impulse;
    nv_float lambda0 = dist_cons->impulse;
    dist_cons->impulse = nv_fclamp(lambda0 + lambda, -limit, limit);
    lambda = dist_cons->impulse - lambda0;

    nvVector2 impulse = nvVector2_mul(dist_cons->normal, lambda);

    // Apply constraint impulse
    if (a) nvRigidBody_apply_impulse(a, nvVector2_neg(impulse), dist_cons->xanchor_a);
    if (b) nvRigidBody_apply_impulse(b, impulse, dist_cons->xanchor_b);
}