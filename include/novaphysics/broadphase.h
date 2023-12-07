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
    nvBody *a;
    nvBody *b;
    uint32_t id_pair;
} nvBroadPhasePair;


/**
 * @brief Algorithm used in broad-phase collision detection.
 */
typedef enum {
    nvBroadPhaseAlg_BRUTE_FORCE, /**< Brute-force broad-phase algorithm. */
    nvBroadPhaseAlg_SPATIAL_HASH_GRID /**< Spatial hash grid. */
} nvBroadPhaseAlg;


/**
 * @brief Brute-force algorithm.
 * 
 * @param space Space
 */
void nvBroadPhase_brute_force(struct nvSpace *space);

/**
 * @brief Spatial hash grid algorithm.
 * 
 * @param space Space
 */
void nvBroadPhase_SHG(struct nvSpace *space);

/**
 * @brief Multi-threaded spatial hash grid algorithm.
 * 
 * @param space Space
 */
void nvBroadPhase_SHG_multithreaded(struct nvSpace *space);


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
    struct nvSpace *space,
    nvBody *a,
    nvBody *b,
    bool res_exists,
    nvResolution *found_res
);


#endif