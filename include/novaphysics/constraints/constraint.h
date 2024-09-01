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
 * @file constraints/constraint.h
 * 
 * @brief Base constraint definition.
 */


/**
 * @brief Type of algorithm used to solve position error in collisions.
 * 
 * @note Changing this setting should be usually avoided unless you have
 * a specific need or familiar with the behavior.
 * 
 * In baumgarte stabilization, the position error is fed back into the velocity
 * constraint, this is an efficient solution however it adds energy to the system.
 * 
 * NGS (Non-Linear Gauss-Seidel) uses pseudo-velocities to resolve the drift.
 * It is computationally bit more expensive but more stable.
 * It is what version 2 of Box2D uses.
 * 
 * @warning Nova, as of 1.0.0, doesn't have NGS solver yet.
 */
typedef enum {
    nvContactPositionCorrection_BAUMGARTE, /**< Baumgarte stabilization. */
    nvContactPositionCorrection_NGS /**< Non-Linear Gauss-Seidel. */
} nvContactPositionCorrection;


/**
 * @brief Coefficient mixing type is the method to mix various coefficients
 *        values like restitution and friction.
 */
typedef enum {
    nvCoefficientMix_AVG, /**< (a + b) * 0.5 */
    nvCoefficientMix_MUL, /**< a * b */
    nvCoefficientMix_SQRT, /**< sqrt(a * b) */
    nvCoefficientMix_MIN, /**< min(a, b) */
    nvCoefficientMix_MAX /**< max(a, b) */
} nvCoefficientMix;

/**
 * @brief Mix two coefficient values.
 * 
 * @param a First value
 * @param b Second value
 * @param mix Mixing type
 * @return nv_float 
 */
static inline nv_float nv_mix_coefficients(
    nv_float a,
    nv_float b,
    nvCoefficientMix mix
) {
    switch (mix) {
        case nvCoefficientMix_AVG:
            return (a + b) * (nv_float)0.5;

        case nvCoefficientMix_MUL:
            return a * b;

        case nvCoefficientMix_SQRT:
            return nv_sqrt(a * b);

        case nvCoefficientMix_MIN:
            return nv_fmin(a, b);

        case nvCoefficientMix_MAX:
            return nv_fmax(a, b);

        default:
            // worth setting error?
            return 0.0;
    }
}


/**
 * @brief Constraint types.
 * 
 * Contact constraint is not included because it's handled internally by the engine.
 */
typedef enum {
    nvConstraintType_DISTANCE, /**< Distance constraint type. See @ref nvDistanceConstraint. */
    nvConstraintType_HINGE, /**< Hinge constraint type. See @ref nvHingeConstraint. */
    nvConstraintType_SPLINE /**< Spline constraint type. See @ref nvSplineConstraint. */
} nvConstraintType;


/**
 * @brief Base two-body constraint.
 */
typedef struct {
    nvConstraintType type; /**< Type of the constraint. */
    void *def; /**< Constraint definition class. (This needs to be casted) */
    nvRigidBody *a; /**< First body. */
    nvRigidBody *b; /**< Second body. */
    nv_bool ignore_collision; /**< Ignore collision of bodies connected with this constraint. */
} nvConstraint;

/**
 * @brief Free constraint.
 * 
 * It's safe to pass `NULL` to this function.
 * 
 * @param cons Constraint
 */
void nvConstraint_free(nvConstraint *cons);

/**
 * @brief Prepare for solving.
 * 
 * @param space Space
 * @param cons Constraint
 * @param dt Delta time
 * @param inv_dt Inverse delta time
 */
void nvConstraint_presolve(
    struct nvSpace *space,
    nvConstraint *cons,
    nv_float dt,
    nv_float inv_dt
);

/**
 * @brief Warmstart / Accumulate impulses from last frame.
 * 
 * @param space Space
 * @param cons Constraint
 */
void nvConstraint_warmstart(struct nvSpace *space, nvConstraint *cons);

/**
 * @brief Solve constraint.
 * 
 * @param cons Constraint
 * @param inv_dt Inverse delta time
 */
void nvConstraint_solve(nvConstraint *cons, nv_float inv_dt);


#endif