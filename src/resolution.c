/*

  This file is a part of the Nova Physics Engine
  project and distributed under the MIT license.

  Copyright © Kadir Aksoy
  https://github.com/kadir014/nova-physics

*/

#include "novaphysics/resolution.h"
#include "novaphysics/space.h"
#include "novaphysics/hashmap.h"
#include "novaphysics/threading.h"


/**
 * @file resolution.c
 * 
 * @brief Collision resolution data structure.
 */


void nvResolution_update(nvSpace *space, nvResolution *res) {
    switch (res->state) {
        case nvResolutionState_FIRST:
        case nvResolutionState_NORMAL:
            res->state = nvResolutionState_CACHED;
            res->collision = false;
            break;

        case nvResolutionState_CACHED:
            if (res->lifetime <= 0) {
                nvHashMap_remove(space->res, &(nvResolution){.a=res->a, .b=res->b});
            }
            else {
                res->lifetime--;
            }
            break;
    }
}