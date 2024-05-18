/*

  This file is a part of the Nova Physics Engine
  project and distributed under the MIT license.

  Copyright © Kadir Aksoy
  https://github.com/kadir014/nova-physics

*/

#ifndef NOVAPHYSICS_NARROWPHASE_H
#define NOVAPHYSICS_NARROWPHASE_H

#include "novaphysics/internal.h"
#include "novaphysics/body.h"
#include "novaphysics/resolution.h"
#include "novaphysics/broadphase.h"


/**
 * @file narrowphase.h
 * 
 * @brief Narrow-phase.
 */


/**
 * @brief Check the final geometry between bodies after finding possible collision
 *        pairs using broad-phase algorithms and update collision resolutions.
 * 
 * @param space Space
 */
void nv_narrow_phase(struct nvSpace *space);


/**
 * @brief Do narrow-phase check between bodies of single pair.
 * 
 * @param space Space
 * @param pair Body pair
 * @param res_exists Does the resolution exist
 * @param found_res Resolution instance (if it exists)
 */
void nv_narrow_phase_between_pair(
    struct nvSpace *space,
    nvBroadPhasePair *pair,
    nv_bool res_exists,
    nvResolution *found_res
);


#endif