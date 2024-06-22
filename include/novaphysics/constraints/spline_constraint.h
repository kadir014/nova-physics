/*

  This file is a part of the Nova Physics Engine
  project and distributed under the MIT license.

  Copyright © Kadir Aksoy
  https://github.com/kadir014/nova-physics

*/

#ifndef NOVAPHYSICS_SPLINE_CONSTRAINT_H
#define NOVAPHYSICS_SPLINE_CONSTRAINT_H

#include "novaphysics/internal.h"
#include "novaphysics/constraints/constraint.h"


/**
 * @file constraints/spline_constraint.h
 * 
 * @brief Spline constraint solver.
 */


/**
 * @brief Spline constraint definition.
 * 
 * This constrains the body to a catmull-rom spline path.
 * https://en.wikipedia.org/wiki/Centripetal_Catmull%E2%80%93Rom_spline
 */
typedef struct {
    /*
        Private members
    */
    nvVector2 anchor_a; /**< Anchor local to body A. */
    nvVector2 anchor_b; /**< Anchor local to body B. */
    nvVector2 xanchor_a; /**< Anchor A transformed with body's rotation. */
    nvVector2 xanchor_b; /**< Anchor B transformed with body's rotation. */
    nvVector2 normal; /**< Normal axis of the constraint. */
    nv_float bias; /**< Constraint position correction bias. */
    nv_float mass; /**< Point constraint effective mass. */
    nv_float impulse; /**< Accumulated point constraint impulse. */
    nv_float max_impulse; /**< Max force * dt. */

    /*
        Public members (setters & getters)
    */
    nvVector2 anchor;
    nv_float max_force;
    nvVector2 controls[NV_SPLINE_CONSTRAINT_MAX_CONTROL_POINTS];
    size_t num_controls;
} nvSplineConstraint;


/**
 * @brief Spline constraint initializer information.
 * 
 * This struct holds basic information for initializing and can be reused
 * for multiple constraints if the bodies are changed.
 */
typedef struct {
    nvRigidBody *body; /**< Body. */
    nvVector2 anchor; /**< Anchor point in world space. */
    nv_float max_force; /**< Maximum force allowed to solve the constraint. */
} nvSplineConstraintInitializer;

static const nvSplineConstraintInitializer nvSplineConstraintInitializer_default = {
    NULL,
    {0.0, 0.0},
    NV_INF
};


/**
 * @brief Create a new spline constraint.
 * 
 * Leave one of the body parameters as `NULL` to link the body to world.
 * Don't forget to change the anchor point to be in world space as well.
 * 
 * Returns `NULL` on error. Use @ref nv_get_error to get more information.
 * 
 * @param init Initializer info
 * @return nvConstraint * 
 */
nvConstraint *nvSplineConstraint_new(nvSplineConstraintInitializer init);

/**
 * @brief Get body of the constraint.
 * 
 * @param cons Constraint
 * @return nvRigidBody * 
 */
nvRigidBody *nvSplineConstraint_get_body(const nvConstraint *cons);

/**
 * @brief Set anchor point in world space.
 * 
 * @param cons Constraint
 * @param anchor Anchor
 */
void nvSplineConstraint_set_anchor(nvConstraint *cons, nvVector2 anchor);

/**
 * @brief Get anchor point in world space.
 * 
 * @param cons Constraint
 * @return nvVector2
 */
nvVector2 nvSplineConstraint_get_anchor(const nvConstraint *cons);

/**
 * @brief Set max force used to solve the constraint.
 * 
 * @param cons Constraint
 * @param max_force Max force
 */
void nvSplineConstraint_set_max_force(nvConstraint *cons, nv_float max_force);

/**
 * @brief Get max force used to solve the constraint.
 * 
 * @param cons Constraint
 * @return nv_float
 */
nv_float nvSplineConstraint_get_max_force(const nvConstraint *cons);

/**
 * @brief Set control points of the spline.
 * 
 * Returns non-zero on error. Use @ref nv_get_error to get more information.
 * 
 * @param cons Constraint
 * @param points Array of nvVector2
 * @param num_points Number of points
 */
int nvSplineConstraint_set_control_points(
    nvConstraint *cons,
    nvVector2 *points,
    size_t num_points
);

/**
 * @brief Get control points of the spline.
 * 
 * @param cons Constraint
 * @return nvVector2 * 
 */
nvVector2 *nvSplineConstraint_get_control_points(const nvConstraint *cons);

/**
 * @brief Get the number of control points of the spline.
 * 
 * @param cons Constraints
 * @return size_t 
 */
size_t nvSplineConstraint_get_number_of_control_points(const nvConstraint *cons);

/**
 * @brief Prepare for solving.
 * 
 * @param space Space
 * @param cons Constraint
 * @param inv_dt Inverse delta time
 */
void nvSplineConstraint_presolve(
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
void nvSplineConstraint_warmstart(struct nvSpace *space, nvConstraint *cons);

/**
 * @brief Solve spline constraint.
 * 
 * @param cons Constraint
 */
void nvSplineConstraint_solve(nvConstraint *cons);


#endif