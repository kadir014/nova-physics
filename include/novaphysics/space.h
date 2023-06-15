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
 * @brief Space struct and its methods.
 */


// Space callback type
typedef void ( *nv_Space_callback)(nv_HashMap *res_arr, void *user_data);


/**
 * @brief Space struct.
 */
struct nv_Space {
    nv_Array *bodies; /**< Array of bodies in the space. */
    nv_Array *attractors; /**< Array of attractive bodies in the space. */
    nv_Array *constraints; /**< Array of constraints in the space. */

    nv_HashMap *res; /**< Set of collision resolutions. */

    nv_Vector2 gravity; /**< Global and uniform gravity applied to all bodies in the space.
                             For gravitational attraction between body pairs, see attractive bodies. */
    
    bool sleeping; /**< Flag that specifies if space allows sleeping of bodies. */
    nv_float sleep_energy_threshold; /**< Threshold value which bodies sleep if they exceed it. */
    nv_float wake_energy_threshold; /**< Threshold value which bodies wake up if they exceed it. */
    int sleep_timer_threshold; /**< How long space should count to before sleeping bodies. */
    
    bool warmstarting; /**< Flag that specifies if solvers use warm starting with accumulated impulses. */
    nv_float baumgarte; /**< Baumgarte stabilization factor. */

    nv_BroadPhase broadphase_algorithm; /**< Broad-phase algorithm used to detect possible collisions. */
    nv_SHG *shg; /**< Spatial Hash Grid object
                     @warning Only accessible if the used broad-phase algorithm is SHG. */

    nv_CoefficientMix mix_restitution; /**< Method to mix restitution coefficients of collided bodies. */
    nv_CoefficientMix mix_friction; /**< Method to mix friction coefficients of collided bodies. */

    void *callback_user_data; /**< User data passed to collision callbacks. */
    nv_Space_callback before_collision; /**< Callback function called before solving collisions. */
    nv_Space_callback after_collision; /**< Callback function called after solving collisions. */
};

typedef struct nv_Space nv_Space;

/**
 * @brief Create new space instance.
 * 
 * @return nv_Space * 
 */
nv_Space *nv_Space_new();

/**
 * @brief Free space.
 * 
 * @param space Space to free
 */
void nv_Space_free(nv_Space *space);

/**
 * @brief Create & set a new SHG and release the old one.
 * 
 * @param space Space
 * @param bounds Boundaries of the new SHG
 * @param cell_width Cell width of the new SHG
 * @param cell_height Cell height of the new SHG
 */
void nv_Space_set_SHG(
    nv_Space *space,
    nv_AABB bounds,
    nv_float cell_width,
    nv_float cell_height
);

/**
 * @brief Clear and free everything in space.
 * 
 * @param space Space
 */
void nv_Space_clear(nv_Space *space);

/**
 * @brief Add body to space.
 * 
 * @param space Space
 * @param body Body to add
 */
void nv_Space_add(nv_Space *space, nv_Body *body);

/**
 * @brief Add constraint to space.
 * 
 * @param space Space
 * @param cons Constraint to add
 */
void nv_Space_add_constraint(nv_Space *space, nv_Constraint *cons);

/**
 * @brief Advance the simulation.
 * 
 * @param space Space instance
 * @param dt Time step size (delta time)
 * @param velocity_iters Velocity solving iteration count
 * @param position_iters Position solving iteration count
 * @param constraint_iters Constraint solving iteration count
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
 * @brief Enable sleeping.
 * 
 * @param space Space
 */
void nv_Space_enable_sleeping(nv_Space *space);

/**
 * @brief Disable sleeping.
 * 
 * @param space Space
 */
void nv_Space_disable_sleeping(nv_Space *space);


#endif