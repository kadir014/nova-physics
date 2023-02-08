/*

  This file is a part of the Nova Physics Engine
  project and distributed under the MIT license.

  Copyright © Kadir Aksoy
  https://github.com/kadir014/Nova-Physics

*/

#ifndef NOVAPHYSICS_CONTACT_H
#define NOVAPHYSICS_CONTACT_H

#include "novaphysics/vector.h"
#include "novaphysics/body.h"
#include "novaphysics/resolution.h"


/**
 * contact.h
 * 
 * Contact point calculation functions
 */


/**
 * @brief Calculate contact point(s) between circle bodies
 * 
 * @param res Collision resolution
 * @return nv_Vector2 
 */
void nv_contact_circle_x_circle(nv_Resolution *res);

/**
 * @brief Calculate contact point(s) between polygon and circle body
 * 
 * @param polygon Polygon body
 * @param circle Circle body
 * @return nv_Vector2 
 */
void nv_contact_polygon_x_circle(nv_Resolution *res);

/**
 * @brief Calculate contact point(s) between polygon bodies
 * 
 * @param a First polygon body
 * @param b Second polygon body
 * @return nv_Contacts 
 */
void nv_contact_polygon_x_polygon(nv_Resolution *res);


#endif