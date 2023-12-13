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
 * @brief Collsion resolution states.
 */
typedef enum {
    nvResolutionState_FIRST, /**< The collision just happened this frame. */
    nvResolutionState_NORMAL, /**< The collision has been existing. */
    nvResolutionState_CACHED /**< The collision has been separated and the resolution is cached. */
} nvResolutionState;


/**
 * @brief Data structure that holds information about contacts of collision.
 */
typedef struct {
    nvVector2 position; /**< Position of the contact point. */
    nvVector2 ra; /**< Contact position relative to body A. */
    nvVector2 rb; /**< Contact position relative to body B. */

    nv_float velocity_bias; /**< Velocity bias for restitution. */
    nv_float position_bias; /**< Position correction bias. */

    nv_float mass_normal; /**< Effective mass of normal impulse. */
    nv_float mass_tangent; /**< Effective mass of tangential impulse. */

    nv_float jn; /**< Accumulated normal impulse. */
    nv_float jb; /**< Accumulated pseudo-impulse. */
    nv_float jt; /**< Accumulated tangential impulse. */
} nvContact;


/**
 * @brief Data structure that holds information about collision between two bodies.
 */
typedef struct {
    bool collision; /**< Flag that reports if the collision has happened. */

    nvBody *a; /**< First body of the collision. */
    nvBody *b; /**< Second body of the collision. */

    nvVector2 normal; /**< Normal vector of the collision separation. */
    nv_float depth; /**< Penetration depth. */

    nv_float friction; /**< Mixed friction coefficient. */

    nvResolutionState state; /**< State of the resolution. */
    int lifetime; /**< Remaining lifetime of the resolution in ticks. */
    
    nvContact contacts[2]; /**< Contact points. */
    nv_uint8 contact_count; /**< Contact point count. */
} nvResolution;


/**
 * @brief Update state of the resolved collision resolution.
 * 
 * @param space Space
 * @param res Resolution to update
 */
void nvResolution_update(struct nvSpace *space, nvResolution *res);


#endif