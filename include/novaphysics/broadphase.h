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
 * @brief Pair of two possibly colliding bodies,
 *        that is going to be used in narrowphase.
 */
typedef struct {
    nvRigidBody *a;
    nvRigidBody *b;
} nvBroadPhasePair;


/**
 * @brief Algorithm used in broad-phase collision detection.
 */
typedef enum {
    nvBroadPhaseAlg_BRUTE_FORCE, /**< Naive brute-force approach.
                                      Every rigid body is checked against each other. O(n^2)*/
    nvBroadPhaseAlg_SHG, /**< SHG (Spatial hash grid).
                              Bodies are layed down on a uniform grid and their cells are hashed
                              so it is faster to check neighboring bodies. */
    nvBroadPhaseAlg_BVH /**< BVH (Bounding Volume Hierarchy) tree.*/
} nvBroadPhaseAlg;


/**
 * @brief Brute-force algorithm.
 * 
 * @param space Space
 */
void nv_broadphase_brute_force(struct nvSpace *space);

/**
 * @brief Spatial hash grid algorithm.
 * 
 * @param space Space
 */
void nv_broadphase_SHG(struct nvSpace *space);

/**
 * @brief Multi-threaded spatial hash grid algorithm.
 * 
 * @param space Space
 */
void nv_broadphase_SHG_parallel(struct nvSpace *space);

/**
 * @brief BVH tree algorithm.
 * 
 * @param space Space
 */
void nv_broadphase_BVH(struct nvSpace *space);

/**
 * @brief Multi-hreaded BVH tree algorithm.
 * 
 * @param space Space
 */
void nv_broadphase_BVH_parallel(struct nvSpace *space);


#endif