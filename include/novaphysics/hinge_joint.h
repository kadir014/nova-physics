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
    nv_Vector2 anchor; /**< Anchor point in world space. */
    bool enable_limits; /**< Enable angular limits or not. */
    nv_float upper_limit; /**< Upper angle limit. */
    nv_float lower_limit; /**< Lower angle limit. */
    nv_float angle; /**< Angle of the constraint. */
    
    nv_Vector2 anchor_a; /**< Joint anchor translated to body A. */
    nv_Vector2 anchor_b; /**< Joint anchor translated to body B. */
    nv_float upper_impulse; /**< Accumulated upper limit impulse. */
    nv_float lower_impulse; /**< Accumulated lower limit impulse. */
    nv_float reference_angle; /**< Reference angle for the constrain. */
    nv_float axial_mass; /**< Axial effective mass. */
    nv_Vector2 ra; /**< Anchor point on body A. */
    nv_Vector2 rb; /**< Anchor point on body B. */
    nv_Vector2 normal; /**< Normal of the distance constraint. */
    nv_float bias; /**< Distance constraint position correction bias. */
    nv_float mass; /**< Distance constraint effective mass. */
    nv_float jc; /**< Accumulated distance constraint impulse. */
} nv_HingeJoint;

/**
 * @brief Create a new hinge joint constraint.
 * 
 * Leave one of the body parameters as :code:`NULL` to link the body to world.
 * Don't forget to change the anchor point to be in world space as well.
 * 
 * @param a First body
 * @param b Second body
 * @param anchor_a Anchor point in world space.
 * @return nv_Constraint * 
 */
nv_Constraint *nv_HingeJoint_new(
    nv_Body *a,
    nv_Body *b,
    nv_Vector2 anchor
);

/**
 * @brief Prepare for solving.
 * 
 * @param space Space
 * @param cons Constraint
 * @param inv_dt Inverse delta time (1/Δt)
 */
void nv_presolve_hinge_joint(
    struct nv_Space *space,
    nv_Constraint *cons,
    nv_float inv_dt
);

/**
 * @brief Solve hinge constraint.
 * 
 * @param cons Constraint
 * @param inv_dt Inverse delta time (1/Δt)
 */
void nv_solve_hinge_joint(nv_Constraint *cons, nv_float inv_dt);


#endif