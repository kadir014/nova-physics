/*

  This file is a part of the Nova Physics Engine
  project and distributed under the MIT license.

  Copyright © Kadir Aksoy
  https://github.com/kadir014/nova-physics

*/

#ifndef NOVAPHYSICS_RESOLUTION_H
#define NOVAPHYSICS_RESOLUTION_H

#include <stdbool.h>
#include "novaphysics/internal.h"
#include "novaphysics/body.h"


/**
 * @file resolution.h
 * 
 * @brief Collision resolution data structure.
 */


/**
 * @brief Data structure that holds information about collision between two bodies.
 */
typedef struct {
    bool collision; /**< Flag that reports if the collision has happened. */

    nv_Body *a; /**< First body of the collision. */
    nv_Body *b; /**< Second body of the collision. */

    nv_Vector2 normal; /**< Normal vector of the collision separation. */
    nv_float depth; /**< Penetration depth. */
    
    nv_Vector2 contacts[2]; /**< Contact points. */
    int contact_count; /**< Contact point count. */

    nv_float restitution[2]; /**< Mixed restitution coefficient. */
    nv_float friction; /**< Mixed friction coefficient. */
    nv_float bias; /**< Position correction bias. */
    nv_float mass_normal; /**< Effective mass of normal impulse. */
    nv_float mass_tangent; /**< Effective mass of tangential impulse. */
    nv_float jn[2]; /**< Accumulated normal impulse. */
    nv_float jb[2]; /**< Accumulated pseudo-impulse. */
    nv_float jt[2]; /**< Accumulated tangential impulse. */
} nv_Resolution;


#endif