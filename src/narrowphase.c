/*

  This file is a part of the Nova Physics Engine
  project and distributed under the MIT license.

  Copyright © Kadir Aksoy
  https://github.com/kadir014/nova-physics

*/

#include "novaphysics/internal.h"
#include "novaphysics/narrowphase.h"
#include "novaphysics/space.h"
#include "novaphysics/math.h"
#include "novaphysics/contact.h"
#include "novaphysics/collision.h"


/**
 * @file narrowphase.c
 * 
 * @brief Narrow-phase.
 */


static void generate_contact_pair(
    nvPersistentContactPair *pcp,
    nvRigidBody *body_a,
    nvRigidBody *body_b,
    nvShape *shape_a,
    nvShape *shape_b
) {
    nvTransform xform_a = {body_a->origin, body_a->angle};
    nvTransform xform_b = {body_b->origin, body_b->angle};
    pcp->contact_count = 0;

    if (shape_a->type == nvShapeType_POLYGON && shape_b->type == nvShapeType_POLYGON) {
        *pcp = nv_collide_polygon_x_polygon(
            shape_a,
            xform_a,
            shape_b,
            xform_b
        );
    }
    else if (shape_a->type == nvShapeType_CIRCLE && shape_b->type == nvShapeType_CIRCLE) {
        *pcp = nv_collide_circle_x_circle(
            shape_a,
            xform_a,
            shape_b,
            xform_b
        );
    }
    else if (shape_a->type == nvShapeType_CIRCLE && shape_b->type == nvShapeType_POLYGON) {
        *pcp = nv_collide_polygon_x_circle(
            shape_b,
            xform_b,
            shape_a,
            xform_a,
            true
        );
    }
    else if (shape_a->type == nvShapeType_POLYGON && shape_b->type == nvShapeType_CIRCLE) {
        *pcp = nv_collide_polygon_x_circle(
            shape_a,
            xform_a,
            shape_b,
            xform_b,
            false
        );
    }

    pcp->body_a = body_a;
    pcp->body_b = body_b;
    pcp->shape_a = shape_a;
    pcp->shape_b = shape_b;
}


void nv_narrow_phase(nvSpace *space) {
    NV_TRACY_ZONE_START;

    for (size_t i = 0; i < space->broadphase_pairs->size; i++) {
        nvRigidBody *body_a = ((nvBroadPhasePair *)space->broadphase_pairs->data[i])->a;
        nvRigidBody *body_b = ((nvBroadPhasePair *)space->broadphase_pairs->data[i])->b;
        nvVector2 com_a = nvVector2_rotate(body_a->com, body_a->angle);
        nvVector2 com_b = nvVector2_rotate(body_b->com, body_b->angle);

        for (size_t j = 0; j < body_a->shapes->size; j++) {
            nvShape *shape_a = body_a->shapes->data[j];

            for (size_t k = 0; k < body_b->shapes->size; k++) {
                nvShape *shape_b = body_b->shapes->data[k];

                nvPersistentContactPair *old_pcp = nvHashMap_get(space->contacts, &(nvPersistentContactPair){.shape_a=shape_a, .shape_b=shape_b});
                
                // Contact already exists, check the collision and update the contact info
                if (old_pcp) {
                    nvPersistentContactPair pcp;
                    generate_contact_pair(&pcp, body_a, body_b, shape_a, shape_b);

                    // Match contact solver info for warm-starting
                    for (size_t c = 0; c < pcp.contact_count; c++) {
                        nvContact *contact = &pcp.contacts[c];

                        // Contacts relative to center of mass
                        contact->anchor_a = nvVector2_sub(contact->anchor_a, com_a);
                        contact->anchor_b = nvVector2_sub(contact->anchor_b, com_b);

                        for (size_t old_c = 0; old_c < old_pcp->contact_count; old_c++) {
                            nvContact old_contact = old_pcp->contacts[old_c];

                            if (old_contact.id == contact->id) {
                                contact->is_persisted = true;
                                contact->remove_invoked = old_contact.remove_invoked;

                                if (space->settings.warmstarting)
                                    contact->solver_info = old_contact.solver_info;

                                if (space->listener) {
                                    nvContactEvent event = {
                                        .body_a = body_a,
                                        .body_b = body_b,
                                        .shape_a = shape_a,
                                        .shape_b = shape_b,
                                        .normal = pcp.normal,
                                        .penetration = contact->separation,
                                        .position = nvVector2_add(body_a->position, contact->anchor_a),
                                        .normal_impulse = contact->solver_info.normal_impulse,
                                        .friction_impulse = contact->solver_info.tangent_impulse,
                                        .id = contact->id
                                    };

                                    // If the contact is penetrating call persisted event callback
                                    // Else call removed callback once

                                    nvContactListenerCallback cb;
                                    if (contact->separation < 0.0) {
                                        if (space->listener->on_contact_persisted)
                                            space->listener->on_contact_persisted(event, space->listener_arg);
                                        contact->remove_invoked = false;
                                    }
                                    else if (!contact->remove_invoked) {
                                        if (space->listener->on_contact_removed)
                                            space->listener->on_contact_removed(event, space->listener_arg);
                                        contact->remove_invoked = true;
                                    };
                                }
                            }
                        }
                    }

                    // All the contacts are now removed but old pair had contacts
                    // So call removed event callbacks
                    if (pcp.contact_count == 0 && old_pcp->contact_count > 0) {
                        for (size_t old_c = 0; old_c < old_pcp->contact_count; old_c++) {
                            nvContact *contact = &old_pcp->contacts[old_c];

                            nvContactEvent event = {
                                .body_a = body_a,
                                .body_b = body_b,
                                .shape_a = shape_a,
                                .shape_b = shape_b,
                                .normal = pcp.normal,
                                .penetration = contact->separation,
                                .position = nvVector2_add(body_a->position, contact->anchor_a),
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
                    }

                    nvHashMap_set(space->contacts, &pcp);
                }

                // Contact doesn't exists, register the new contact info
                else {
                    nvPersistentContactPair pcp;
                    generate_contact_pair(&pcp, body_a, body_b, shape_a, shape_b);

                    // Register if the contacts are actually penetrating
                    // TODO: is this necessary??
                    if (nvPersistentContactPair_penetrating(&pcp)) {
                        for (size_t c = 0; c < pcp.contact_count; c++) {
                            nvContact *contact = &pcp.contacts[c];

                            // Contacts relative to center of mass
                            contact->anchor_a = nvVector2_sub(contact->anchor_a, com_a);
                            contact->anchor_b = nvVector2_sub(contact->anchor_b, com_b);

                            if (
                                space->listener &&
                                space->listener->on_contact_added &&
                                contact->separation < 0.0
                            ) {
                                nvContactEvent event = {
                                    .body_a = body_a,
                                    .body_b = body_b,
                                    .shape_a = shape_a,
                                    .shape_b = shape_b,
                                    .normal = pcp.normal,
                                    .penetration = contact->separation,
                                    .position = nvVector2_add(body_a->position, contact->anchor_a),
                                    .normal_impulse = contact->solver_info.normal_impulse,
                                    .friction_impulse = contact->solver_info.tangent_impulse,
                                    .id = contact->id
                                };
                                space->listener->on_contact_added(event, space->listener_arg);
                            }
                        }

                        nvHashMap_set(space->contacts, &pcp);
                    }
                }
            }
        }
    }

    NV_TRACY_ZONE_END;
}