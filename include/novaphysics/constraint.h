/*

  This file is a part of the Nova Physics Engine
  project and distributed under the MIT license.

  Copyright © Kadir Aksoy
  https://github.com/kadir014/nova-physics

*/

#ifndef NOVAPHYSICS_CONSTRAINT_H
#define NOVAPHYSICS_CONSTRAINT_H

#include "novaphysics/internal.h"
#include "novaphysics/body.h"


/**
 * @file constraint.h
 * 
 * @brief Various constraints definitions.
 */


/**
 * @brief Constraint types
 */
typedef enum {
    nv_ConstraintType_SPRING,
    nv_ConstraintType_DISTANCEJOINT
} nv_ConstraintType;


/**
 * @brief Constraint base struct.
 */
typedef struct {
    nv_ConstraintType type; /**< Type of the constraint. */
    void *def; /**< Constraint definition data. (This needs to be casted) */
    nv_Body *a; /**< First body. */
    nv_Body *b; /**< Seconds body. */

    nv_Vector2 ra; /**< Anchor point on body A. */
    nv_Vector2 rb; /**< Anchor point on body B. */
    nv_Vector2 normal; /**< Normal of the constraint. */
    nv_float bias; /**< Constraint correction/stabilization bias. */
    nv_float mass; /**< Constraint effective mass. */
    nv_float jc; /**< Accumulated constraint impulse. */
} nv_Constraint;

/**
 * @brief Free constraint.
 * 
 * @param cons Constraint
 */
void nv_Constraint_free(void *cons);


// Different constraint allocator & initializer helpers

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


#endif