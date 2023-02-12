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
 * @param a First body of the collision
 * @param b Second body of the collision
 * @param normal Normal vector of the collision surface
 * @param depth Overlap depth
 * @param contacts Contact points
 * @param contact_count Count of contact points
 */
typedef struct {
    bool collision;
    nv_Body *a;
    nv_Body *b;
    nv_Vector2 normal;
    double depth;
    nv_Vector2 contacts[2];
    int contact_count;
} nv_Resolution;


/**
 * @brief Array of nv_Resolution objects
 * 
 * @param size Size of the array
 * @param data Pointer to array
 */
typedef struct {
    size_t size;
    nv_Resolution *data;
} nv_ResolutionArray;

nv_ResolutionArray *nv_ResolutionArray_new();

void nv_ResolutionArray_free(nv_ResolutionArray *array);

void nv_ResolutionArray_add(nv_ResolutionArray *array, nv_Resolution res);


#endif