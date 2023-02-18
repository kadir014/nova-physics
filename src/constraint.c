/*

  This file is a part of the Nova Physics Engine
  project and distributed under the MIT license.

  Copyright © Kadir Aksoy
  https://github.com/kadir014/nova-physics

*/

#include "novaphysics/internal.h"
#include "novaphysics/constraint.h"


/**
 * constraint.c
 * 
 * Constraints
 */


nv_Constraint *nv_SpringConstraint_new(
    nv_Body *a,
    nv_Body *b,
    nv_Vector2 anchor_a,
    nv_Vector2 anchor_b,
    double length,
    double stiffness,
    double damping
) {
    nv_Constraint *cons = NV_NEW(nv_Constraint);

    cons->a = a;
    cons->b = b;
    cons->type = nv_ConstraintType_SPRING;

    cons->head = (void *)NV_NEW(nv_Spring);
    nv_Spring *spring = (nv_Spring *)cons->head;
    
    spring->length = length;
    spring->stiffness = stiffness;
    spring->damping = damping;
    spring->anchor_a = anchor_a;
    spring->anchor_b = anchor_b;

    return cons;
}

void nv_Constraint_free(void *cons) {
    if (cons == NULL) return;
    nv_Constraint *c = (nv_Constraint *)cons;

    free(c->head);
    free(c);
}