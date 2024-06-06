/*

  This file is a part of the Nova Physics Engine
  project and distributed under the MIT license.

  Copyright © Kadir Aksoy
  https://github.com/kadir014/nova-physics

*/

#ifndef NOVAPHYSICS_HINGE_CONSTRAINT_H
#define NOVAPHYSICS_HINGE_CONSTRAINT_H

#include "novaphysics/internal.h"
#include "novaphysics/body.h"
#include "novaphysics/constraints/constraint.h"


/**
 * @file constraints/hinge_constraint.h
 * 
 * @brief Hinge constraint solver functions.
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
    nvVector2 anchor_a; /**< Joint anchor translated to body A. */
    nvVector2 anchor_b; /**< Joint anchor translated to body B. */
    nv_float upper_impulse; /**< Accumulated upper limit impulse. */
    nv_float lower_impulse; /**< Accumulated lower limit impulse. */
    nv_float reference_angle; /**< Reference angle for the constraint. */
    nv_float axial_mass; /**< Axial effective mass. */
    nvVector2 ra; /**< Anchor point on body A. */
    nvVector2 rb; /**< Anchor point on body B. */
    nvVector2 normal; /**< Normal of the distance constraint. */
    nv_float bias; /**< Distance constraint position correction bias. */
    nv_float mass; /**< Distance constraint effective mass. */
    nv_float jc; /**< Accumulated distance constraint impulse. */

    /*
        Public members (setters & getters)
    */
    nvVector2 anchor;
    nv_bool enable_limits;
    nv_float upper_limit;
    nv_float lower_limit;
    nv_float angle;
} nvHingeJoint;


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
} nvHingeConstraintInitializer;

static const nvHingeConstraintInitializer nvHingeConstraintInitializer_default = {
    NULL,
    NULL,
    {0.0, 0.0},
    false,
    NV_PI * 0.5,
    -NV_PI * 0.5
};


/**
 * @brief Create a new hinge joint constraint.
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
 * @brief Prepare for solving.
 * 
 * @param space Space
 * @param cons Constraint
 * @param inv_dt Inverse delta time
 */
void nvHingeConstraint_presolve(
    struct nvSpace *space,
    nvConstraint *cons,
    nv_float inv_dt
);

/**
 * @brief Solve hinge constraint.
 * 
 * @param cons Constraint
 * @param inv_dt Inverse delta time
 */
void nvHingeConstraint_solve(nvConstraint *cons, nv_float inv_dt);


#endif