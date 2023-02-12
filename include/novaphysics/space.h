/*

  This file is a part of the Nova Physics Engine
  project and distributed under the MIT license.

  Copyright © Kadir Aksoy
  https://github.com/kadir014/nova-physics

*/

#ifndef NOVAPHYSICS_SPACE_H
#define NOVAPHYSICS_SPACE_H

#include "novaphysics/internal.h"
#include "novaphysics/body.h"
#include "novaphysics/resolution.h"
#include "novaphysics/contact.h"


/**
 * space.h
 * 
 * Space
 */


// Space callback type
typedef void ( *nv_Space_callback)(nv_ResolutionArray *res_arr, void *user_data);


/**
 * Space struct
 * 
 * @param bodies Body array
 * @param attractors Attractor bodies array
 * @param gravity Gravity vector
 * @param sleeping Whether to allow sleeping or not
 * @param callback_user_data User data passed to collision callbacks
 * @param before_collision Callback function called before solving collision
 * @param after_collision Callback function called after solving collision
 */
struct _nv_Space{
    nv_BodyArray *bodies;
    nv_BodyArray *attractors;

    nv_Vector2 gravity;
    
    bool sleeping;

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
 * @brief Add body to space
 * 
 * @param body Body to add
 */
void nv_Space_add(nv_Space *space, nv_Body *body);

/**
 * @brief Advance the simulation
 * 
 * @param space Space instance
 * @param dt Time step length (delta time)
 * @param iterations Iteration count
 * @param substeps Substep count
 */
void nv_Space_step(
    nv_Space *space,
    double dt,
    int iterations,
    int substeps
);


typedef struct {
    nv_Body *a;
    nv_Body *b;
} nv_BodyPair;

typedef struct {
    size_t size;
    nv_BodyPair *data;
} nv_BodyPairArray;

nv_BodyPairArray *nv_BodyPairArray_new();

void nv_BodyPairArray_free(nv_BodyPairArray *array);

void nv_BodyPairArray_add(nv_BodyPairArray *array, nv_BodyPair pair);


nv_BodyPair *nv_Space_broadphase(nv_Space *space, nv_BodyPairArray *pairs);

nv_ResolutionArray *nv_Space_narrowphase(
    nv_Space *space,
    nv_BodyPairArray *pairs
);


#endif