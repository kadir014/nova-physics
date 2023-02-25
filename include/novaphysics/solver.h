/*

  This file is a part of the Nova Physics Engine
  project and distributed under the MIT license.

  Copyright © Kadir Aksoy
  https://github.com/kadir014/nova-physics

*/

#ifndef NOVAPHYSICS_SOLVER_H
#define NOVAPHYSICS_SOLVER_H

#include "novaphysics/body.h"
#include "novaphysics/collision.h"
#include "novaphysics/resolution.h"
#include "novaphysics/constraint.h"


/**
 * solver.h
 * 
 * Collision and constraint solver functions
 */


/**
 * @brief nv_CoefficientMix enum is for specifying the method
 *        to mix various coefficients (restitution, friction)
 * 
 * @param AVG (a + b) / 2
 * @param SQRT sqrt(a * b)
 * @param MIN min(a, b)
 * @param MAX max(a, b)
 */
typedef enum {
    nv_CoefficientMix_AVG,
    nv_CoefficientMix_SQRT,
    nv_CoefficientMix_MIN,
    nv_CoefficientMix_MAX
} nv_CoefficientMix;

/**
 * @brief Mix two coefficients
 * 
 * @param a First value
 * @param b Second value
 * @param mix Mixing type
 * @return nv_float 
 */
static inline nv_float nv_mix_coefficients(nv_float a, nv_float b, nv_CoefficientMix mix) {
    switch (mix) {
        case nv_CoefficientMix_AVG:
            return (a + b) / 2.0;

        case nv_CoefficientMix_SQRT:
            return nv_sqrt(a * b);

        case nv_CoefficientMix_MIN:
            return nv_fmin(a, b);

        case nv_CoefficientMix_MAX:
            return nv_fmax(a, b);
    }
}


/**
 * @brief Prepare for resolving
 * 
 * @param res Collision resolution
 * @param inv_dt Inverse delta time (1/Δt)
 * @param baumgarte Position correction bias factor
 */
void nv_prestep_collision(
    struct _nv_Space *space,
    nv_Resolution *res,
    nv_float inv_dt
);

/**
 * @brief Resolve collision between two bodies
 * 
 * @param res Collision resolution
 * @param accumulate Enable/disable accumulating impulses
 */
void nv_resolve_collision(nv_Resolution *res, bool accumulate);


/**
 * @brief Prepare for resolving
 * 
 * @param cons Constraintt
 * @param inv_dt Inverse delta time (1/Δt)
 * @param baumgarte Baumgarte stabilization bias factor
 */
void nv_prestep_constraint(
    nv_Constraint *cons,
    nv_float inv_dt,
    nv_float baumgarte
);

/**
 * @brief Resolve constraint
 * 
 * @param cons Constraint
 */
void nv_resolve_constraint(nv_Constraint *cons);


/**
 * @brief Prepare for resolving
 * 
 * @param cons Constraint
 * @param inv_dt Inverse delta time (1/Δt)
 * @param baumgarte Baumgarte stabilization bias factor
 */
void nv_prestep_spring(
    nv_Constraint *cons,
    nv_float inv_dt,
    nv_float baumgarte
);

/**
 * @brief Resolve spring constraint
 * 
 * @param cons Constraint
 */
void nv_resolve_spring(nv_Constraint *cons);


/**
 * @brief Prepare for resolving
 * 
 * @param cons Constraint
 * @param inv_dt Inverse delta time (1/Δt)
 * @param baumgarte Baumgarte stabilization bias factor
 */
void nv_prestep_distance_joint(
    nv_Constraint *cons,
    nv_float inv_dt,
    nv_float baumgarte
);

/**
 * @brief Resolve distance constraint
 * 
 * @param cons Constraint
 */
void nv_resolve_distance_joint(nv_Constraint *cons);


#endif