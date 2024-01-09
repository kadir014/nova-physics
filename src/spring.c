/*

  This file is a part of the Nova Physics Engine
  project and distributed under the MIT license.

  Copyright © Kadir Aksoy
  https://github.com/kadir014/nova-physics

*/

#include "novaphysics/spring.h"
#include "novaphysics/space.h"


/**
 * @file spring.c
 * 
 * @brief Damped spring implementation.
 */


nvConstraint *nvSpring_new(
    nvBody *a,
    nvBody *b,
    nvVector2 anchor_a,
    nvVector2 anchor_b,
    nv_float length,
    nv_float stiffness,
    nv_float damping
) {
    nvConstraint *cons = NV_NEW(nvConstraint);
    if (!cons) return NULL;

    cons->a = a;
    cons->b = b;
    cons->type = nvConstraintType_SPRING;

    cons->def = (void *)NV_NEW(nvSpring);
    if (!cons->def) return NULL;
    nvSpring *spring = (nvSpring *)cons->def;

    spring->length = length;
    spring->stiffness = stiffness;
    spring->damping = damping;

    spring->anchor_a = anchor_a;
    spring->anchor_b = anchor_b;
    spring->ra = nvVector2_zero;
    spring->rb = nvVector2_zero;
    spring->normal = nvVector2_zero;
    spring->mass = 0.0;
    spring->jc = 0.0;

    return cons;
}

void nvSpring_presolve(
    nvSpace *space,
    nvConstraint *cons,
    nv_float inv_dt
) {
    nvSpring *spring = (nvSpring *)cons->def;
    nvBody *a = cons->a;
    nvBody *b = cons->b;

    // Transform anchor points
    nvVector2 rpa, rpb;
    nv_float invmass_a, invmass_b, invinertia_a, invinertia_b;

    if (a == NULL) {
        spring->ra = nvVector2_zero;
        rpa = spring->anchor_a;
        invmass_a = invinertia_a = 0.0;
    } else {
        spring->ra = nvVector2_rotate(spring->anchor_a, a->angle);
        rpa = nvVector2_add(spring->ra, a->position);
        invmass_a = a->invmass;
        invinertia_a = a->invinertia;
    }

    if (b == NULL) {
        spring->rb = nvVector2_zero;
        rpb = spring->anchor_b;
        invmass_b = invinertia_b = 0.0;
    } else {
        spring->rb = nvVector2_rotate(spring->anchor_b, b->angle);
        rpb = nvVector2_add(spring->rb, b->position);
        invmass_b = b->invmass;
        invinertia_b = b->invinertia;
    }

    nvVector2 delta = nvVector2_sub(rpb, rpa);
    spring->normal = nvVector2_normalize(delta);
    nv_float dist = nvVector2_len(delta);

    // Constraint effective mass
    nv_float mass_k = nv_calc_mass_k(
        spring->normal,
        spring->ra, spring->rb,
        invmass_a, invmass_b,
        invinertia_a, invinertia_b
    );
    spring->mass = 1.0 / mass_k;
    
    spring->target_vel = 0.0;
    spring->damping_bias = 1.0 - nv_exp(-spring->damping / inv_dt * mass_k);

    // Apply spring force
    nv_float spring_force = (spring->length - dist) * spring->stiffness;

    spring->jc = spring_force / inv_dt;
    nvVector2 spring_impulse = nvVector2_mul(spring->normal, spring->jc);

    if (a) nvBody_apply_impulse(a, nvVector2_neg(spring_impulse), spring->ra);
    if (b) nvBody_apply_impulse(b, spring_impulse, spring->rb);
}

void nvSpring_solve(nvConstraint *cons) {
    nvSpring *spring = (nvSpring *)cons->def;
    nvBody *a = cons->a;
    nvBody *b = cons->b;

    nvVector2 linear_velocity_a, linear_velocity_b;
    nv_float angular_velocity_a, angular_velocity_b;

    if (a == NULL) {
        linear_velocity_a = nvVector2_zero;
        angular_velocity_a = 0.0;
    } else {
        linear_velocity_a = a->linear_velocity;
        angular_velocity_a = a->angular_velocity;
    }

    if (b == NULL) {
        linear_velocity_b = nvVector2_zero;
        angular_velocity_b = 0.0;
    } else {
        linear_velocity_b = b->linear_velocity;
        angular_velocity_b = b->angular_velocity;
    }

    // Relative velocity
    nvVector2 rv = nv_calc_relative_velocity(
        linear_velocity_a, angular_velocity_a, spring->ra,
        linear_velocity_b, angular_velocity_b, spring->rb
    );

    nv_float rn = nvVector2_dot(rv, spring->normal);

    // Velocity loss from drag
    nv_float damped = (spring->target_vel - rn) * spring->damping_bias;
    spring->target_vel = rn + damped;

    nv_float jc_damp = damped * spring->mass;
    spring->jc += jc_damp;

    nvVector2 impulse_damp = nvVector2_mul(spring->normal, jc_damp);

    if (a) nvBody_apply_impulse(a, nvVector2_neg(impulse_damp), spring->ra);
    if (b) nvBody_apply_impulse(b, impulse_damp, spring->rb);
}