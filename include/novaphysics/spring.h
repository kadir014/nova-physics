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
 * 
 * A spring constraint is a damped spring with rest length, stiffness and damping value.
 */
typedef struct {
    nv_float length; /**< Resting length of the spring. */
    nv_float stiffness; /**< Stiffness (strength) of the spring. */
    nv_float damping; /**< Damping of the spring. */
    
    nvVector2 anchor_a; /**< Local anchor point on body A. */
    nvVector2 anchor_b; /**< Local anchor point on body B. */
    nv_float target_vel; /**< Target relative velocity. */
    nv_float damping_bias; /**< Damping bias. */
    nvVector2 ra; /**< Anchor point on body A. */
    nvVector2 rb; /**< Anchor point on body B. */
    nvVector2 normal; /**< Normal of the constraint. */
    nv_float mass; /**< Constraint effective mass. */
    nv_float jc; /**< Accumulated constraint impulse. */
} nvSpring;

/**
 * @brief Create a new spring constraint.
 * 
 * Leave one of the body parameters as :code:`NULL` to link the body to world.
 * Don't forget to change the anchor point to be in world space as well.
 * 
 * @param a First body
 * @param b Second body
 * @param anchor_a Local anchor point on body A
 * @param anchor_b Local anchor point on body B
 * @param length Length of the spring
 * @param stiffness Stiffness (strength) of the spring
 * @param damping Damping of the spring
 * 
 * @return nvConstraint * 
 */
nvConstraint *nvSpring_new(
    nvBody *a,
    nvBody *b,
    nvVector2 anchor_a,
    nvVector2 anchor_b,
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
void nvSpring_presolve(
    struct nvSpace *space,
    nvConstraint *cons,
    nv_float inv_dt
);

/**
 * @brief Solve spring constraint.
 * 
 * @param cons Constraint
 */
void nvSpring_solve(nvConstraint *cons);


#endif