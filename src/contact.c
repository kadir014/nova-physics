/*

  This file is a part of the Nova Physics Engine
  project and distributed under the MIT license.

  Copyright © Kadir Aksoy
  https://github.com/kadir014/nova-physics

*/

#include "novaphysics/contact.h"


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