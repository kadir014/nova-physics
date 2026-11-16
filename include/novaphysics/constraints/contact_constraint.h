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
#include "novaphysics/space_settings.h"


/**
 * @file constraints/contact_constraint.h
 * 
 * @brief Contact constraint solver functions.
 */


/**
 * @brief Prepare for solving contact constraints.
 * 
 * @param pcp Contact pair
 * @param inv_dt Inverse delta time
 * @param settings Space settings
 */
void nv_contact_presolve(
    nvPersistentContactPair *pcp,
    nv_float inv_dt,
    nvSpaceSettings *settings
);

/**
 * @brief Apply accumulated impulses from last frame.
 * 
 * @param pcp Contact pair
 * @param warmstart Warmstarting
 */
void nv_contact_warmstart(nvPersistentContactPair *pcp, nv_bool warmstart);

/**
 * @brief Solve contact velocity constraints.
 * 
 * @param pcp Contact pair
 */
void nv_contact_solve_velocity(nvPersistentContactPair *pcp);


#endif