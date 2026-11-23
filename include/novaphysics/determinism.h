/*

  This file is a part of the Nova Physics Engine
  project and distributed under the MIT license.

  Copyright © Kadir Aksoy
  https://github.com/kadir014/nova-physics

*/

#ifndef NOVAPHYSICS_DETERMINISM_H
#define NOVAPHYSICS_DETERMINISM_H

#include "novaphysics/internal.h"


/**
 * @file determinism.h
 * 
 * @brief Same-machine determinism helpers.
 */


/**
 * @brief Sort contact points by their shape IDs before solving.
 * 
 * @param space Space.
 * @return Non-zero on error, use @ref nv_get_error to get more information.
 */
int nv_sort_contacts(struct nvSpace *space);


#endif // NOVAPHYSICS_DETERMINISM_H