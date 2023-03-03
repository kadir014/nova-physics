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
 * @details Collision resolution data structure
 */


/**
 * @brief Data structure that holds information
 *        about collision between two bodies
 * 
 * @param collision Whether the collision happened or not
 * 
 * @param a First body of the collision
 * @param b Second body of the collision
 * 
 * @param normal Normal vector of the collision separation
 * @param depth Penetration depth
 * 
 * @param contacts Contact points
 * @param contact_count Count of contact points
 * 
 * @param restitution Mixed restitution coefficient of bodies
 * @param friction Mixed friction coefficient of bodies
 * @param bias Position correction bias
 * @param mass_normal Effective mass of normal impulse
 * @param mass_tangent Effective mass of tangential impulse
 * @param jn Accumulated normal impulse
 * @param jb Accumulated pseudo impulse
 * @param jt Accumulated tangential impulse
 */
typedef struct {
    bool collision;

    nv_Body *a;
    nv_Body *b;

    nv_Vector2 normal;
    nv_float depth;
    
    nv_Vector2 contacts[2];
    int contact_count;

    nv_float restitution[2];
    nv_float friction;
    nv_float bias;
    nv_float mass_normal;
    nv_float mass_tangent;
    nv_float jn[2];
    nv_float jb[2];
    nv_float jt[2];
} nv_Resolution;


#endif