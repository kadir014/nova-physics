/*

  This file is a part of the Nova Physics Engine
  project and distributed under the MIT license.

  Copyright © Kadir Aksoy
  https://github.com/kadir014/nova-physics

*/

#ifndef NOVAPHYSICS_MATERIAL_H
#define NOVAPHYSICS_MATERIAL_H

#include "novaphysics/internal.h"


/**
 * material.h
 * 
 * Material
 */


/**
 * @brief Material struct
 * 
 * @param density Density of material
 * @param restitution Coefficient of restitution (bounciness) of material
 * @param friction Friction coefficient of material
 */
typedef struct {
    nv_float density;
    nv_float restitution;
    nv_float friction;
} nv_Material;


/*
    Common material definitions

    Values below are mostly guesses and estimates
    gathered from different sources.
*/


static const nv_Material nv_Material_STEEL = {
    .density = 7.8,
    .restitution = 0.43,
    .friction = 0.45
};

static const nv_Material nv_Material_WOOD = {
    .density = 1.5,
    .restitution = 0.37,
    .friction = 0.52
};

static const nv_Material nv_Material_GLASS = {
    .density = 2.5,
    .restitution = 0.55,
    .friction = 0.19
};

static const nv_Material nv_Material_ICE = {
    .density = 0.92,
    .restitution = 0.05,
    .friction = 0.02
};

static const nv_Material nv_Material_CONCRETE = {
    .density = 3.6,
    .restitution = 0.2,
    .friction = 0.73
};

static const nv_Material nv_Material_RUBBER = {
    .density = 1.4,
    .restitution = 0.89,
    .friction = 0.92
};

static const nv_Material nv_Material_GOLD = {
    .density = 19.3,
    .restitution = 0.4,
    .friction = 0.35
};

static const nv_Material nv_Material_CARDBOARD = {
    .density = 0.6,
    .restitution = 0.02,
    .friction = 0.2
};


#endif