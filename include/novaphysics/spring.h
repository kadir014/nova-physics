/*

  This file is a part of the Nova Physics Engine
  project and distributed under the MIT license.

  Copyright © Kadir Aksoy
  https://github.com/kadir014/nova-physics

*/

#ifndef NOVAPHYSICS_SPRING_H
#define NOVAPHYSICS_SPRING_H

#include "novaphysics/vector.h"


/**
 * spring.h
 * 
 * Spring constraints
 */


/**
 * @brief Spring constraint head
 * 
 * @param length Length of the spring
 * @param stiffness Stiffnes (strength) of the spring
 * @param damping TODO
 * @param achor_a Local anchor point on body A
 * @param anchor_b Local anchor point on body B
 */
typedef struct {
    double length;
    double stiffness;
    double damping;
    nv_Vector2 anchor_a;
    nv_Vector2 anchor_b;
} nv_Spring;


#endif