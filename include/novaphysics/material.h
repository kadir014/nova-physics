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
    .density = 1.0f,
    .restitution = 0.1f,
    .friction = 0.4f
};

static const nvMaterial nvMaterial_STEEL = {
    .density = 7.8f,
    .restitution = 0.43f,
    .friction = 0.45f
};

static const nvMaterial nvMaterial_WOOD = {
    .density = 1.5f,
    .restitution = 0.37f,
    .friction = 0.52f
};

static const nvMaterial nvMaterial_GLASS = {
    .density = 2.5f,
    .restitution = 0.55f,
    .friction = 0.19f
};

static const nvMaterial nvMaterial_ICE = {
    .density = 0.92f,
    .restitution = 0.05f,
    .friction = 0.02f
};

static const nvMaterial nvMaterial_CONCRETE = {
    .density = 3.6f,
    .restitution = 0.075f,
    .friction = 0.73f
};

static const nvMaterial nvMaterial_RUBBER = {
    .density = 1.4f,
    .restitution = 0.89f,
    .friction = 0.92f
};

static const nvMaterial nvMaterial_GOLD = {
    .density = 19.3f,
    .restitution = 0.4f,
    .friction = 0.35f
};

static const nvMaterial nvMaterial_CARDBOARD = {
    .density = 0.6f,
    .restitution = 0.02f,
    .friction = 0.2f
};


#endif