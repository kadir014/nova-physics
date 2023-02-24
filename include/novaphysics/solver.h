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
 * Collision and constraint(not yet) resolver functions
 */


/**
 * @brief Prepare for resolving
 * 
 * @param res Collision resolution
 * @param inv_dt Inverse delta time (1/Δt)
 * @param baumgarte Position correction bias factor
 */
void nv_prestep_collision(
    nv_Resolution *res,
    double inv_dt,
    bool accumulate,
    double baumgarte
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
    double inv_dt,
    double baumgarte
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
void nv_prestep_spring(nv_Constraint *cons, double inv_dt, double baumgarte);

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
    double inv_dt,
    double baumgarte
);

/**
 * @brief Resolve distance constraint
 * 
 * @param cons Constraint
 */
void nv_resolve_distance_joint(nv_Constraint *cons);


#endif