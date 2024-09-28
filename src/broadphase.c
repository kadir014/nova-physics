/*

  This file is a part of the Nova Physics Engine
  project and distributed under the MIT license.

  Copyright © Kadir Aksoy
  https://github.com/kadir014/nova-physics

*/

#include "novaphysics/broadphase.h"
#include "novaphysics/core/array.h"
#include "novaphysics/aabb.h"
#include "novaphysics/space.h"
#include "novaphysics/bvh.h"


/**
 * @file broadphase.c
 * 
 * @brief Broad-phase algorithms.
 */


/**
 * @brief Early-out from checking collisions.
 */
static inline nv_bool nvBroadPhase_early_out(
    nvSpace *space,
    nvRigidBody *a,
    nvRigidBody *b
) {
    // Same body or already checked
    if (a->id >= b->id)
        return true;

    // One of the bodies have collision detection disabled
    if (!a->collision_enabled || !b->collision_enabled)
        return true;

    // Two static bodies do not need to interact
    if (a->type == nvRigidBodyType_STATIC && b->type == nvRigidBodyType_STATIC)
        return true;

    // Bodies share the same non-zero group
    if (a->collision_group == b->collision_group && a->collision_group != 0)
        return true;

    // One of the collision mask doesn't fit the category
    if ((a->collision_mask & b->collision_category) == 0 ||
        (b->collision_mask & a->collision_category) == 0)
        return true;

    // TODO: There must be a more efficient way
    for (size_t i = 0; i < space->constraints->size; i++) {
        nvConstraint *cons = space->constraints->data[i];

        if (
            cons->ignore_collision &&
            ((a == cons->a && b == cons->b) || (a == cons->b && b == cons->a))
        ) {
            return true;
        }
    }

    return false;
}


void nv_broadphase_brute_force(nvSpace *space) {
    NV_TRACY_ZONE_START;

    nvMemoryPool_clear(space->broadphase_pairs);

    for (size_t i = 0; i < space->bodies->size; i++) {
        nvRigidBody *a = (nvRigidBody *)space->bodies->data[i];
        nvTransform xform_a = (nvTransform){a->origin, a->angle};
        nvAABB abox = nvRigidBody_get_aabb(a);

        for (size_t j = 0; j < space->bodies->size; j++) {
            nvRigidBody *b = (nvRigidBody *)space->bodies->data[j];

            if (nvBroadPhase_early_out(space, a, b)) continue;

            nvBroadPhasePair pair = {a, b};

            nvTransform xform_b = (nvTransform){b->origin, b->angle};
            nvAABB bbox = nvRigidBody_get_aabb(b);

            // First check the body AABB, then check every shape AABB
            // TODO: Improve this & use in BVH as well
            nv_bool overlaps = false;
            if (nv_collide_aabb_x_aabb(abox, bbox)) {
                for (size_t k = 0; k < a->shapes->size; k++) {
                    nvShape *shape_a = a->shapes->data[k];
                    nvAABB sabox = nvShape_get_aabb(shape_a, xform_a);

                    for (size_t l = 0; l < b->shapes->size; l++) {
                        nvShape *shape_b = b->shapes->data[l];
                        nvAABB sbbox = nvShape_get_aabb(shape_b, xform_b);

                        if (nv_collide_aabb_x_aabb(sabox, sbbox)) {
                            overlaps = true;
                            break;
                        }
                    }

                    if (overlaps)
                        break;
                }
            }

            if (overlaps) {
                nvMemoryPool_add(space->broadphase_pairs, &pair);
            }
        }
    }

    NV_TRACY_ZONE_END;
}


