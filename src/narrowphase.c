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


void nv_narrow_phase(nvSpace *space) {
    NV_TRACY_ZONE_START;

    for (size_t i = 0; i < space->broadphase_pairs->size; i++) {
        nvRigidBody *body_a = ((nvBroadPhasePair *)space->broadphase_pairs->data[i])->a;
        nvRigidBody *body_b = ((nvBroadPhasePair *)space->broadphase_pairs->data[i])->b;

        for (size_t j = 0; j < body_a->shapes->size; j++) {
            nvShape *shape_a = body_a->shapes->data[j];

            for (size_t k = 0; k < body_b->shapes->size; k++) {
                nvShape *shape_b = body_b->shapes->data[k];

                nvPersistentContactPair *old_pcp = nvHashMap_get(space->contacts, &(nvPersistentContactPair){.shape_a=shape_a, .shape_b=shape_b});
                
                // Contact already exists, check the collision and update the contact info
                if (old_pcp) {
                    nvPersistentContactPair pcp = nv_collide_polygon_x_polygon(
                        shape_a,
                        (nvTransform){body_a->position, body_a->angle},
                        shape_b,
                        (nvTransform){body_b->position, body_b->angle}
                    );
                    pcp.body_a = body_a;
                    pcp.body_b = body_b;
                    pcp.shape_a = shape_a;
                    pcp.shape_b = shape_b;

                    // Match contact solver info for warm-starting
                    for (size_t c = 0; c < pcp.contact_count; c++) {
                        nvContact *contact = &pcp.contacts[c];

                        for (size_t old_c = 0; old_c < old_pcp->contact_count; old_c++) {
                            nvContact old_contact = old_pcp->contacts[old_c];

                            if (old_contact.id == contact->id) {
                                contact->is_persisted = true;
                                contact->solver_info = old_contact.solver_info;
                            }
                        }
                    }

                    nvHashMap_set(space->contacts, &pcp);
                }

                // Contact doesn't exists, register the new contact info
                else {
                    nvPersistentContactPair pcp = nv_collide_polygon_x_polygon(
                        shape_a,
                        (nvTransform){body_a->position, body_a->angle},
                        shape_b,
                        (nvTransform){body_b->position, body_b->angle}
                    );
                    pcp.body_a = body_a;
                    pcp.body_b = body_b;
                    pcp.shape_a = shape_a;
                    pcp.shape_b = shape_b;

                    if (nvPersistentContactPair_penetrating(&pcp))
                        nvHashMap_set(space->contacts, &pcp);
                }
            }
        }
    }

    NV_TRACY_ZONE_END;
}