/*

  This file is a part of the Nova Physics Engine
  project and distributed under the MIT license.

  Copyright © Kadir Aksoy
  https://github.com/kadir014/nova-physics

*/

#include "novaphysics/internal.h"
#include "novaphysics/constraints/constraint.h"
#include "novaphysics/space.h"
#include "novaphysics/constraints/distance_constraint.h"
#include "novaphysics/constraints/hinge_constraint.h"
#include "novaphysics/constraints/spline_constraint.h"


/**
 * @file constraints/constraint.c
 * 
 * @brief Base constraint definition.
 */


void nvConstraint_free(nvConstraint *cons) {
    if (!cons) return;

    NV_FREE(cons->def);
    NV_FREE(cons);
}

nv_bool nvConstraint_get_ignore_collision(const nvConstraint *cons) {
    return cons->ignore_collision;
}

int nvConstraint_set_ignore_collision(
    nvConstraint *cons,
    nv_bool ignore_collision
) {
    if (cons->ignore_collision == ignore_collision) return 0;

    nvSpace *space;
    if (cons->a) {
        space = cons->a->space;
    }
    else {
        space = cons->b->space;
    }

    // Create or increment reference
    if (cons->ignore_collision == false && ignore_collision == true) {
        nvIgnoredConstraintPair search = nvIgnoredConstraintPair_make(cons->a, cons->b);
        nvIgnoredConstraintPair *pair = nvHashMap_get(space->ignored_pairs, &search);

        if (pair) {
            pair->ref_count++;
        }
        else {
            if (!nvHashMap_set(space->ignored_pairs, &search)) {
                return 1;
            }
        }
    }
    // Remove or decrement reference
    else {
        nvIgnoredConstraintPair search = nvIgnoredConstraintPair_make(cons->a, cons->b);
        nvIgnoredConstraintPair *pair = nvHashMap_get(space->ignored_pairs, &search);

        // The pair somehow doesn't exist.
        // This should NEVER happen, but in case it does, raise error even if it doesn't affect anything.
        if (!pair) return 1;

        if (pair->ref_count <= 1) {
            if (!nvHashMap_remove(space->ignored_pairs, &search)) {
                return 2;
            }
        }
        else {
            pair->ref_count--;
        }
    }

    return 0;
}

void nvConstraint_presolve(
    nvSpace *space,
    nvConstraint *cons,
    nv_float dt,
    nv_float inv_dt
) {
    switch (cons->type) {
        case nvConstraintType_DISTANCE:
            nvDistanceConstraint_presolve(space, cons, dt, inv_dt);
            break;

        case nvConstraintType_HINGE:
            nvHingeConstraint_presolve(space, cons, dt, inv_dt);
            break;

        case nvConstraintType_SPLINE:
            nvSplineConstraint_presolve(space, cons, dt, inv_dt);
            break;
    }
}

void nvConstraint_warmstart(nvSpace *space, nvConstraint *cons) {
    switch (cons->type) {
        case nvConstraintType_DISTANCE:
            nvDistanceConstraint_warmstart(space, cons);
            break;

        case nvConstraintType_HINGE:
            nvHingeConstraint_warmstart(space, cons);
            break;

        case nvConstraintType_SPLINE:
            nvSplineConstraint_warmstart(space, cons);
            break;
    }
}

void nvConstraint_solve(nvConstraint *cons, nv_float inv_dt) {
    switch (cons->type) {
        case nvConstraintType_DISTANCE:
            nvDistanceConstraint_solve(cons);
            break;

        case nvConstraintType_HINGE:
            nvHingeConstraint_solve(cons, inv_dt);
            break;

        case nvConstraintType_SPLINE:
            nvSplineConstraint_solve(cons);
            break;
    }
}