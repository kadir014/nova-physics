/*

  This file is a part of the Nova Physics Engine
  project and distributed under the MIT license.

  Copyright © Kadir Aksoy
  https://github.com/kadir014/nova-physics

*/

#ifndef NOVAPHYSICS_VERSION_H
#define NOVAPHYSICS_VERSION_H


/**
 * @file types.h
 * 
 * @brief Nova Physics version information.
 */


// Stringify macro
#define _NV_STRINGIFY(x) #x
#define NV_STRINGIFY(x) _NV_STRINGIFY(x)

// Version in MAJOR.MINOR.PATCH format
#define NV_VERSION_MAJOR 1
#define NV_VERSION_MINOR 0
#define NV_VERSION_PATCH 1
// Version string
#define NV_VERSION_STRING                  \
        NV_STRINGIFY(NV_VERSION_MAJOR) "." \
        NV_STRINGIFY(NV_VERSION_MINOR) "." \
        NV_STRINGIFY(NV_VERSION_PATCH)

/**
 * @brief Get current Nova Physics version.
 * 
 * This is for mainly CFFI, you can just use `NV_VERSION_[...]` definitions too.
 * 
 * @return char *
 */
char *nv_get_version() {
    return NV_VERSION_STRING;
}


#endif