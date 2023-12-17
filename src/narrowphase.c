/*

  This file is a part of the Nova Physics Engine
  project and distributed under the MIT license.

  Copyright © Kadir Aksoy
  https://github.com/kadir014/nova-physics

*/

#include "novaphysics/internal.h"
#include "novaphysics/narrowphase.h"
#include "novaphysics/space.h"


/**
 * @file narrowphase.c
 * 
 * @brief Narrow-phase.
 */


inline void nv_narrow_phase(nvSpace *space) {
    void *map_val;
    size_t l = 0;
    while (nvHashMap_iter(space->broadphase_pairs, &l, &map_val)) {
        nvBroadPhasePair *pair = map_val;

        nvResolution *res_value;
        res_value = nvHashMap_get(space->res, &(nvResolution){.a=pair->a, .b=pair->b});
        bool res_exists = (res_value == NULL) ? false : true;

        nv_narrow_phase_between_pair(space, pair, res_exists, res_value);
    }
}


void nv_narrow_phase_between_pair(
    nvSpace *space,
    nvBroadPhasePair *pair,
    bool res_exists,
    nvResolution *found_res
) {
    NV_TRACY_ZONE_START;

    nvBody *a = pair->a;
    nvBody *b = pair->b;

    nvResolution res;
    res.collision = false;

    if (a->shape->type == nvShapeType_CIRCLE && b->shape->type == nvShapeType_CIRCLE)
        res = nv_collide_circle_x_circle(a, b);

    else if (a->shape->type == nvShapeType_CIRCLE && b->shape->type == nvShapeType_POLYGON)
        res = nv_collide_polygon_x_circle(b, a);

    else if (a->shape->type == nvShapeType_POLYGON && b->shape->type == nvShapeType_CIRCLE)
        res = nv_collide_polygon_x_circle(a, b);

    else if (a->shape->type == nvShapeType_POLYGON && b->shape->type == nvShapeType_POLYGON) {
        res.a = a;
        res.b = b;
        nv_contact_polygon_x_polygon(&res);
    }

    if (res.collision) {
        if (a->shape->type == nvShapeType_CIRCLE && b->shape->type == nvShapeType_CIRCLE)
            nv_contact_circle_x_circle(&res);

        else if (a->shape->type == nvShapeType_CIRCLE && b->shape->type == nvShapeType_POLYGON)
            nv_contact_polygon_x_circle(&res);

        else if (a->shape->type == nvShapeType_POLYGON && b->shape->type == nvShapeType_CIRCLE)
            nv_contact_polygon_x_circle(&res);

        /*
            If one body is asleep and other is not, wake up the asleep body
            depending on the awake body's motion.
        */
        if (space->sleeping) {
            if (a->is_sleeping && (!b->is_sleeping && b->type != nvBodyType_STATIC)) {
                nv_float linear = nvVector2_len2(b->linear_velocity) * (1.0 / 60.0);
                nv_float angular = b->angular_velocity * (1.0 / 60.0);
                nv_float total_energy = linear + angular;

                if (total_energy > space->wake_energy_threshold)
                    nvBody_awake(a);
            }

            if (b->is_sleeping && (!a->is_sleeping && a->type != nvBodyType_STATIC)) {
                nv_float linear = nvVector2_len2(a->linear_velocity) * (1.0 / 60.0);
                nv_float angular = a->angular_velocity * (1.0 / 60.0);
                nv_float total_energy = linear + angular;

                if (total_energy > space->wake_energy_threshold)
                    nvBody_awake(b);
            }
        }

        /*
            If the resolution between bodies already exists then
            just update it. Else, create a new resolution.
        */
        if (res_exists) {
            found_res->normal = res.normal;
            found_res->depth = res.depth;
            found_res->collision = res.collision;
            found_res->contact_count = res.contact_count;
            found_res->contacts[0].position = res.contacts[0].position;
            found_res->contacts[1].position = res.contacts[1].position;

            if (found_res->state == nvResolutionState_CACHED) {
                found_res->lifetime = space->collision_persistence;
                found_res->state = nvResolutionState_FIRST;
            }
            else if (found_res->state == nvResolutionState_FIRST) {
                found_res->state = nvResolutionState_NORMAL;
            }
        }
        else {
            nvResolution res_new;
            res_new.a = res.a;
            res_new.b = res.b;
            res_new.normal = res.normal;
            res_new.depth = res.depth;
            res_new.collision = res.collision;
            res_new.contact_count = res.contact_count;
            res_new.contacts[0] = res.contacts[0];
            res_new.contacts[1] = res.contacts[1];
            res_new.contacts[0].jb = 0.0;
            res_new.contacts[1].jb = 0.0;
            res_new.contacts[0].jn = 0.0;
            res_new.contacts[1].jn = 0.0; 
            res_new.contacts[0].jt = 0.0;
            res_new.contacts[1].jt = 0.0; 
            res_new.state = nvResolutionState_FIRST;
            res_new.lifetime = space->collision_persistence;
            
            nvHashMap_set(space->res, &res_new);
        }
    }

    // If the pair is actually not colliding, update the resolution state
    else if (res_exists) {
        nvResolution_update(space, found_res);
    }

    NV_TRACY_ZONE_END;
}