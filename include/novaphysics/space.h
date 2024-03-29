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
#include "novaphysics/contact_solver.h"
#include "novaphysics/hashmap.h"
#include "novaphysics/shg.h"
#include "novaphysics/threading.h"
#include "novaphysics/profiler.h"


/**
 * @file space.h
 * 
 * @brief Space struct and its methods.
 */


// Space callback type
typedef void ( *nvSpace_callback)(struct nvSpace *space, void *user_data);


/**
 * @brief Space struct.
 * 
 * Space is the core of the simulation.
 * It manages and simulates all bodies, constraints and collisions.
 */
struct nvSpace {
    nvArray *bodies; /**< Array of bodies in the space. */
    nvArray *awake_bodies;
    nvArray *attractors; /**< Array of attractive bodies in the space. */
    nvArray *constraints; /**< Array of constraints in the space. */

    nvArray *_removed_bodies; /**< Bodies that are waiting to be removed.
                                    You shouldn't access this directly, instead use @ref nvSpace_remove method. */
    nvArray *_killed_bodies; /**< Bodies that are waiting to be removed and freed.
                                   You shouldn't access this directly, instead use @ref nvSpace_kill method.*/

    nvHashMap *res; /**< Set of collision resolutions. */

    nvVector2 gravity; /**< Global and uniform gravity applied to all bodies in the space.
                             For gravitational attraction between body pairs, see attractive bodies. */
    
    bool sleeping; /**< Flag that specifies if space allows sleeping of bodies. */
    nv_float sleep_energy_threshold; /**< Threshold value which bodies sleep if they exceed it. */
    nv_float wake_energy_threshold; /**< Threshold value which bodies wake up if they exceed it. */
    unsigned int sleep_timer_threshold; /**< How long space should count to before sleeping bodies. */
    
    bool warmstarting; /**< Flag that specifies if solvers use warm-starting for accumulated impulses. */
    int collision_persistence; /**< Number of frames the collision resolutions kept cached. */
    nvPositionCorrection position_correction; /**< Position correction algorithm used. */

    nvBroadPhaseAlg broadphase_algorithm; /**< Broad-phase algorithm used to detect possible collisions. */
    nvHashMap *broadphase_pairs;
    nvSHG *shg; /**< Spatial Hash Grid object.
                     @warning Should be only accessed if the used broad-phase algorithm is SHG. */

    nvAABB kill_bounds; /**< Boundary where bodies get deleted if they go out of. */
    bool use_kill_bounds; /**< Whether to use the kill bounds or not. True by default. */

    nvCoefficientMix mix_restitution; /**< Method to mix restitution coefficients of collided bodies. */
    nvCoefficientMix mix_friction; /**< Method to mix friction coefficients of collided bodies. */

    void *callback_user_data; /**< User data passed to collision callbacks. */
    nvSpace_callback before_collision; /**< Callback function called before solving collisions. */
    nvSpace_callback after_collision; /**< Callback function called after solving collisions. */

    nvProfiler profiler; /**< Profiler. */

    bool multithreading; /**< Whether multi-threading is enabled or not. */
    size_t thread_count; /**< Number of threads Nova Physics utilizes.
                              0 if multithreading is disabled. */
    nvTaskExecutor *task_executor; /**< Task executor. */
    nvArray *mt_shg_pairs;
    nvArray *mt_shg_bins;

    nv_uint16 _id_counter; /**< Internal ID counter. */
};

typedef struct nvSpace nvSpace;

/**
 * @brief Create new space instance.
 * 
 * @return nvSpace * 
 */
nvSpace *nvSpace_new();

/**
 * @brief Free space.
 * 
 * @param space Space to free
 */
void nvSpace_free(nvSpace *space);

/**
 * @brief Set the current broadphase algorithm used to check possible collision pairs.
 * 
 * @param space Space
 * @param broadphase_type Broadphase algorithm
 */
void nvSpace_set_broadphase(nvSpace *space, nvBroadPhaseAlg broadphase_alg_type);

/**
 * @brief Create & set a new SHG and release the old one.
 * 
 * @param space Space
 * @param bounds Boundaries of the new SHG
 * @param cell_width Cell width of the new SHG
 * @param cell_height Cell height of the new SHG
 */
void nvSpace_set_SHG(
    nvSpace *space,
    nvAABB bounds,
    nv_float cell_width,
    nv_float cell_height
);

/**
 * @brief Clear and free everything in space.
 * 
 * @param space Space
 */
void nvSpace_clear(nvSpace *space);

/**
 * @brief Add body to space.
 * 
 * @param space Space
 * @param body Body to add
 */
void nvSpace_add(nvSpace *space, nvBody *body);

/**
 * @brief Remove body from the space.
 * 
 * The removal will not pe performed until the current simulation step ends.
 * After removing the body managing body's memory belongs to user. You should
 * use @ref nvBody_free if you are not going to add it to the space again.
 * 
 * @param space Space
 * @param body Body to remove
 */
void nvSpace_remove(nvSpace *space, nvBody *body);

/**
 * @brief Remove body from the space and free it.
 * 
 * The removal will not pe performed until the current simulation step ends.
 * Unlike @ref nvSpace_remove, this method also frees the body. It can be
 * useful in games where references to bullets aren't usually kept.
 * 
 * @param space Space
 * @param body Body to remove and free
 */
void nvSpace_kill(nvSpace *space, nvBody *body);

/**
 * @brief Add constraint to space.
 * 
 * @param space Space
 * @param cons Constraint to add
 */
void nvSpace_add_constraint(nvSpace *space, nvConstraint *cons);

/**
 * @brief Advance the simulation.
 * 
 * Iteration counts defines how many iterations the solver uses to converge constraints.
 * Higher the iteration count, more accurate simulation but higher CPU usage, thus lower performance.
 * Velocity and position iteration counts are used for the contact constraint solver.
 * Constraint iteration count is used for other constraints like joints.
 * For a game, it is usually sufficient to keep them around 5-10.
 * 
 * Substep count defines how many substeps the current simulation step is going to get
 * divided into. This effectively increases the accuracy of the simulation but
 * also impacts the performance greatly because the whole simulation is processed 
 * and collisions are recalculated by given amounts of times internally. In a game,
 * you wouldn't need this much detail. Best to leave it at 1.
 * 
 * @param space Space instance
 * @param dt Time step size (delta time)
 * @param velocity_iters Velocity solving iteration count
 * @param position_iters Position solving iteration count
 * @param constraint_iters Constraint solving iteration count
 * @param substeps Substep count
 */
void nvSpace_step(
    nvSpace *space,
    nv_float dt,
    size_t velocity_iters,
    size_t position_iters,
    size_t constraint_iters,
    size_t substeps
);

/**
 * @brief Enable sleeping.
 * 
 * @param space Space
 */
void nvSpace_enable_sleeping(nvSpace *space);

/**
 * @brief Disable sleeping.
 * 
 * @param space Space
 */
void nvSpace_disable_sleeping(nvSpace *space);

/**
 * @brief Enable multithreading.
 * 
 * If the given number of threads is 0, system's CPU core count is used.
 * 
 * @param space Space
 * @param threads Number of threads
 */
void nvSpace_enable_multithreading(nvSpace *space, size_t threads);

/**
 * @brief Disable multithreading.
 * 
 * @param space Space
 */
void nvSpace_disable_multithreading(nvSpace *space);


#endif