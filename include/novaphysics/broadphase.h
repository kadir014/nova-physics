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
#include "novaphysics/bvh.h"


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
    nvBVHProxy *a;
    nvBVHProxy *b;
} nvBroadPhasePair;

/**
 * @brief Do BVH broadphase and update pairs.
 * 
 * @param space 
 */
void nv_broadphase_BVH(struct nvSpace *space);

void nv_broadphase_finalize(struct nvSpace *space);


#endif