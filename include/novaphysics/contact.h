/*

  This file is a part of the Nova Physics Engine
  project and distributed under the MIT license.

  Copyright © Kadir Aksoy
  https://github.com/kadir014/nova-physics

*/

#ifndef NOVAPHYSICS_CONTACT_H
#define NOVAPHYSICS_CONTACT_H

#include "novaphysics/internal.h"
#include "novaphysics/vector.h"
#include "novaphysics/shape.h"
#include "novaphysics/body.h"


/**
 * @file contact.h
 * 
 * @brief Collision and contact information.
 */


/**
 * @brief Solver related information for collision.
 */
typedef struct {
    nv_float normal_impulse; /**< Accumulated normal impulse. */
    nv_float tangent_impulse; /**< Accumulated tangent impulse. */
    nv_float mass_normal; /**< Normal effective mass. */
    nv_float mass_tangent; /**< Tangent effective mass. */
    nv_float velocity_bias; /**< Restitution bias. */
    nv_float position_bias; /**< Baumgarte position correction bias. */
    nv_float friction; /**< Friction coefficient. */
} nvContactSolverInfo;

static const nvContactSolverInfo nvContactSolverInfo_zero = {
    0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0
};


/**
 * @brief Contact point that persists across frames.
 */
typedef struct {
    nvVector2 anchor_a; /**< Location of point relative to body A position. */
    nvVector2 anchor_b; /**< Location of point relative to body B position. */
    nv_float separation; /**< Depth of the contact point in reference body. */
    nv_uint64 id; /**< Contact point feature ID. */
    nv_bool is_persisted; /**< Did this contact point persist? */
    nv_bool remove_invoked; /**< Did event listener invoke this point for removed? */
    nvContactSolverInfo solver_info; /**< Solver related information. */
} nvContact;


/**
 * @brief Collision information structure that persists across frames.
 */
typedef struct {
    nvVector2 normal; /**< Normal axis of collision. */
    nvContact contacts[2]; /**< Contact points. */
    nv_uint32 contact_count; /**< Number of contact points. */
    nvShape *shape_a; /**< First shape. */
    nvShape *shape_b; /**< Second shape. */
    nvRigidBody *body_a; /**< First body. */
    nvRigidBody *body_b; /**< Second body. */
} nvPersistentContactPair;

/**
 * @brief Is this current contact pair actually penetrating?
 * 
 * @param pcp Persistent contact pair
 * @return nv_bool 
 */
nv_bool nvPersistentContactPair_penetrating(nvPersistentContactPair *pcp);

/**
 * @brief Make a unique key from two contact shapes.
 */
static inline nv_uint64 nvPersistentContactPair_key(nvShape *a, nvShape *b) {
    /*
        Combining truncated parts of the pointers might better than using
        just the truncated low bits.
    */

    // Using IDs directly instead of hashing creates lots of collisions
    nv_uint32 fpa = nv_u32hash(a->id);
    nv_uint32 fpb = nv_u32hash(b->id);

    return nv_u32pair(fpa, fpb);
}

/**
 * @brief Persistent contact pair hashmap callback.
 */
nv_uint64 nvPersistentContactPair_hash(void *item);

/**
 * @brief Remove contact and invoke event.
 * 
 * @param space Space
 */
void nvPersistentContactPair_remove(
    struct nvSpace *space,
    nvPersistentContactPair *pcp
);


/**
 * @brief Contact event information.
 */
typedef struct {
    nvRigidBody *body_a; /**< Body A. */
    nvRigidBody *body_b; /**< Body B. */
    nvShape *shape_a; /**< Shape A. */
    nvShape *shape_b; /**< Shape B. */
    nvVector2 normal; /**< Collision normal. */
    nv_float penetration; /**< Contact point penetration depth. */
    nvVector2 position; /**< Contact point position in world space. */
    nvVector2 normal_impulse; /**< Impulse applied for non-penetration. */
    nvVector2 friction_impulse; /**< Impulse applied for friction. */
    nv_uint64 id; /**< Contact feature ID. */
} nvContactEvent;

typedef void (*nvContactListenerCallback)(struct nvSpace *space, nvContactEvent event, void *user_arg);

/**
 * @brief Contact event listener.
 */
typedef struct {
    nvContactListenerCallback on_contact_added; /**< This function is called the first frame where a contact point is detected.
                                                     Since it's not solved yet, impulse information is zeros. */
    nvContactListenerCallback on_contact_persisted; /**< This function is called every frame when a contact point persist across frames. */
    nvContactListenerCallback on_contact_removed; /**< This function is called the first frame when a contact point no longer exists. */
} nvContactListener;


#endif