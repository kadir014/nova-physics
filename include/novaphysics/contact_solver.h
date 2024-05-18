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
#include "novaphysics/collision.h"
#include "novaphysics/resolution.h"
#include "novaphysics/constraint.h"


/**
 * @file contact_solver.h
 * 
 * @brief Contact solver functions.
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
 * @brief Prepare for solving contact constraints.
 * 
 * @param space Space
 * @param res Collision resolution
 * @param inv_dt Inverse delta time (1/Δt)
 */
void nv_presolve_contact(
    struct nvSpace *space,
    nvResolution *res,
    nv_float inv_dt
);

/**
 * @brief Apply accumulated impulses.
 * 
 * @param space Space
 * @param res Collision resolution
 */
void nv_warmstart(struct nvSpace *space, nvResolution *res);

/**
 * @brief Solve contact velocity constraints.
 * 
 * @param res Collision resolution
 */
void nv_solve_velocity(nvResolution *res);

/**
 * @brief Solve position error (pseudo-velocities / NGS).
 * 
 * @param res Collision resolution
 */
void nv_solve_position(nvResolution *res);


#endif