void nv_broadphase_BVH(nvSpace *space) {
    NV_TRACY_ZONE_START;

    nvMemoryPool_clear(space->broadphase_pairs);

    nvPrecisionTimer timer;
    NV_PROFILER_START(timer);
    // Prepare median splitting coords
    for (size_t i = 0; i < space->bodies->size; i++) {
        nvRigidBody *body = space->bodies->data[i];
        nvAABB aabb = nvRigidBody_get_aabb(body);
        body->bvh_median_x = (aabb.min_x + aabb.max_x) * 0.5;
        body->bvh_median_y = (aabb.min_y + aabb.max_y) * 0.5;
    }

    // Build the tree top-down
    nvBVHNode *bvh = nvBVHTree_new(space->bodies);
    NV_PROFILER_STOP(timer, space->profiler.bvh_build);

    NV_PROFILER_START(timer);
    for (size_t i = 0; i < space->bodies->size; i++) {
        nvRigidBody *a = space->bodies->data[i];
        nvAABB aabb = nvRigidBody_get_aabb(a);

        nvArray_clear(space->bvh_traversed, NULL);
        nvBVHNode_collide(bvh, aabb, space->bvh_traversed);
        if (space->bvh_traversed->size == 0) continue;

        for (size_t j = 0; j < space->bvh_traversed->size; j++) {
            nvRigidBody *b = space->bvh_traversed->data[j];

            if (nvBroadPhase_early_out(space, a, b)) continue;

            nvAABB bbox = nvRigidBody_get_aabb(b);

            nvBroadPhasePair pair = {a, b};

            if (nv_collide_aabb_x_aabb(aabb, bbox)) {
                nvMemoryPool_add(space->broadphase_pairs, &pair);
            }
        }
    }
    NV_PROFILER_STOP(timer, space->profiler.bvh_traverse);

    NV_PROFILER_START(timer);
    nvBVHTree_free(bvh);
    NV_PROFILER_STOP(timer, space->profiler.bvh_free);

    NV_TRACY_ZONE_END;
}

void nv_broadphase_finalize(nvSpace *space) {
    NV_TRACY_ZONE_START;

    /*
        Keeping the removed contacts in the main iteration then actually removing
        them in another iteration is way more performant than modifying the map
        in single iteration. Resetting the iterator causes very bad performance spikes
        in large scenes.
    */

    nvHashMap_clear(space->removed_contacts);

    void *map_val;
    size_t map_iter = 0;
    while (nvHashMap_iter(space->contacts, &map_iter, &map_val)) {
        nvPersistentContactPair *pcp = map_val;

        nvRigidBody *a = pcp->body_a;
        nvRigidBody *b = pcp->body_b;
        nvAABB abox = nvRigidBody_get_aabb(a);
        nvAABB bbox = nvRigidBody_get_aabb(b);

        if (!nv_collide_aabb_x_aabb(abox, bbox)) {
            for (size_t k = 0; k < a->shapes->size; k++) {
                nvShape *shape_a = a->shapes->data[k];

                for (size_t l = 0; l < b->shapes->size; l++) {
                    nvShape *shape_b = b->shapes->data[l];

                    nvPersistentContactPair *key = &(nvPersistentContactPair){.shape_a=shape_a, .shape_b=shape_b};

                    nvPersistentContactPair *pcp = nvHashMap_get(space->contacts, key);
                    if (pcp) {
                        for (size_t c = 0; c < pcp->contact_count; c++) {
                            nvContact *contact = &pcp->contacts[c];

                            nvContactEvent event = {
                                .body_a = pcp->body_a,
                                .body_b = pcp->body_b,
                                .shape_a = pcp->shape_a,
                                .shape_b = pcp->shape_b,
                                .normal = pcp->normal,
                                .penetration = contact->separation,
                                .position = nvVector2_add(pcp->body_a->position, contact->anchor_a),
                                .normal_impulse = {contact->solver_info.normal_impulse},
                                .friction_impulse = {contact->solver_info.tangent_impulse},
                                .id = contact->id
                            };

                            if (space->listener && !contact->remove_invoked) {
                                if (space->listener->on_contact_removed)
                                    space->listener->on_contact_removed(space, event, space->listener_arg);
                                contact->remove_invoked = true;
                            };
                        }

                        nvHashMap_set(space->removed_contacts, pcp);
                    }
                }
            }
        }
    }

    // Actually remove all "removed" contacts
    map_val = NULL;
    map_iter = 0;
    while (nvHashMap_iter(space->removed_contacts, &map_iter, &map_val)) {
        nvPersistentContactPair *pcp = map_val;

        nvHashMap_remove(space->contacts, pcp);
    }

    NV_TRACY_ZONE_END;
}