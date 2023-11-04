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
#include "novaphysics/profiler.h"


/**
 * @file space.h
 * 
 * @brief Space struct and its methods.
 */


// Space callback type
typedef void ( *nv_Space_callback)(nv_HashMap *res_arr, void *user_data);


/**
 * @brief Space struct.
 * 
 * Space is the core of the simulation. It manages all bodies, constraints, collisions and handles them.
 */
struct nv_Space {
    nv_Array *bodies; /**< Array of bodies in the space. */
    nv_Array *attractors; /**< Array of attractive bodies in the space. */
    nv_Array *constraints; /**< Array of constraints in the space. */

    nv_Array *_removed_bodies; /**< Bodies that are waiting to be removed.
                                    You shouldn't access this directly, instead use @ref nv_Space_remove method. */
    nv_Array *_killed_bodies; /**< Bodies that are waiting to be removed and freed.
                                   You shouldn't access this directly, instead use @ref nv_Space_kill method.*/

    nv_HashMap *res; /**< Set of collision resolutions. */

    nv_Vector2 gravity; /**< Global and uniform gravity applied to all bodies in the space.
                             For gravitational attraction between body pairs, see attractive bodies. */
    
    bool sleeping; /**< Flag that specifies if space allows sleeping of bodies. */
    nv_float sleep_energy_threshold; /**< Threshold value which bodies sleep if they exceed it. */
    nv_float wake_energy_threshold; /**< Threshold value which bodies wake up if they exceed it. */
    int sleep_timer_threshold; /**< How long space should count to before sleeping bodies. */
    
    bool warmstarting; /**< Flag that specifies if solvers use warm-starting for accumulated impulses. */
    nv_float baumgarte; /**< Baumgarte stabilization factor. */
    int collision_persistence; /**< Number of frames the collision resolutions kept cached. */

    nv_BroadPhase broadphase_algorithm; /**< Broad-phase algorithm used to detect possible collisions. */
    nv_HashMap *pairs;
    nv_SHG *shg; /**< Spatial Hash Grid object.
                      @warning Only accessible if the used broad-phase algorithm is SHG. */

    nv_AABB kill_bounds; /**< Boundary where bodies get deleted if they go out of. */
    bool use_kill_bounds; /**< Whether to use the kill bounds or not. True by default. */

    nv_CoefficientMix mix_restitution; /**< Method to mix restitution coefficients of collided bodies. */
    nv_CoefficientMix mix_friction; /**< Method to mix friction coefficients of collided bodies. */

    void *callback_user_data; /**< User data passed to collision callbacks. */
    nv_Space_callback before_collision; /**< Callback function called before solving collisions. */
    nv_Space_callback after_collision; /**< Callback function called after solving collisions. */

    nv_Profiler profiler; /**< Profiler. */
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
 * @brief Remove body from the space.
 * 
 * The removal will not pe performed until the current simulation step ends.
 * After removing the body managing body's memory belongs to user. You should
 * use @ref nv_Body_free if you are not going to add it to the space again.
 * 
 * @param space Space
 * @param body Body to remove
 */
void nv_Space_remove(nv_Space *space, nv_Body *body);

/**
 * @brief Remove body from the space and free it.
 * 
 * The removal will not pe performed until the current simulation step ends.
 * Unlike @ref nv_Space_remove, this method also frees the body. It can be
 * useful in games where references to bullets aren't usually kept.
 * 
 * @param space Space
 * @param body Body to remove and free
 */
void nv_Space_kill(nv_Space *space, nv_Body *body);

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
 * Iteration counts are how many iterations the solver uses to solve and converge constraints.
 * Higher the iteration count, more accurate simulation but higher CPU usage, thus lower performance.
 * For a game, it is sufficient to keep iteration counts around 5-10.
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