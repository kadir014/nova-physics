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
 * constraint.h
 * 
 * Constraints
 */


/**
 * @brief Constraint types
 */
typedef enum {
    nv_ConstraintType_SPRING,
    nv_ConstraintType_DISTANCEJOINT
} nv_ConstraintType;


/**
 * @brief Constraint base struct
 * 
 * @param type Type of the constraint
 * @param head Constraint head data (this needs to be casted)
 * @param a First body
 * @param b Second body
 * 
 * @param bias Stabilization bias
 * @param mass Constraint mass
 * @param jc Accumulated constraint impulse
 * 
 */
typedef struct {
    nv_ConstraintType type;
    void *head;
    nv_Body *a;
    nv_Body *b;

    nv_float bias;
    nv_float mass;
    nv_float jc;
} nv_Constraint;

/**
 * @brief Free constraint
 * 
 * @param cons Constraint
 */
void nv_Constraint_free(void *cons);


// Different constraint allocator & initializer helpers

/**
 * @brief Spring constraint head
 * 
 * @param length Length of the spring
 * @param stiffness Stiffnes (strength) of the spring
 * @param damping Damping (reducing overtime)
 * @param achor_a Local anchor point on body A
 * @param anchor_b Local anchor point on body B
 */
typedef struct {
    nv_float length;
    nv_float stiffness;
    nv_float damping;
    nv_Vector2 anchor_a;
    nv_Vector2 anchor_b;
} nv_Spring;

/**
 * @brief Create a new spring constraint
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
 * @brief Distance joint constraint head
 * 
 * @param length Length of the joint
 * @param anchor_a Local anchor point on body A
 * @param anchor_b Local anchor point on body B
 */
typedef struct {
    nv_float length;
    nv_Vector2 anchor_a;
    nv_Vector2 anchor_b;
} nv_DistanceJoint;

/**
 * @brief Create a new distance joint constraint
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