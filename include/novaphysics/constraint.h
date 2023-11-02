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
 * @brief Base constraint definition.
 */


/**
 * @brief Constraint types.
 */
typedef enum {
    nv_ConstraintType_SPRING, /**< Spring constraint type. See @ref nv_Spring_new */
    nv_ConstraintType_DISTANCEJOINT /**< Distance joint constraint type. See @ref nv_DistanceJoint_new */
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


#endif