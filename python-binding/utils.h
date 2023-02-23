/*

  This file is a part of the Nova Physics Engine
  project and distributed under the MIT license.

  Copyright © Kadir Aksoy
  https://github.com/kadir014/Nova-Physics

*/

#ifndef NV_PYTHON_UTILS_H
#define NV_PYTHON_UTILS_H

#include <Python.h>


/**
 * Raise exception
 */
#define RAISE_EXC(exc, msg) {  \
    PyErr_SetString(exc, msg); \
    return 0;                  \
}


#endif