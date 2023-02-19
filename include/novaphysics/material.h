/*

  This file is a part of the Nova Physics Engine
  project and distributed under the MIT license.

  Copyright © Kadir Aksoy
  https://github.com/kadir014/nova-physics

*/

#ifndef NOVAPHYSICS_MATERIAL_H
#define NOVAPHYSICS_MATERIAL_H


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
 * @param static_friction Static friction coefficient of material
 * @param dynamic_friction Dynamic friction coefficient of material
 */
typedef struct {
    double density;
    double restitution;
    double static_friction;
    double dynamic_friction;
} nv_Material;


/*
    Common material definitions

    Values below are mostly guesses and estimates
    gathered from different sources.
*/


static const nv_Material nv_Material_STEEL = {
    .density = 7.8,
    .restitution = 0.63,
    .static_friction = 0.6,
    .dynamic_friction = 0.4
};

static const nv_Material nv_Material_WOOD = {
    .density = 1.5,
    .restitution = 0.37,
    .static_friction = 0.55,
    .dynamic_friction = 0.3
};

static const nv_Material nv_Material_GLASS = {
    .density = 2.5,
    .restitution = 0.55,
    .static_friction = 0.3,
    .dynamic_friction = 0.19
};

static const nv_Material nv_Material_ICE = {
    .density = 0.92,
    .restitution = 0.05,
    .static_friction = 0.07,
    .dynamic_friction = 0.02
};

static const nv_Material nv_Material_CONCRETE = {
    .density = 3.6,
    .restitution = 0.2,
    .static_friction = 0.89,
    .dynamic_friction = 0.6
};

static const nv_Material nv_Material_RUBBER = {
    .density = 1.4,
    .restitution = 0.89,
    .static_friction = 0.93,
    .dynamic_friction = 0.75
};

static const nv_Material nv_Material_GOLD = {
    .density = 19.3,
    .restitution = 0.4,
    .static_friction = 0.49,
    .dynamic_friction = 0.2
};

static const nv_Material nv_Material_CARDBOARD = {
    .density = 0.6,
    .restitution = 0.02,
    .static_friction = 0.3,
    .dynamic_friction = 0.1
};


#endif