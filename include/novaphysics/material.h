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
 * @file material.h
 * 
 * @brief Material struct and common instances.
 */


/**
 * @brief Material struct
 */
typedef struct {
    nv_float density; /**< Density of the material. */
    nv_float restitution; /**< Coefficient of restitution (bounciness or elasticity) of material. */
    nv_float friction; /**< Friction coefficient of material */
} nvMaterial;


/*
    Common material definitions

    Values below are mostly guesses and estimates
    gathered from different sources.
*/

static const nvMaterial nvMaterial_BASIC = {
    .density = (nv_float)1.0,
    .restitution = (nv_float)0.1,
    .friction = (nv_float)0.4
};

static const nvMaterial nvMaterial_STEEL = {
    .density = (nv_float)7.8,
    .restitution = (nv_float)0.43,
    .friction = (nv_float)0.45
};

static const nvMaterial nvMaterial_WOOD = {
    .density = (nv_float)1.5,
    .restitution = (nv_float)0.37,
    .friction = (nv_float)0.52
};

static const nvMaterial nvMaterial_GLASS = {
    .density = (nv_float)2.5,
    .restitution = (nv_float)0.55,
    .friction = (nv_float)0.19
};

static const nvMaterial nvMaterial_ICE = {
    .density = (nv_float)0.92,
    .restitution = (nv_float)0.05,
    .friction = (nv_float)0.02
};

static const nvMaterial nvMaterial_CONCRETE = {
    .density = (nv_float)3.6,
    .restitution = (nv_float)0.075,
    .friction = (nv_float)0.73
};

static const nvMaterial nvMaterial_RUBBER = {
    .density = (nv_float)1.4,
    .restitution = (nv_float)0.89,
    .friction = (nv_float)0.92
};

static const nvMaterial nvMaterial_GOLD = {
    .density = (nv_float)19.3,
    .restitution = (nv_float)0.4,
    .friction = (nv_float)0.35
};

static const nvMaterial nvMaterial_CARDBOARD = {
    .density = (nv_float)0.6,
    .restitution = (nv_float)0.02,
    .friction = (nv_float)0.2
};


#endif