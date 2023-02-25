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


void nv_Constraint_free(void *cons) {
    if (cons == NULL) return;
    nv_Constraint *c = (nv_Constraint *)cons;

    free(c->head);
    free(c);
}


nv_Constraint *nv_Spring_new(
    nv_Body *a,
    nv_Body *b,
    nv_Vector2 anchor_a,
    nv_Vector2 anchor_b,
    nv_float length,
    nv_float stiffness,
    nv_float damping
) {
    nv_Constraint *cons = NV_NEW(nv_Constraint);

    cons->a = a;
    cons->b = b;
    cons->jc = 0.0;
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


nv_Constraint *nv_DistanceJoint_new(
    nv_Body *a,
    nv_Body *b,
    nv_Vector2 anchor_a,
    nv_Vector2 anchor_b,
    nv_float length
) {
    nv_Constraint *cons = NV_NEW(nv_Constraint);

    cons->a = a;
    cons->b = b;
    cons->jc = 0.0;
    cons->type = nv_ConstraintType_DISTANCEJOINT;

    cons->head = (void *)NV_NEW(nv_DistanceJoint);
    nv_DistanceJoint *dist_joint = (nv_DistanceJoint *)cons->head;

    dist_joint->length = length;
    dist_joint->anchor_a = anchor_a;
    dist_joint->anchor_b = anchor_b;

    return cons;
}