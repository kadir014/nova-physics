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
    nv_uint64 id; /**< Unique identifier of contact point. */
    nv_bool is_persisted; /**< Did this contact point persist? */
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
static nv_bool nvPersistentContactPair_penetrating(nvPersistentContactPair *pcp) {
    nv_bool penetrating = false;

    for (size_t c = 0; c < pcp->contact_count; c++) {
        nvContact contact = pcp->contacts[c];
       
        if (contact.separation < 0.0) {
            penetrating = true;
            break;
       }
    }

    return penetrating;
}

/**
 * @brief Make a unique key from two contact shapes.
 */
static inline nv_uint64 nvPersistentContactPair_key(nvShape *a, nvShape *b) {
    /*
        Combining truncated parts of the pointers might better than using
        just the truncated low bits.
    */

    // uintptr_t pa = (uintptr_t)a;
    // uintptr_t pb = (uintptr_t)b;

    // nv_uint32 pa_high = (nv_uint32)(pa >> 32);
    // nv_uint32 pa_low = (nv_uint32)pa;
    // nv_uint32 fpa = pa_high ^ pa_low;

    // nv_uint32 pb_high = (nv_uint32)(pb >> 32);
    // nv_uint32 pb_low = (nv_uint32)pb;
    // nv_uint32 fpb = pb_high ^ pb_low;
    nv_uint32 fpa = nv_u32hash(a->id);
    nv_uint32 fpb = nv_u32hash(b->id);

    if (fpa < fpb)
        return nv_u32pair(fpa, fpb);
    else
        return nv_u32pair(fpb, fpa);
}

/**
 * @brief Persistent contact pair hashmap callback.
 */
static nv_uint64 nvPersistentContactPair_hash(void *item) {
    nvPersistentContactPair *pcp = (nvPersistentContactPair *)item;
    return nvPersistentContactPair_key(pcp->shape_a, pcp->shape_b);
}


#endif