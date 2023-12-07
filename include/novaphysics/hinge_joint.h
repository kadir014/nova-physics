/*

  This file is a part of the Nova Physics Engine
  project and distributed under the MIT license.

  Copyright © Kadir Aksoy
  https://github.com/kadir014/nova-physics

*/

#ifndef NOVAPHYSICS_HINGE_JOINT_CONSTRAINT_H
#define NOVAPHYSICS_HINGE_JOINT_CONSTRAINT_H

#include "novaphysics/internal.h"
#include "novaphysics/body.h"
#include "novaphysics/constraint.h"


/**
 * @file hinge_joint.h
 * 
 * @brief Hinge joint implementation.
 */


/**
 * @brief Hinge joint constraint definition.
 */
typedef struct {
    nvVector2 anchor; /**< Anchor point in world space. */
    bool enable_limits; /**< Enable angular limits or not. */
    nv_float upper_limit; /**< Upper angle limit. */
    nv_float lower_limit; /**< Lower angle limit. */
    nv_float angle; /**< Angle of the constraint. */
    
    nvVector2 anchor_a; /**< Joint anchor translated to body A. */
    nvVector2 anchor_b; /**< Joint anchor translated to body B. */
    nv_float upper_impulse; /**< Accumulated upper limit impulse. */
    nv_float lower_impulse; /**< Accumulated lower limit impulse. */
    nv_float reference_angle; /**< Reference angle for the constrain. */
    nv_float axial_mass; /**< Axial effective mass. */
    nvVector2 ra; /**< Anchor point on body A. */
    nvVector2 rb; /**< Anchor point on body B. */
    nvVector2 normal; /**< Normal of the distance constraint. */
    nv_float bias; /**< Distance constraint position correction bias. */
    nv_float mass; /**< Distance constraint effective mass. */
    nv_float jc; /**< Accumulated distance constraint impulse. */
} nvHingeJoint;

/**
 * @brief Create a new hinge joint constraint.
 * 
 * Leave one of the body parameters as :code:`NULL` to link the body to world.
 * Don't forget to change the anchor point to be in world space as well.
 * 
 * @param a First body
 * @param b Second body
 * @param anchor_a Anchor point in world space.
 * @return nvConstraint * 
 */
nvConstraint *nvHingeJoint_new(
    nvBody *a,
    nvBody *b,
    nvVector2 anchor
);

/**
 * @brief Prepare for solving.
 * 
 * @param space Space
 * @param cons Constraint
 * @param inv_dt Inverse delta time (1/Δt)
 */
void nv_presolve_hinge_joint(
    struct nvSpace *space,
    nvConstraint *cons,
    nv_float inv_dt
);

/**
 * @brief Solve hinge constraint.
 * 
 * @param cons Constraint
 * @param inv_dt Inverse delta time (1/Δt)
 */
void nv_solve_hinge_joint(nvConstraint *cons, nv_float inv_dt);


#endif