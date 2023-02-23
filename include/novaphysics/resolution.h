/*

  This file is a part of the Nova Physics Engine
  project and distributed under the MIT license.

  Copyright © Kadir Aksoy
  https://github.com/kadir014/nova-physics

*/

#ifndef NOVAPHYSICS_RESOLUTION_H
#define NOVAPHYSICS_RESOLUTION_H

#include <stdbool.h>
#include "novaphysics/body.h"


/**
 * resolution.h
 * 
 * Collision resolution (manifold) data structure
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
 * @param e Mixed restitution of bodies
 * @param sf Mixed static friction of bodies
 * @param df Mixed dynamic friction of bodies
 * @param bias Position correction bias
 * @param mass_normal Mass of normal impulse
 * @param mass_tangent Mass of tangential impulse
 * @param jn Accumulated normal impulse
 * @param jt Accumulated tangential impulse
 */
typedef struct {
    bool collision;

    nv_Body *a;
    nv_Body *b;

    nv_Vector2 normal;
    double depth;
    
    nv_Vector2 contacts[2];
    int contact_count;

    double e;
    double sf;
    double df;
    double bias;
    double mass_normal;
    double mass_tangent;
    double jn[2];
    double jt[2];
} nv_Resolution;


#endif