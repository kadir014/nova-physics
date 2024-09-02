/*

  This file is a part of the Nova Physics Engine
  project and distributed under the MIT license.

  Copyright © Kadir Aksoy
  https://github.com/kadir014/nova-physics

*/

#include "novaphysics/core/error.h"


/**
 * @file core/error.c
 * 
 * @brief Error handling.
 */


char _nv_error_buffer[NV_ERROR_BUFFER_SIZE] = "";

char *nv_get_error() {
    return _nv_error_buffer;
}