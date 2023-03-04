/*

  This file is a part of the Nova Physics Engine
  project and distributed under the MIT license.

  Copyright © Kadir Aksoy
  https://github.com/kadir014/nova-physics

*/

#ifndef NOVAPHYSICS_SPACE_H
#define NOVAPHYSICS_SPACE_H

#include "novaphysics/internal.h"
#include "novaphysics/array.h"
#include "novaphysics/body.h"
#include "novaphysics/broadphase.h"
#include "novaphysics/resolution.h"
#include "novaphysics/contact.h"
#include "novaphysics/constraint.h"
#include "novaphysics/solver.h"
#include "novaphysics/hashmap.h"
#include "novaphysics/shg.h"


/**
 * @file space.h
 * 
 * @details Space is the container of everything. Handles simulation.
 */


// Space callback type
typedef void ( *nv_Space_callback)(nv_HashMap *res_arr, void *user_data);


/**
 * Space struct
 * 
 * @param bodies Body array
 * @param attractors Attractor bodies array
 * @param constraints Constraint array
 * 
 * @param res Array of resolution objects
 * 
 * @param gravity Gravity vector
 * 
 * @param sleep_energy_threshold Energy threshold to sleep bodies
 * @param wake_energy_threshold Energy threshold to wake up bodies
 * @param sleeping Whether to allow sleeping or not
 * 
 * @param warmstarting Enable/disable warm starting using accumulated impulses
 * @param baumgarte Baumgarte stabilization constant
 * 
 * @param broadphase_algorithm Broad-phase algorithm used to detect collisions
 * 
 * @param mix_restitution Method to mix restitution of collided bodies
 * @param mix_friction Method to mix friction of collided bodies
 * 
 * @param callback_user_data User data passed to collision callbacks
 *                           Space doesn't free the user data
 * @param before_collision Callback function called before solving collision
 * @param after_collision Callback function called after solving collision
 */
struct _nv_Space{
    nv_Array *bodies;
    nv_Array *attractors;
    nv_Array *constraints;

    nv_HashMap *res;

    nv_Vector2 gravity;
    
    bool sleeping;
    nv_float sleep_energy_threshold;
    nv_float wake_energy_threshold;
    int sleep_timer_threshold;
    
    bool warmstarting;
    nv_float baumgarte;

    nv_BroadPhase broadphase_algorithm;
    nv_SHG *shg;

    nv_CoefficientMix mix_restitution;
    nv_CoefficientMix mix_friction;

    void *callback_user_data;
    nv_Space_callback before_collision;
    nv_Space_callback after_collision;
};

typedef struct _nv_Space nv_Space;

/**
 * @brief Create new space instance
 * 
 * @return nv_Space * 
 */
nv_Space *nv_Space_new();

/**
 * @brief Free space
 * 
 * @param space Space to free
 */
void nv_Space_free(nv_Space *space);

/**
 * @brief Clear and free everything in space
 * 
 * @param space Space
 */
void nv_Space_clear(nv_Space *space);

/**
 * @brief Add body to space
 * 
 * @param space Space
 * @param body Body to add
 */
void nv_Space_add(nv_Space *space, nv_Body *body);

/**
 * @brief Add constraint to space
 * 
 * @param space Space
 * @param cons Constraint to add
 */
void nv_Space_add_constraint(nv_Space *space, nv_Constraint *cons);

/**
 * @brief Advance the simulation
 * 
 * @param space Space instance
 * @param dt Time step length (delta time)
 * @param velocity_iters Velocity solving iteration amount
 * @param position_iters Position solving iteration amount
 * @param constraint_iters Constraint solving iteration amount
 * @param substeps Substep count
 */
void nv_Space_step(
    nv_Space *space,
    nv_float dt,
    int velocity_iters,
    int position_iters,
    int constraint_iters,
    int substeps
);

/**
 * @brief Enable sleeping
 * 
 * @param space Space
 */
void nv_Space_enable_sleeping(nv_Space *space);

/**
 * @brief Disable sleeping
 * 
 * @param space Space
 */
void nv_Space_disable_sleeping(nv_Space *space);


#endif