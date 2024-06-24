/*

  This file is a part of the Nova Physics Engine
  project and distributed under the MIT license.

  Copyright © Kadir Aksoy
  https://github.com/kadir014/nova-physics

*/

#ifndef NOVAPHYSICS_HINGE_CONSTRAINT_H
#define NOVAPHYSICS_HINGE_CONSTRAINT_H

#include "novaphysics/internal.h"
#include "novaphysics/constraints/constraint.h"


/**
 * @file constraints/hinge_constraint.h
 * 
 * @brief Hinge constraint solver.
 */


/**
 * @brief Hinge constraint definition.
 * 
 * A hinge, also known as revolute or point, constraint allows the
 * bodies to rotate around a common axis. Ensuring the relative orientation of
 * the bodies remains fixed.
 */
typedef struct {
    /*
        Private members
    */
    nvVector2 anchor_a; /**< Anchor local to body A. */
    nvVector2 anchor_b; /**< Anchor local to body B. */
    nvVector2 xanchor_a; /**< Anchor A transformed with body's rotation. */
    nvVector2 xanchor_b; /**< Anchor B transformed with body's rotation. */
    nv_float upper_impulse; /**< Accumulated upper limit impulse. */
    nv_float lower_impulse; /**< Accumulated lower limit impulse. */
    nv_float upper_bias; /**< Upper angle limit constraint correction bias. */
    nv_float lower_bias;/**< Lower angle limit constraint correction bias. */
    nv_float reference_angle; /**< Reference angle for the constraint. */
    nv_float axial_mass; /**< Axial effective mass. */
    nvVector2 normal; /**< Normal axis of the constraint. */
    nv_float bias; /**< Point constraint position correction bias. */
    nv_float mass; /**< Point constraint effective mass. */
    nv_float impulse; /**< Accumulated point constraint impulse. */
    nv_float max_impulse; /**< Max force * dt. */

    /*
        Public members (setters & getters)
    */
    nvVector2 anchor;
    nv_bool enable_limits;
    nv_float upper_limit;
    nv_float lower_limit;
    nv_float angle;
    nv_float max_force;
} nvHingeConstraint;


/**
 * @brief Hinge constraint initializer information.
 * 
 * This struct holds basic information for initializing and can be reused
 * for multiple constraints if the bodies are changed.
 */
typedef struct {
    nvRigidBody *a; /**< Body A. */
    nvRigidBody *b; /**< Body B. */
    nvVector2 anchor; /**< Anchor point in world space. */
    nv_bool enable_limits; /**< Whether to enable angular limits or not. */
    nv_float upper_limit; /**< Upper angle limit. */
    nv_float lower_limit; /**< Lower angle limit. */
    nv_float max_force; /**< Maximum force allowed to solve the constraint. */
} nvHingeConstraintInitializer;

static const nvHingeConstraintInitializer nvHingeConstraintInitializer_default = {
    NULL,
    NULL,
    {0.0, 0.0},
    false,
    NV_PI * 0.5,
    -NV_PI * 0.5,
    NV_INF
};


/**
 * @brief Create a new hinge constraint.
 * 
 * Leave one of the body parameters as `NULL` to link the body to world.
 * Don't forget to change the anchor point to be in world space as well.
 * 
 * Returns `NULL` on error. Use @ref nv_get_error to get more information.
 * 
 * @param init Initializer info
 * @return nvConstraint * 
 */
nvConstraint *nvHingeConstraint_new(nvHingeConstraintInitializer init);

/**
 * @brief Get body A of the constraint.
 * 
 * @param cons Constraint
 * @return nvRigidBody * 
 */
nvRigidBody *nvHingeConstraint_get_body_a(const nvConstraint *cons);

/**
 * @brief Get body B of the constraint.
 * 
 * @param cons Constraint
 * @return nvRigidBody * 
 */
nvRigidBody *nvHingeConstraint_get_body_b(const nvConstraint *cons);

/**
 * @brief Set anchor of the hinge constraint in world space.
 * 
 * @param cons Constraint
 * @param anchor Anchor point in world space
 */
void nvHingeConstraint_set_anchor(nvConstraint *cons, nvVector2 anchor);

/**
 * @brief Get anchor of the hinge constraint in world space.
 * 
 * @param cons Constraint
 * @return nvVector2
 */
nvVector2 nvHingeConstraint_get_anchor(const nvConstraint *cons);

/**
 * @brief Enable/disable angular limits.
 * 
 * @param cons Constraint
 * @param limits Bool
 */
void nvHingeConstraint_set_limits(nvConstraint *cons, nv_bool limits);

/**
 * @brief Get whether angular limits is enabled or not.
 * 
 * @param cons Constraint
 * @return nv_bool 
 */
nv_bool nvHingeConstraint_get_limits(const nvConstraint *cons);

/**
 * @brief Set upper angular limit.
 * 
 * @param cons Constraint
 * @param upper_limit Upper limit
 */
void nvHingeConstraint_set_upper_limit(nvConstraint *cons, nv_float upper_limit);

/**
 * @brief Get upper angular limit.
 * 
 * @param cons Constraint
 * @return nv_float 
 */
nv_float nvHingeConstraint_get_upper_limit(const nvConstraint *cons);

/**
 * @brief Set lower angular limit.
 * 
 * @param cons Constraint
 * @param lower_limit Lower limit
 */
void nvHingeConstraint_set_lower_limit(nvConstraint *cons, nv_float lower_limit);

/**
 * @brief Get lower angular limit.
 * 
 * @param cons Constraint
 * @return nv_float 
 */
nv_float nvHingeConstraint_get_lower_limit(const nvConstraint *cons);

/**
 * @brief Set max force used to solve the constraint.
 * 
 * @param cons Constraint
 * @param max_force Max force
 */
void nvHingeConstraint_set_max_force(nvConstraint *cons, nv_float max_force);

/**
 * @brief Get max force used to solve the constraint.
 * 
 * @param cons Constraint
 * @return nv_float 
 */
nv_float nvHingeConstraint_get_max_force(const nvConstraint *cons);


/**
 * @brief Prepare for solving.
 * 
 * @param space Space
 * @param cons Constraint
 * @param inv_dt Inverse delta time
 */
void nvHingeConstraint_presolve(
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
void nvHingeConstraint_warmstart(struct nvSpace *space, nvConstraint *cons);

/**
 * @brief Solve hinge constraint.
 * 
 * @param cons Constraint
 * @param inv_dt Inverse delta time
 */
void nvHingeConstraint_solve(nvConstraint *cons, nv_float inv_dt);


#endif