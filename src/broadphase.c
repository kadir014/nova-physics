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

    return false;
}


/**
 * @brief Matching information about rigid bodies' bph keys.
 */
typedef struct {
    nv_bool a_found;
    size_t a_index;
    nv_bool b_found;
    size_t b_index;
} nvBroadPhasePairMatch;

/**
 * @brief Search bodies' keys for matching pairs. 
 */
static inline nvBroadPhasePairMatch search_pair(
    nvBroadPhasePair *pair,
    nv_uint64 pair_key
) {
    nvBroadPhasePairMatch key = {false, 0, false, 0};

    for (size_t i = 0; i < pair->a->bph_key.size; i++) {
        if (pair->a->bph_key.pairs[i] == pair_key) {
            key.b_found = true;
            key.b_index = i;
            break;
        }
    }

    for (size_t i = 0; i < pair->b->bph_key.size; i++) {
        if (pair->b->bph_key.pairs[i] == pair_key) {
            key.a_found = true;
            key.a_index = i;
            break;
        }
    }
    
    return key;
}

/**
 * @brief Remove separated pairs from body keys.
 */
static inline void remove_pair(
    nvBroadPhasePairMatch bph_key,
    nvBroadPhasePair *pair
) {
    size_t j = 0;
    size_t k = 0;

    for (size_t i = 0; i < pair->a->bph_key.size; i++) {
        if (i == bph_key.b_index) {
            j++;
            continue;
        }
        pair->a->bph_key.pairs[k] = pair->a->bph_key.pairs[j];
        j++;
        k++;
    }

    j = 0;
    k = 0;
    for (size_t i = 0; i < pair->b->bph_key.size; i++) {
        if (i == bph_key.a_index) {
            j++;
            continue;
        }
        pair->b->bph_key.pairs[k] = pair->b->bph_key.pairs[j];
        j++;
        k++;
    }

    pair->a->bph_key.size--;
    pair->b->bph_key.size--;
}

/**
 * @brief Update body's bph pair key.
 */
static inline void append_pair_key(
    nvBroadPhasePair *pair,
    nv_uint64 pair_key
) {
    pair->a->bph_key.pairs[pair->a->bph_key.size++] = pair_key;
    pair->b->bph_key.pairs[pair->b->bph_key.size++] = pair_key;
    if (pair->a->id == 2)
        printf("a:%zu b:%zu %llu %zu %zu\n", pair->a->id, pair->b->id, pair_key, pair->a->bph_key.size, pair->b->bph_key.size);
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
            nv_uint64 pair_key = nv_u32pair(a->id, b->id);

            nvTransform xform_b = (nvTransform){b->origin, b->angle};
            nvAABB bbox = nvRigidBody_get_aabb(b);

            // First check the body AABB, then check every shape AABB
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

                nvBroadPhasePairMatch bph_key = search_pair(&pair, pair_key);

                // Both bodies do not have key, register them
                if (!bph_key.a_found && !bph_key.b_found) {
                    append_pair_key(&pair, pair_key);
                }
            }

            // AABBs are not touching, destroy any existing contact
            else {
                nv_bool pair_existed = false;

                nvBroadPhasePairMatch bph_key = search_pair(&pair, pair_key);

                // Bodies were actually colliding last frame but not now
                // So update the individual broadphase keys and destroy contact
                if (bph_key.b_found && bph_key.a_found) {
                    pair_existed = true;

                    // Update body key arrays so the removed pair is not in array anymore
                    remove_pair(bph_key, &pair);
                }

                if (pair_existed) {
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
                                        .normal_impulse = contact->solver_info.normal_impulse,
                                        .friction_impulse = contact->solver_info.tangent_impulse,
                                        .id = contact->id
                                    };

                                    if (space->listener && !contact->remove_invoked) {
                                        if (space->listener->on_contact_removed)
                                            space->listener->on_contact_removed(event, space->listener_arg);
                                        contact->remove_invoked = true;
                                    };
                                }

                                nvHashMap_remove(space->contacts, key);
                            }
                        }
                    }
                }
            }
        }
    }

    NV_TRACY_ZONE_END;
}


void nv_broadphase_BVH(nvSpace *space) {
    NV_TRACY_ZONE_START;

    nvMemoryPool_clear(space->broadphase_pairs);

    nvBVHNode *bvh = nvBVHTree_new(space->bodies);

    for (size_t i = 0; i < space->bodies->size; i++) {
        nvRigidBody *a = space->bodies->data[i];
        nvAABB aabb = nvRigidBody_get_aabb(a);

        nv_bool is_combined;
        nvArray *collided = nvBVHNode_collide(bvh, aabb, &is_combined);
        if (!collided) {
            if (is_combined) nvArray_free(collided);
            continue;
        }

        for (size_t j = 0; j < collided->size; j++) {
            nvRigidBody *b = collided->data[j];

            if (nvBroadPhase_early_out(space, a, b)) continue;

            nvAABB bbox = nvRigidBody_get_aabb(b);

            nvBroadPhasePair pair = {a, b};
            nv_uint64 pair_key = nv_u32pair(a->id, b->id);

            if (nv_collide_aabb_x_aabb(aabb, bbox)) {
                nvMemoryPool_add(space->broadphase_pairs, &pair);

                nvBroadPhasePairMatch bph_key = search_pair(&pair, pair_key);

                if (!bph_key.a_found && !bph_key.b_found) {
                    append_pair_key(&pair, pair_key);
                }
            }
        }

        if (is_combined) nvArray_free(collided);
    }

    nvBVHTree_free(bvh);

    NV_TRACY_ZONE_END;
}

void nv_broadphase_finalize(nvSpace *space) {
    NV_TRACY_ZONE_START;

    void *map_val;
    size_t l = 0;
    while (nvHashMap_iter(space->contacts, &l, &map_val)) {
        nvPersistentContactPair *pcp = map_val;

        nvRigidBody *a = pcp->body_a;
        nvRigidBody *b = pcp->body_b;
        nvAABB abox = nvRigidBody_get_aabb(a);
        nvAABB bbox = nvRigidBody_get_aabb(b);
        nvBroadPhasePair pair = {a, b};
        nv_uint64 pair_key = nv_u32pair(a->id, b->id);

        if (!nv_collide_aabb_x_aabb(abox, bbox)) {
            nv_bool pair_existed = false;

            nvBroadPhasePairMatch bph_key = search_pair(&pair, pair_key);

            // Bodies were actually colliding last frame but not now
            // So update the individual broadphase keys and destroy contact
            if (bph_key.b_found && bph_key.a_found) {
                pair_existed = true;

                // Update body key arrays so the removed pair is not in array anymore
                remove_pair(bph_key, &pair);
            }

            if (pair_existed) {
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
                                    .normal_impulse = contact->solver_info.normal_impulse,
                                    .friction_impulse = contact->solver_info.tangent_impulse,
                                    .id = contact->id
                                };

                                if (space->listener && !contact->remove_invoked) {
                                    if (space->listener->on_contact_removed)
                                        space->listener->on_contact_removed(event, space->listener_arg);
                                    contact->remove_invoked = true;
                                };
                            }

                            nvHashMap_remove(space->contacts, key);
                            l = 0;
                        }
                    }
                }
            }
        }
    }

    NV_TRACY_ZONE_END;
}