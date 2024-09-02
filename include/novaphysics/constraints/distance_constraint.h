/*

  This file is a part of the Nova Physics Engine
  project and distributed under the MIT license.

  Copyright © Kadir Aksoy
  https://github.com/kadir014/nova-physics

*/

#ifndef NOVAPHYSICS_DISTANCE_CONSTRAINT_H
#define NOVAPHYSICS_DISTANCE_CONSTRAINT_H

#include "novaphysics/internal.h"
#include "novaphysics/constraints/constraint.h"


/**
 * @file constraints/distance_constraint.h
 * 
 * @brief Distance constraint solver.
 */


/**
 * @brief Distance constraint definition.
 * 
 * It constraints the distance of two points on the two bodies to be constant.
 * This acts like as if the two bodies are linked with a solid bar.
 * TODO: lower upper limit
 * TODO: explain spring
 */
typedef struct {
    /*
        Private members
    */
    nvVector2 xanchor_a; /**< Anchor A transformed with body's rotation. */
    nvVector2 xanchor_b; /**< Anchor B transformed with body's rotation. */
    nvVector2 normal; /**< Normal axis of the constraint. */
    nv_float bias; /**< Constraint position correction bias. */
    nv_float mass; /**< Constraint effective mass. */
    nv_float impulse; /**< Accumulated impulse. */
    nv_float max_impulse; /**< Max force * dt. */

    // Soft-constraint coefficients for the incremental lambda
    nv_float bias_rate;
    nv_float mass_coeff;
    nv_float impulse_coeff;

    /*
        Public members (setters & getters)
    */
    nv_float length;
    nvVector2 anchor_a;
    nvVector2 anchor_b;
    nv_float max_force;
    nv_bool spring;
    nv_float hertz;
    nv_float damping;
} nvDistanceConstraint;


/**
 * @brief Distance constraint initializer information.
 * 
 * This struct holds basic information for initializing and can be reused
 * for multiple constraints if the bodies are changed.
 */
typedef struct {
    nvRigidBody *a; /**< Body A. */
    nvRigidBody *b; /**< Body B. */
    nv_float length; /**< Length of the distance constraint. */
    nvVector2 anchor_a; /**< Local anchor point on body A. */
    nvVector2 anchor_b; /**< Local anchor point on body B. */
    nv_float max_force; /**< Maximum force allowed to solve the constraint. */
    nv_bool spring; /**< Is this distance constraint spring (soft-constraint)? */
    nv_float hertz; /**< Spring frequency. */
    nv_float damping; /**< Spring damping ratio. */
} nvDistanceConstraintInitializer;

static const nvDistanceConstraintInitializer nvDistanceConstraintInitializer_default = {
    NULL,
    NULL,
    1.0,
    {0.0, 0.0},
    {0.0, 0.0},
    NV_INF,
    false,
    3.0,
    0.3
};


/**
 * @brief Create a new distance constraint.
 * 
 * Leave one of the body parameters as `NULL` to link the body to world.
 * Don't forget to change the anchor point to be in world space as well.
 * 
 * Returns `NULL` on error. Use @ref nv_get_error to get more information.
 * 
 * @param init Initializer info
 * @return nvConstraint * 
 */
nvConstraint *nvDistanceConstraint_new(nvDistanceConstraintInitializer init);

/**
 * @brief Get body A of the constraint.
 * 
 * @param cons Constraint
 * @return nvRigidBody * 
 */
nvRigidBody *nvDistanceConstraint_get_body_a(const nvConstraint *cons);

/**
 * @brief Get body B of the constraint.
 * 
 * @param cons Constraint
 * @return nvRigidBody * 
 */
nvRigidBody *nvDistanceConstraint_get_body_b(const nvConstraint *cons);

/**
 * @brief Set length of the distance constraint.
 * 
 * @param cons Constraint
 * @param length Length
 */
void nvDistanceConstraint_set_length(nvConstraint *cons, nv_float length);

/**
 * @brief Get length of the distance constraint.
 * 
 * @param cons Constraint
 * @return nv_float
 */
nv_float nvDistanceConstraint_get_length(const nvConstraint *cons);

/**
 * @brief Set local anchor point on body A.
 * 
 * @param cons Constraint
 * @param anchor_a Anchor
 */
void nvDistanceConstraint_set_anchor_a(nvConstraint *cons, nvVector2 anchor_a);

/**
 * @brief Get local anchor point on body A.
 * 
 * @param cons Constraint
 * @return nvVector2
 */
nvVector2 nvDistanceConstraint_get_anchor_a(const nvConstraint *cons);

/**
 * @brief Set local anchor point on body B.
 * 
 * @param cons Constraint
 * @param anchor_b Anchor
 */
void nvDistanceConstraint_set_anchor_b(nvConstraint *cons, nvVector2 anchor_b);

/**
 * @brief Get local anchor point on body B.
 * 
 * @param cons Constraint
 * @return nvVector2
 */
nvVector2 nvDistanceConstraint_get_anchor_b(const nvConstraint *cons);

/**
 * @brief Set max force used to solve the constraint.
 * 
 * @param cons Constraint
 * @param max_force Max force
 */
void nvDistanceConstraint_set_max_force(nvConstraint *cons, nv_float max_force);

/**
 * @brief Get max force used to solve the constraint.
 * 
 * @param cons Constraint
 * @return nv_float
 */
nv_float nvDistanceConstraint_get_max_force(const nvConstraint *cons);

/**
 * @brief Enable/disable spring behavior.
 * 
 * @param cons Constraint
 * @param spring Bool
 */
void nvDistanceConstraint_set_spring(nvConstraint *cons, nv_bool spring);

/**
 * @brief Get whether spring behavior is enabled or not.
 * 
 * @param cons Constraint
 * @return nv_bool
 */
nv_bool nvDistanceConstraint_get_spring(const nvConstraint *cons);

/**
 * @brief Set spring frequency.
 * 
 * @param cons Constraint
 * @param hertz Frequency
 */
void nvDistanceConstraint_set_hertz(nvConstraint *cons, nv_float hertz);

/**
 * @brief Get spring frequency.
 * 
 * @param cons Constraint
 * @return nv_float
 */
nv_float nvDistanceConstraint_get_hertz(const nvConstraint *cons);

/**
 * @brief Set spring damping ratio.
 * 
 * @param cons Constraint
 * @param damping Damping ratio
 */
void nvDistanceConstraint_set_damping(nvConstraint *cons, nv_float damping);

/**
 * @brief Get spring damping ratio.
 * 
 * @param cons Constraint
 * @return nv_float
 */
nv_float nvDistanceConstraint_get_damping(const nvConstraint *cons);

/**
 * @brief Prepare for solving.
 * 
 * @param space Space
 * @param cons Constraint
 * @param dt Delta time
 * @param inv_dt Inverse delta time
 */
void nvDistanceConstraint_presolve(
    struct nvSpace *space,
    nvConstraint *cons,
    nv_float dt,
    nv_float inv_dt
);

/**
 * @brief Apply accumulated impulses from last frame.
 * 
 * @param space Space
 * @param cons Constraint
 */
void nvDistanceConstraint_warmstart(struct nvSpace *space, nvConstraint *cons);

/**
 * @brief Solve distance constraint.
 * 
 * @param cons Constraint
 */
void nvDistanceConstraint_solve(nvConstraint *cons);


#endif