/*

  This file is a part of the Nova Physics Engine
  project and distributed under the MIT license.

  Copyright © Kadir Aksoy
  https://github.com/kadir014/nova-physics

*/

#ifndef NOVAPHYSICS_NARROWPHASE_H
#define NOVAPHYSICS_NARROWPHASE_H

#include "novaphysics/internal.h"


/**
 * @file narrowphase.h
 * 
 * @brief Narrow-phase.
 */


/**
 * @brief Check the final geometry between bodies after finding possible collision
 *        pairs using broad-phase algorithms and update collision informations.
 * 
 * @param space Space
 */
void nv_narrow_phase(struct nvSpace *space);


#endif