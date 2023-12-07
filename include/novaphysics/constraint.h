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
    nvConstraintType_SPRING, /**< Spring constraint type. See @ref nvSpring. */
    nvConstraintType_DISTANCEJOINT, /**< Distance joint constraint type. See @ref nvDistanceJoint. */
    nvConstraintType_HINGEJOINT /**< Hinge joint constraint type. See @ref nvHingeJoint. */
} nvConstraintType;


/**
 * @brief Constraint base struct.
 */
typedef struct {
    nvConstraintType type; /**< Type of the constraint. */
    void *def; /**< Constraint definition class. (This needs to be casted) */
    nvBody *a; /**< First body. */
    nvBody *b; /**< Second body. */
} nvConstraint;

/**
 * @brief Free constraint.
 * 
 * @param cons Constraint
 */
void nvConstraint_free(void *cons);


#endif