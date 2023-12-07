/*

  This file is a part of the Nova Physics Engine
  project and distributed under the MIT license.

  Copyright © Kadir Aksoy
  https://github.com/kadir014/nova-physics

*/

#include "novaphysics/internal.h"
#include "novaphysics/constraint.h"


/**
 * @file constraint.c
 * 
 * @brief Base constraint definition.
 */


void nvConstraint_free(void *cons) {
    if (cons == NULL) return;
    nvConstraint *c = (nvConstraint *)cons;

    free(c->def);
    free(c);
}