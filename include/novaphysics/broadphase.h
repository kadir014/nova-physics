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
#include "novaphysics/resolution.h"


/**
 * @file broadphase.h
 * 
 * @brief Broad-phase algorithms.
 */


typedef struct {
    nv_Body *a;
    nv_Body *b;
    uint32_t id_pair;
} nv_BroadPhasePair;


/**
 * @brief Algorithm used in broad-phase collision detection.
 */
typedef enum {
    nv_BroadPhase_BRUTE_FORCE, /**< Brute-force broad-phase algorithm. */
    nv_BroadPhase_SPATIAL_HASH_GRID /**< Spatial hash grid. */
} nv_BroadPhase;

/**
 * @brief Brute-force algorithm.
 * 
 * @param space Space
 */
void nv_BroadPhase_brute_force(struct nv_Space *space);

/**
 * @brief Spatial hash grid algorithm.
 * 
 * @param space Space
 */
void nv_BroadPhase_SHG(struct nv_Space *space);


/**
 * @brief Narrow-phase function that checks the final geometry and generates
 * or updates existing collision resolution.
 * 
 * @param space Space
 * @param a Body A
 * @param b Body B
 * @param res_exists Whether the resolution already exists or not 
 * @param found_res Existing resolution
 */
void nv_narrow_phase(
    struct nv_Space *space,
    nv_Body *a,
    nv_Body *b,
    bool res_exists,
    nv_Resolution *found_res
);


#endif