/*

  This file is a part of the Nova Physics Engine
  project and distributed under the MIT license.

  Copyright © Kadir Aksoy
  https://github.com/kadir014/nova-physics

*/

#ifndef NOVAPHYSICS_BROADPHASE_H
#define NOVAPHYSICS_BROADPHASE_H

#include "novaphysics/internal.h"
#include "novaphysics/body.h"


/**
 * @file broadphase.h
 * 
 * @brief Broad-phase algorithms.
 */


/**
 * @brief Algorithm used to do broad-phase collision detection
 */
typedef enum {
    nv_BroadPhase_BRUTE_FORCE, /**< Brute-force */
    nv_BroadPhase_SPATIAL_HASH_GRID /**< Spatial hash grid */
} nv_BroadPhase;

/**
 * @brief Brute-force algorithm
 * 
 * @param space Space
 */
void nv_BroadPhase_brute_force(struct nv_Space *space);

/**
 * @brief Spatial hash grid algorithm
 * 
 * @param space Space
 */
void nv_BroadPhase_spatial_hash_grid(struct nv_Space *space);


/**
 * @brief Check if broad-phase should early out or not
 * 
 * @param space Space
 * @param a Body A
 * @param b Body B
 * @return bool
 */
bool nv_BroadPhase_early_out(struct nv_Space *space, nv_Body *a, nv_Body *b);


/**
 * @brief Expensive narrow-phase function that is called after making sure
 *        bodies could be colliding. Updates resolutions.
 * 
 * @param space Space
 * @param a Body A
 * @param b Body B
 * @param res_exists Whether the resolution already exists or not 
 * @param res_index Existing resolution's index
 */
void nv_narrow_phase(
    struct nv_Space *space,
    nv_Body *a,
    nv_Body *b,
    bool res_exists,
    nv_uint32 res_key,
    void *res_value
);


#endif