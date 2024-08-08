/*

  This file is a part of the Nova Physics Engine
  project and distributed under the MIT license.

  Copyright © Kadir Aksoy
  https://github.com/kadir014/nova-physics

*/

#include "novaphysics/contact.h"
#include "novaphysics/space.h"


/**
 * @file contact.c
 * 
 * @brief Collision and contact information.
 */


nv_bool nvPersistentContactPair_penetrating(nvPersistentContactPair *pcp) {
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

nv_uint64 nvPersistentContactPair_hash(void *item) {
    nvPersistentContactPair *pcp = (nvPersistentContactPair *)item;
    return nvPersistentContactPair_key(pcp->shape_a, pcp->shape_b);
}

void nvPersistentContactPair_remove(
    nvSpace *space,
    nvPersistentContactPair *pcp
) {
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

    nvHashMap_remove(space->contacts, pcp);
}