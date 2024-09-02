/*

  This file is a part of the Nova Physics Engine
  project and distributed under the MIT license.

  Copyright © Kadir Aksoy
  https://github.com/kadir014/nova-physics

*/

#ifndef NOVAPHYSICS_ERROR_H
#define NOVAPHYSICS_ERROR_H

#include <stdio.h>


/**
 * @file core/error.h
 * 
 * @brief Error handling.
 */


#define NV_ERROR_BUFFER_SIZE 512
extern char _nv_error_buffer[NV_ERROR_BUFFER_SIZE];

/**
 * @brief Fill the current error buffer in with related information.
 * 
 * @param message Error message
 */
#define nv_set_error(message) {                     \
    sprintf(                                        \
        _nv_error_buffer,                           \
        "Nova Physics error in %s, line %d: %s\n",  \
        __FILE__, __LINE__, message                 \
    );                                              \
}

/**
 * @brief Get the last occured error.
 * 
 * @return char *
 */
char *nv_get_error();


#endif