/*

  This file is a part of the Nova Physics Engine
  project and distributed under the MIT license.

  Copyright © Kadir Aksoy
  https://github.com/kadir014/Nova-Physics

*/

#ifndef NOVAPHYSICS_SPACE_H
#define NOVAPHYSICS_SPACE_H

#include "novaphysics/body.h"
#include "novaphysics/resolution.h"
#include "novaphysics/contact.h"


/**
 * space.h
 * 
 * Space
 */


/**
 * Space struct
 * 
 * @param bodies Body array
 * @param gravity Gravity vector
 * @param sleeping Whether to allow sleeping or not
 */
typedef struct {
    nv_BodyArray *bodies;
    nv_Vector2 gravity;
    bool sleeping;
} nv_Space;

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

nv_BodyPair *nv_Space_broadphase(
    nv_Space *space,
    nv_BodyPair pair_out[],
    size_t *pair_count_out
);

nv_ResolutionArray *nv_Space_narrowphase(
    nv_Space *space,
    nv_BodyPair pairs[],
    size_t pair_count
);


#endif