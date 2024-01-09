/*

  This file is a part of the Nova Physics Engine
  project and distributed under the MIT license.

  Copyright © Kadir Aksoy
  https://github.com/kadir014/nova-physics

*/

#include "novaphysics/internal.h"
#include "novaphysics/constraint.h"
#include "novaphysics/space.h"
#include "novaphysics/spring.h"
#include "novaphysics/distance_joint.h"
#include "novaphysics/hinge_joint.h"


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

void nvConstraint_presolve(
    nvSpace *space,
    nvConstraint *cons,
    nv_float inv_dt
) {
    switch (cons->type) {
        case nvConstraintType_SPRING:
            nvSpring_presolve(space, cons, inv_dt);
            break;

        case nvConstraintType_DISTANCEJOINT:
            nvDistanceJoint_presolve(space, cons, inv_dt);
            break;

        case nvConstraintType_HINGEJOINT:
            nvHingeJoint_presolve(space, cons, inv_dt);
            break;
    }
}


void nvConstraint_solve(nvConstraint *cons, nv_float inv_dt) {
    switch (cons->type) {

        case nvConstraintType_SPRING:
            nvSpring_solve(cons);
            break;

        case nvConstraintType_DISTANCEJOINT:
            nvDistanceJoint_solve(cons);
            break;

        case nvConstraintType_HINGEJOINT:
            nvHingeJoint_solve(cons, inv_dt);
            break;
    }
}