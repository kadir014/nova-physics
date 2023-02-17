/*

  This file is a part of the Nova Physics Engine
  project and distributed under the MIT license.

  Copyright © Kadir Aksoy
  https://github.com/kadir014/nova-physics

*/

#ifndef NOVAPHYSICS_INTERNAL_H
#define NOVAPHYSICS_INTERNAL_H

#include <stdlib.h>


/**
 * types.h
 * 
 * Nova Physics utility functions and struct forward declarations
 * to prevent circular includes
 */

struct _nv_Space;


// Utility macro to create objects on HEAP
#define NV_NEW(type) ((type *)malloc(sizeof(type)))
  

#endif