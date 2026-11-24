/*

  This file is a part of the Nova Physics Engine
  project and distributed under the MIT license.

  Copyright © Kadir Aksoy
  https://github.com/kadir014/nova-physics

*/

#include "novaphysics/broadphase.h"
#include "novaphysics/core/refarray.h"
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
    nvBVHProxy *a,
    nvBVHProxy *b
) {
    // Same body or already checked
    if (a->body->id >= b->body->id) {
        return true;
    }

    /*
        Body ID orientation check works because body IDs are assignned when
        bodies are added to space.

        However, shape IDs are assigned whenever they are created.

        So this check might accidentally eliminate proper shape pairs.

        Removing this as commented out because I will probably need to remember
        this at some point...
    */                                                                 
    // if (a->shape->id >= b->shape->id) {
    //     return true;
    // }

    // Two static bodies do not need to interact
    if (
        a->body->type == nvRigidBodyType_STATIC &&
        b->body->type == nvRigidBodyType_STATIC
    ) {
        return true;
    }

    // Evaluate two filters
    if (!nvCollisionFilter_eval(a->body->collision_filter, b->body->collision_filter)) {
        return true;
    }

    // Bodies were ignored by a constraint 
    if (space->ignored_pairs->size > 0) {
        nvIgnoredConstraintPair search = nvIgnoredConstraintPair_make(
            a->body, b->body
        );
        if (nvHashMap_contains(space->ignored_pairs, &search)) {
            return true;
        }
    }

    return false;
}

int nv_broadphase_build_BVH(nvSpace *space) {
    // Prepare AABBs and splitting coordinates
    for (size_t i = 0; i < space->proxies->size; i++) {
        nvBVHProxy *proxy = NV_ARRAY_PTR_AT(space->proxies, i, nvBVHProxy);

        proxy->aabb = nvShape_get_aabb(proxy->shape, nvRigidBody_get_local_transform(proxy->body));

        proxy->centroid = NV_VECTOR2(
            (proxy->aabb.min_x + proxy->aabb.max_x) * 0.5f,
            (proxy->aabb.min_y + proxy->aabb.max_y) * 0.5f
        );
    }

    // Prepare children indices
    for (size_t i = 0; i < space->proxies->size; i++) {
        space->bvh_context.children[i] = i;
    }

    space->bvh_context.node_count = 0;

    // Build BVH only with at least 2 bodies
    space->bvh = NULL;
    if (space->bodies->size <= 1) return 1;

    // Build the tree top-down
    space->bvh = nvBVHTree_new(&space->bvh_context);

    return 0;
}

void nv_broadphase_BVH(nvSpace *space) {
    NV_TRACY_ZONE_START;

    nvArray_clear(space->broadphase_pairs);

    nvPrecisionTimer timer;
    NV_PROFILER_START(timer);
    if (nv_broadphase_build_BVH(space)) return;
    NV_PROFILER_STOP(timer, space->profiler.bvh_build);

    NV_PROFILER_START(timer);
    for (size_t i = 0; i < space->proxies->size; i++) {
        nvBVHProxy *a = NV_ARRAY_PTR_AT(space->proxies, i, nvBVHProxy);

        nvRefArray_clear(space->bvh_traversed, NULL);
        nvBVHNode_collide_aabb(
            &space->bvh_context, space->bvh, a->aabb, space->bvh_traversed
        );
        if (space->bvh_traversed->size == 0)
            continue;

        for (size_t j = 0; j < space->bvh_traversed->size; j++) {
            nvBVHProxy *b = space->bvh_traversed->data[j];

            if (nvBroadPhase_early_out(space, a, b)) continue;

            if (nv_collide_aabb_x_aabb(a->aabb, b->aabb)) {
                nvBroadPhasePair pair = {a, b};

                nvArray_add(space->broadphase_pairs, &pair);
            }
        }
    }
    NV_PROFILER_STOP(timer, space->profiler.bvh_traverse);

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

    void *contact_v;
    size_t contact_i = 0;
    while (nvHashMap_iter(space->contacts, &contact_i, &contact_v)) {
        nvPersistentContactPair *pcp = contact_v;

        nvRigidBody *a = pcp->body_a;
        nvRigidBody *b = pcp->body_b;

        nvTransform xform_a = nvRigidBody_get_local_transform(a);
        nvTransform xform_b = nvRigidBody_get_local_transform(b);

        nvShape *shape_a = pcp->shape_a;
        nvShape *shape_b = pcp->shape_b;

        nvAABB aabb_a = nvShape_get_aabb(shape_a, xform_a);
        nvAABB aabb_b = nvShape_get_aabb(shape_b, xform_b);
        if (nv_collide_aabb_x_aabb(aabb_a, aabb_b)) continue;

        for (size_t c = 0; c < pcp->contact_count; c++) {
            nvContact *contact = &pcp->contacts[c];

            if (space->listener && !contact->remove_invoked) {
                if (space->listener->on_contact_removed) {
                    nvVector2 normal_impulse = nvVector2_mul(
                        pcp->normal, contact->solver_info.normal_impulse
                    );
                    nvVector2 friction_impulse = nvVector2_mul(
                        nvVector2_perpr(pcp->normal), contact->solver_info.tangent_impulse
                    );

                    nvContactEvent event = {
                        .body_a = pcp->body_a,
                        .body_b = pcp->body_b,
                        .shape_a = pcp->shape_a,
                        .shape_b = pcp->shape_b,
                        .normal = pcp->normal,
                        .penetration = contact->separation,
                        .position = nvVector2_add(pcp->body_a->position, contact->anchor_a),
                        .normal_impulse = normal_impulse,
                        .friction_impulse = friction_impulse,
                        .id = contact->id
                    };

                    nvSpaceQueuedContactEvent queued_event = {
                        .callback = space->listener->on_contact_removed,
                        .event = event
                    };
                    nvArray_add(space->queued_contact_events, &queued_event);
                }
                contact->remove_invoked = true;
            }
        }

        nvHashMap_set(space->removed_contacts, pcp);
    }

    // Actually remove all "removed" contacts
    contact_v = NULL;
    contact_i = 0;
    while (nvHashMap_iter(space->removed_contacts, &contact_i, &contact_v)) {
        nvPersistentContactPair *pcp = contact_v;

        nvHashMap_remove(space->contacts, pcp);
    }

    NV_TRACY_ZONE_END;
}