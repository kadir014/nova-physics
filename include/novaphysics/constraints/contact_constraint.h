/*

  This file is a part of the Nova Physics Engine
  project and distributed under the MIT license.

  Copyright © Kadir Aksoy
  https://github.com/kadir014/nova-physics

*/

#ifndef NOVAPHYSICS_CONTACT_CONSTRAINT_H
#define NOVAPHYSICS_CONTACT_CONSTRAINT_H

#include "novaphysics/internal.h"
#include "novaphysics/body.h"
#include "novaphysics/contact.h"
#include "novaphysics/collision.h"
#include "novaphysics/constraints/constraint.h"


/**
 * @file constraints/contact_constraint.h
 * 
 * @brief Contact constraint solver functions.
 */


/**
 * @brief Prepare for solving contact constraints.
 * 
 * @param space Space
 * @param pcp Contact pair
 * @param inv_dt Inverse delta time
 */
void nv_contact_presolve(
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
void nv_contact_warmstart(struct nvSpace *space, nvPersistentContactPair *pcp);

/**
 * @brief Solve contact velocity constraints (PGS [+ Baumgarte]).
 * 
 * @param pcp Contact pair
 */
void nv_contact_solve_velocity(nvPersistentContactPair *pcp);

/**
 * @brief Solve position error (NGS).
 * 
 * @param pcp Contact pair
 */
void nv_contact_solve_position(nvPersistentContactPair *pcp);


#endif