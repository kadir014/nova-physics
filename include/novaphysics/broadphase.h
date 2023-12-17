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
    nvBroadPhaseAlg_BRUTE_FORCE, /**< Naive brute-force approach. */
    nvBroadPhaseAlg_SPATIAL_HASH_GRID, /**< Spatial hash grid. */
    nvBroadPhaseAlg_BOUNDING_VOLUME_HIERARCHY /**< BVH (Bounding Volume Hierarchy) tree.*/
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
void nvBroadPhase_SHG_parallel(struct nvSpace *space);

/**
 * @brief BVH tree algorithm.
 * 
 * @param space Space
 */
void nvBroadPhase_BVH(struct nvSpace *space);

/**
 * @brief Multi-hreaded BVH tree algorithm.
 * 
 * @param space Space
 */
void nvBroadPhase_BVH_parallel(struct nvSpace *space);


#endif