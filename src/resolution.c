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
    if (space->multithreading) {
        if (!nvMutex_lock(space->res_mutex)) {
            NV_ERROR("Error occured while locking res mutex. (nvResolution_update)\n");
        }
    }

    if (
        res->state == nvResolutionState_FIRST ||
        res->state == nvResolutionState_NORMAL
    ) {
        res->state = nvResolutionState_CACHED;
        res->collision = false;
    }

    else if (res->state == nvResolutionState_CACHED) {
        if (res->lifetime <= 0) {
            nvHashMap_remove(space->res, &(nvResolution){.a=res->a, .b=res->b});
        }
        else {
            res->lifetime--;
        }
    }

    if (space->multithreading) {
        if (!nvMutex_unlock(space->res_mutex)) {
           NV_ERROR("Error occured while unlocking res mutex. (nvResolution_update)\n");
        }
    }
}