/*

  This file is a part of the Nova Physics Engine
  project and distributed under the MIT license.

  Copyright © Kadir Aksoy
  https://github.com/kadir014/nova-physics

*/

#ifndef NOVAPHYSICS_DISTANCE_JOINT_CONSTRAINT_H
#define NOVAPHYSICS_DISTANCE_JOINT_CONSTRAINT_H

#include "novaphysics/internal.h"
#include "novaphysics/body.h"
#include "novaphysics/constraint.h"


/**
 * @file distance_joint.h
 * 
 * @brief Distance joint implementation.
 */


/**
 * @brief Distance joint constraint definition.
 * 
 * @param length Length of the joint
 * @param anchor_a Local anchor point on body A
 * @param anchor_b Local anchor point on body B
 */
typedef struct {
    nv_float length; /**< Length of the distance joint. */
    nv_Vector2 anchor_a; /**< Local anchor point on body A. */
    nv_Vector2 anchor_b; /**< Local anchor point on body B. */
} nv_DistanceJoint;

/**
 * @brief Create a new distance joint constraint.
 * 
 * @param a First body
 * @param b Second body
 * @param anchor_a Local anchor point on body A
 * @param anchor_b Local anchor point on body B
 * @param length Length of the joint
 * @return nv_Constraint * 
 */
nv_Constraint *nv_DistanceJoint_new(
    nv_Body *a,
    nv_Body *b,
    nv_Vector2 anchor_a,
    nv_Vector2 anchor_b,
    nv_float length
);

/**
 * @brief Prepare for solving.
 * 
 * @param space Space
 * @param cons Constraint
 * @param inv_dt Inverse delta time (1/Δt)
 */
void nv_presolve_distance_joint(
    struct nv_Space *space,
    nv_Constraint *cons,
    nv_float inv_dt
);

/**
 * @brief Solve distance constraint.
 * 
 * @param cons Constraint
 */
void nv_solve_distance_joint(nv_Constraint *cons);


#endif