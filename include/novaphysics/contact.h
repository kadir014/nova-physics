/*

  This file is a part of the Nova Physics Engine
  project and distributed under the MIT license.

  Copyright © Kadir Aksoy
  https://github.com/kadir014/nova-physics

*/

#ifndef NOVAPHYSICS_CONTACT_H
#define NOVAPHYSICS_CONTACT_H

#include "novaphysics/vector.h"
#include "novaphysics/array.h"
#include "novaphysics/body.h"
#include "novaphysics/resolution.h"


/**
 * @file contact.h
 * 
 * @brief Contact point calculation functions.
 */


/**
 * @brief Calculate contact point between circle bodies
 * 
 * @param res Collision resolution
 * @return nvVector2 
 */
void nv_contact_circle_x_circle(nvResolution *res);

/**
 * @brief Calculate contact point between polygon and circle body
 * 
 * @param polygon Polygon body
 * @param circle Circle body
 * @return nvVector2 
 */
void nv_contact_polygon_x_circle(nvResolution *res);

/**
 * @brief Calculate contact points between polygon bodies
 * 
 * @param a First polygon body
 * @param b Second polygon body
 * @return nvContacts 
 */
void nv_contact_polygon_x_polygon(nvResolution *res);


#endif