/*

  This file is a part of the Nova Physics Engine
  project and distributed under the MIT license.

  Copyright © Kadir Aksoy
  https://github.com/kadir014/nova-physics

*/

#ifndef NOVAPHYSICS_SPACE_H
#define NOVAPHYSICS_SPACE_H

#include "novaphysics/internal.h"
#include "novaphysics/core/array.h"
#include "novaphysics/core/hashmap.h"
#include "novaphysics/core/pool.h"
#include "novaphysics/body.h"
#include "novaphysics/broadphase.h"
#include "novaphysics/contact.h"
#include "novaphysics/constraints/constraint.h"
#include "novaphysics/constraints/contact_constraint.h"
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
    nvHashMap *contacts;
    nvHashMap *removed_contacts;
    nvMemoryPool *broadphase_pairs;
    nvArray *bvh_traversed;
    nv_uint32 id_counter;

    /*
        Public members (setters & getters)
    */
    nvVector2 gravity;
    nvSpaceSettings settings;
    nvBroadPhaseAlg broadphase_algorithm;

    nvContactListener *listener;
    void *listener_arg;

    nvProfiler profiler;
};
typedef struct nvSpace nvSpace;

/**
 * @brief Create new space instance.
 * 
 * Returns `NULL` on error. Use @ref nv_get_error to get more information.
 * 
 * @return nvSpace * 
 */
nvSpace *nvSpace_new();

/**
 * @brief Free space.
 * 
 * It's safe to pass `NULL` to this function.
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
 * @param space Space
 * @return nvBroadPhaseAlg 
 */
nvBroadPhaseAlg nvSpace_get_broadphase(const nvSpace *space);

/**
 * @brief Get the current simulation settings struct.
 * 
 * This returns a pointer to the current settings. So you can directly modify it.
 * 
 * @param space Space
 * @return nvSpaceSettings *
 */
nvSpaceSettings *nvSpace_get_settings(nvSpace *space);

/**
 * @brief Get profiler of space.
 * 
 * @param space Space
 * @return nvProfiler 
 */
nvProfiler nvSpace_get_profiler(const nvSpace *space);

/**
 * @brief Set the current contact event listener.
 * 
 * Space allocates using the functions provided by listener param.
 * 
 * Returns non-zero on error. Use @ref nv_get_error to get more information.
 * 
 * @param space Space
 * @param listener Contact event listener
 * @param user_arg User argument
 */
int nvSpace_set_contact_listener(
    nvSpace *space,
    nvContactListener listener,
    void *user_arg
);

/**
 * @brief Get the current contact event listener.
 * 
 * @param space Space
 * @return nvContactListener * 
 */
nvContactListener *nvSpace_get_contact_listener(const nvSpace *space);

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
int nvSpace_add_rigidbody(nvSpace *space, nvRigidBody *body);

/**
 * @brief Remove body from the space.
 * 
 * After removing the body, managing it's memory belongs to user. You should
 * use @ref nvRigidBody_free if you are not going to add it to the space again.
 * 
 * This function also removes any constraints attached to the body.
 * 
 * Returns non-zero on error. Use @ref nv_get_error to get more information.
 * 
 * @param space Space
 * @param body Body to remove
 * @return int Status
 */
int nvSpace_remove_rigidbody(nvSpace *space, nvRigidBody *body);

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
 * @brief Remove constraint from the space.
 * 
 * After removing the constraint managing it's memory belongs to user. You should
 * use @ref nvConstraint_free if you are not going to add it to the space again.
 * 
 * Returns non-zero on error. Use @ref nv_get_error to get more information.
 * 
 * @param space Space
 * @param cons Constraint to remove
 * @return int 
 */
int nvSpace_remove_constraint(nvSpace *space, nvConstraint *cons);

/**
 * @brief Iterate over the rigid bodies in this space.
 * 
 * Make sure to reset the index if you alter the space in any way while iterating.
 * 
 * @param space Space
 * @param body Pointer to rigid body
 * @param index Pointer to iteration index
 * @return nv_bool 
 */
nv_bool nvSpace_iter_bodies(nvSpace *space, nvRigidBody **body, size_t *index);

/**
 * @brief Iterate over the constraints in this space.
 * 
 * Make sure to reset the index if you alter the space in any way while iterating.
 * 
 * @param space Space
 * @param cons Pointer to constraint
 * @param index Pointer to iteration index
 * @return nv_bool 
 */
nv_bool nvSpace_iter_constraints(nvSpace *space, nvConstraint **cons, size_t *index);

/**
 * @brief Advance the simulation.
 * 
 * @param space Space instance
 * @param dt Time step size (delta time)
 */
void nvSpace_step(nvSpace *space, nv_float dt);

/**
 * @brief Cast a ray in space and collect intersections.
 * 
 * @param space Space
 * @param from Starting position of ray in world space
 * @param to End position of ray in world space
 * @param results_array Array of ray cast result structs to be filled
 * @param num_hits Number of hits (size of results array)
 * @param capacity Size allocated for the results array
 */
void nvSpace_cast_ray(
    nvSpace *space,
    nvVector2 from,
    nvVector2 to,
    nvRayCastResult *results_array,
    size_t *num_hits,
    size_t capacity
);


#endif