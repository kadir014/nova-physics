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
#include "novaphysics/space_settings.h"


/**
 * @file space.h
 * 
 * @brief Space struct and its methods.
 */


/**
 * @brief Space struct.
 * 
 * A space is the core of the physics simulation.
 * It manages and simulates all bodies, constraints and collisions.
 */
struct nvSpace {
    /*
        Private members
    */
    nvArray *bodies;
    nvArray *constraints;
    nvArray *removed_bodies;
    nvHashMap *contacts;
    nvArray *broadphase_pairs;

    // TODO: 64-bit counter would eliminate ID conflicts in a regular scenario
    nv_uint64 id_counter;

    /*
        Public members (setters & getters)
    */
    nvVector2 gravity;
    nvSpaceSettings settings;
    nvBroadPhaseAlg broadphase_algorithm; /**< Broad-phase algorithm used to detect possible collisions. */

    nvAABB kill_bounds; /**< Boundary where bodies get removed if they go out of. */
    nv_bool use_kill_bounds; /**< Whether to use the kill bounds or not. On by default. */

    nvProfiler profiler; /**< Simulation profiler. */
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
 * It's safe to pass NULL to this function.
 * 
 * @param space Space to free
 */
void nvSpace_free(nvSpace *space);

/**
 * @brief Set global gravity vector.
 * 
 * @param space Space
 * @param gravity Gravity vector
 */
void nvSpace_set_gravity(nvSpace *space, nvVector2 gravity);

/**
 * @brief Get global gravity vector.
 * 
 * @param space Space
 * @return nvVector2 Gravity vector
 */
nvVector2 nvSpace_get_gravity(const nvSpace *space);

/**
 * @brief Set the current broadphase algorithm.
 * 
 * Broadphase is where we check for possible collided pairs of bodies. Quickly
 * determining those pairs is important for efficiency before narrowphase.
 * 
 * @param space Space
 * @param broadphase_type Broadphase algorithm
 */
void nvSpace_set_broadphase(nvSpace *space, nvBroadPhaseAlg broadphase_alg_type);

/**
 * @brief Get the current broadphase algorithm.
 * 
 * @param space 
 * @return nvBroadPhaseAlg 
 */
nvBroadPhaseAlg nvSpace_get_broadphase(const nvSpace *space);

/**
 * @brief Clear bodies and constraints in space.
 * 
 * Returns non-zero on error. Use @ref nv_get_error to get more information.
 * 
 * @param space Space
 * @param free_all Whether to free objects after removing them from space
 * @return int Status
 */
int nvSpace_clear(nvSpace *space, nv_bool free_all);

/**
 * @brief Add body to space.
 * 
 * Returns non-zero on error. Use @ref nv_get_error to get more information.
 * 
 * @param space Space
 * @param body Body to add
 * @return int Status
 */
int nvSpace_add_body(nvSpace *space, nvRigidBody *body);

/**
 * @brief Remove body from the space.
 * 
 * The removal will not be performed until the current simulation step ends.
 * After removing the body managing body's memory belongs to user. You should
 * use @ref nvRigidBody_free if you are not going to add it to the space again.
 * 
 * Returns non-zero on error. Use @ref nv_get_error to get more information.
 * 
 * @param space Space
 * @param body Body to remove
 * @return int Status
 */
int nvSpace_remove_body(nvSpace *space, nvRigidBody *body);

/**
 * @brief Add constraint to space.
 * 
 * Returns non-zero on error. Use @ref nv_get_error to get more information.
 * 
 * @param space Space
 * @param cons Constraint to add
 * @return int Status
 */
int nvSpace_add_constraint(nvSpace *space, nvConstraint *cons);

/**
 * @brief Advance the simulation.
 * 
 * Iteration counts defines how many iterations the solver uses to converge constraints.
 * Higher the iteration count, more accurate simulation but higher CPU usage, thus lower performance.
 * For a game, it is usually sufficient to keep them around 5-10.

 * 
 * @param space Space instance
 * @param dt Time step size (delta time)
 */
void nvSpace_step(nvSpace *space, nv_float dt);


#endif