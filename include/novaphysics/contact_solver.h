/*

  This file is a part of the Nova Physics Engine
  project and distributed under the MIT license.

  Copyright © Kadir Aksoy
  https://github.com/kadir014/nova-physics

*/

#ifndef NOVAPHYSICS_SOLVER_H
#define NOVAPHYSICS_SOLVER_H

#include "novaphysics/internal.h"
#include "novaphysics/body.h"
#include "novaphysics/contact.h"
#include "novaphysics/collision.h"
#include "novaphysics/constraint.h"


/**
 * @file contact_solver.h
 * 
 * @brief Contact constraint solver functions.
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
 */
typedef enum {
    nvPositionCorrection_BAUMGARTE, /**< Baumgarte stabilization. */
    nvPositionCorrection_NGS /**< Non-Linear Gauss-Seidel. */
} nvPositionCorrection;


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
            return (a + b) * 0.5;

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
 * @brief Prepare for solving contact constraints.
 * 
 * @param space Space
 * @param pcp Contact pair
 * @param inv_dt Inverse delta time
 */
void nv_presolve_contact(
    struct nvSpace *space,
    nvPersistentContactPair *pcp,
    nv_float inv_dt
);

/**
 * @brief Apply accumulated impulses from last frame.
 * 
 * @param space Space
 * @param pcp Contact pair
 */
void nv_warmstart(struct nvSpace *space, nvPersistentContactPair *pcp);

/**
 * @brief Solve contact velocity constraints (PGS [+ Baumgarte]).
 * 
 * @param pcp Contact pair
 */
void nv_solve_velocity(nvPersistentContactPair *pcp);

/**
 * @brief Solve position error (NGS).
 * 
 * @param pcp Contact pair
 */
void nv_solve_position(nvPersistentContactPair *pcp);


#endif