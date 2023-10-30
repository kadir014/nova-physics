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
 * @brief Collsiion resolution states.
 */
typedef enum {
    nv_ResolutionState_FIRST, /**< The collision just happened this frame. */
    nv_ResolutionState_NORMAL, /**< The collision has been existing. */
    nv_ResolutionState_CACHED /**< The collision is separated and the resolution is cached. */
} nv_ResolutionState;


/**
 * @brief Data structure that holds information about contacts of collision.
 */
typedef struct {
    nv_Vector2 position; /**< Position of the contact point. */
    nv_Vector2 ra; /**< Contact position relative to body A. */
    nv_Vector2 rb; /**< Contact position relative to body B. */

    nv_float velocity_bias; /**< Velocity bias for restitution. */
    nv_float position_bias; /**< Position correction bias. */

    nv_float mass_normal; /**< Effective mass of normal impulse. */
    nv_float mass_tangent; /**< Effective mass of tangential impulse. */

    nv_float jn; /**< Accumulated normal impulse. */
    nv_float jb; /**< Accumulated pseudo-impulse. */
    nv_float jt; /**< Accumulated tangential impulse. */
} nv_Contact;


/**
 * @brief Data structure that holds information about collision between two bodies.
 */
typedef struct {
    bool collision; /**< Flag that reports if the collision has happened. */

    nv_Body *a; /**< First body of the collision. */
    nv_Body *b; /**< Second body of the collision. */

    nv_Vector2 normal; /**< Normal vector of the collision separation. */
    nv_float depth; /**< Penetration depth. */

    nv_float friction; /**< Mixed friction coefficient. */

    nv_ResolutionState state; /**< State of the resolution. */
    int lifetime; /**< Remaining lifetime of the resolution in ticks. */
    
    nv_Contact contacts[2]; /**< Contact points. */
    int contact_count; /**< Contact point count. */
} nv_Resolution;


#endif