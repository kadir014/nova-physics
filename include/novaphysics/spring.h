/*

  This file is a part of the Nova Physics Engine
  project and distributed under the MIT license.

  Copyright © Kadir Aksoy
  https://github.com/kadir014/nova-physics

*/

#ifndef NOVAPHYSICS_SPRING_CONSTRAINT_H
#define NOVAPHYSICS_SPRING_CONSTRAINT_H

#include "novaphysics/internal.h"
#include "novaphysics/body.h"
#include "novaphysics/constraint.h"


/**
 * @file spring.h
 * 
 * @brief Damped spring implementation.
 */


/**
 * @brief Spring constraint definition.
 */
typedef struct {
    nv_float length; /**< Resting length of the spring. */
    nv_float stiffness; /**< Stiffness (strength) of the spring. */
    nv_float damping; /**< Damping of the spring. */
    nv_Vector2 anchor_a; /**< Local anchor point on body A. */
    nv_Vector2 anchor_b; /**< Local anchor point on body B. */
    nv_float target_rn;
    nv_float v_coef;
} nv_Spring;

/**
 * @brief Create a new spring constraint.
 * 
 * @param a First body
 * @param b Second body
 * @param anchor_a Local anchor point on body A
 * @param anchor_b Local anchor point on body B
 * @param length Length of the spring
 * @param stiffness Stiffness (strength) of the spring
 * @param damping Damping of the spring
 * 
 * @return nv_Constraint * 
 */
nv_Constraint *nv_Spring_new(
    nv_Body *a,
    nv_Body *b,
    nv_Vector2 anchor_a,
    nv_Vector2 anchor_b,
    nv_float length,
    nv_float stiffness,
    nv_float damping
);

/**
 * @brief Prepare for solving.
 * 
 * @param space Space
 * @param cons Constraint
 * @param inv_dt Inverse delta time (1/Δt)
 */
void nv_presolve_spring(
    struct nv_Space *space,
    nv_Constraint *cons,
    nv_float inv_dt
);

/**
 * @brief Solve spring constraint.
 * 
 * @param cons Constraint
 */
void nv_solve_spring(nv_Constraint *cons);


#endif