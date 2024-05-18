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
 * A distance joint constraints the distance of two points on the two bodies to be constant.
 * This acts like as if the two bodies are linked with a solid bar.
 * TODO: lower upper limit
 */
typedef struct {
    nv_float length; /**< Length of the distance joint. */
    nvVector2 anchor_a; /**< Local anchor point on body A. */
    nvVector2 anchor_b; /**< Local anchor point on body B. */

    nvVector2 ra; /**< Anchor point on body A. */
    nvVector2 rb; /**< Anchor point on body B. */
    nvVector2 normal; /**< Normal of the constraint. */
    nv_float bias; /**< Constraint position correction bias. */
    nv_float mass; /**< Constraint effective mass. */
    nv_float jc; /**< Accumulated constraint impulse. */

    nv_float biasRate;
    nv_float massCoeff;
    nv_float impulseCoeff;
} nvDistanceJoint;

/**
 * @brief Create a new distance joint constraint.
 * 
 * Leave one of the body parameters as :code:`NULL` to link the body to world.
 * Don't forget to change the anchor point to be in world space as well.
 * 
 * @param a First body
 * @param b Second body
 * @param anchor_a Local anchor point on body A
 * @param anchor_b Local anchor point on body B
 * @param length Length of the joint
 * @return nvConstraint * 
 */
nvConstraint *nvDistanceJoint_new(
    nvRigidBody *a,
    nvRigidBody *b,
    nvVector2 anchor_a,
    nvVector2 anchor_b,
    nv_float length
);

/**
 * @brief Prepare for solving.
 * 
 * @param space Space
 * @param cons Constraint
 * @param inv_dt Inverse delta time (1/Δt)
 */
void nvDistanceJoint_presolve(
    struct nvSpace *space,
    nvConstraint *cons,
    nv_float inv_dt
);

/**
 * @brief Solve distance constraint.
 * 
 * @param cons Constraint
 */
void nvDistanceJoint_solve(nvConstraint *cons);


#endif