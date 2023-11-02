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


nv_Constraint *nv_Spring_new(
    nv_Body *a,
    nv_Body *b,
    nv_Vector2 anchor_a,
    nv_Vector2 anchor_b,
    nv_float length,
    nv_float stiffness,
    nv_float damping
) {
    nv_Constraint *cons = NV_NEW(nv_Constraint);
    if (!cons) return NULL;

    cons->a = a;
    cons->b = b;
    cons->type = nv_ConstraintType_SPRING;
    cons->jc = 0.0;

    cons->def = (void *)NV_NEW(nv_Spring);
    if (!cons->def) return NULL;
    nv_Spring *spring = (nv_Spring *)cons->def;

    spring->length = length;
    spring->stiffness = stiffness;
    spring->damping = damping;
    spring->anchor_a = anchor_a;
    spring->anchor_b = anchor_b;

    return cons;
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