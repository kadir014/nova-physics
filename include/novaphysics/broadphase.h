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

static inline nv_uint64 nvBroadPhasePair_hash(void *item) {
    nvBroadPhasePair *pair = (nvBroadPhasePair *)item;
    return nv_u32pair(pair->a->id, pair->b->id);
}


/**
 * @brief Algorithm used in broad-phase collision detection.
 */
typedef enum {
    nvBroadPhaseAlg_BRUTE_FORCE, /**< Naive brute-force approach.
                                      Every rigid body is checked against each other. O(n^2)*/

    nvBroadPhaseAlg_BVH /**< BVH (Bounding Volume Hierarchy) tree. */
} nvBroadPhaseAlg;


/**
 * @brief Do brute-force broadphase and update pairs.
 * 
 * @param space Space
 */
void nv_broadphase_brute_force(struct nvSpace *space);

/**
 * @brief Do BVH broadphase and update pairs.
 * 
 * @param space 
 */
void nv_broadphase_BVH(struct nvSpace *space);

void nv_broadphase_finalize(struct nvSpace *space);


#endif