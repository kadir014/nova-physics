/*

  This file is a part of the Nova Physics Engine
  project and distributed under the MIT license.

  Copyright © Kadir Aksoy
  https://github.com/kadir014/nova-physics

*/

#ifndef NOVAPHYSICS_CONSTRAINT_H
#define NOVAPHYSICS_CONSTRAINT_H

#include "novaphysics/internal.h"
#include "novaphysics/body.h"


/**
 * @file constraints/constraint.h
 * 
 * @brief Base constraint definition.
 */


/**
 * @brief Type of algorithm used to solve position error in collisions.
 * 
 * @note Changing this setting should be usually avoided unless you have
 * a specific need or familiar with the behavior.
 * 
 * In baumgarte stabilization, the position error is fed back into the velocity
 * constraint, this is an efficient solution however it adds energy to the system.
 * 
 * NGS (Non-Linear Gauss-Seidel) uses pseudo-velocities to resolve the drift.
 * It is computationally bit more expensive but more stable.
 * It is what version 2 of Box2D uses.
 * 
 * @warning Nova doesn't have NGS solver yet.
 */
typedef enum {
    nvContactPositionCorrection_BAUMGARTE, /**< Baumgarte stabilization. */
    nvContactPositionCorrection_NGS /**< Non-Linear Gauss-Seidel. */
} nvContactPositionCorrection;


/**
 * @brief Coefficient mixing type is the method to mix various coefficients
 *        values like restitution and friction.
 */
typedef enum {
    nvCoefficientMix_AVG, /**< (a + b) * 0.5 */
    nvCoefficientMix_MUL, /**< a * b */
    nvCoefficientMix_SQRT, /**< sqrt(a * b) */
    nvCoefficientMix_MIN, /**< min(a, b) */
    nvCoefficientMix_MAX /**< max(a, b) */
} nvCoefficientMix;

/**
 * @brief Mix two coefficient values.
 * 
 * @param a First value
 * @param b Second value
 * @param mix Mixing type
 * @return nv_float 
 */
static inline nv_float nv_mix_coefficients(
    nv_float a,
    nv_float b,
    nvCoefficientMix mix
) {
    switch (mix) {
        case nvCoefficientMix_AVG:
            return (a + b) * (nv_float)0.5;

        case nvCoefficientMix_MUL:
            return a * b;

        case nvCoefficientMix_SQRT:
            return nv_sqrt(a * b);

        case nvCoefficientMix_MIN:
            return nv_fmin(a, b);

        case nvCoefficientMix_MAX:
            return nv_fmax(a, b);

        default:
            // worth setting error?
            return 0.0f;
    }
}


/**
 * @brief Pair of two bodies connected with a constraint that ignores collision.
 * 
 * This pair is not created for every constraint, only for ignored collisions.
 */
typedef struct {
    nv_uint32 lower_id; /**< Rigid body with a lower ID number. */
    nv_uint32 higher_id; /**< Rigid body with a higher ID number. */
    size_t ref_count; /**< Number of collision-ignoring constraints for this pair. */
} nvIgnoredConstraintPair;

/**
 * @brief Create a new ignored constraint pair with one reference count.
 * 
 * @param a First body.
 * @param b Second body
 * @return nvIgnoredConstraintPair 
 */
static inline nvIgnoredConstraintPair nvIgnoredConstraintPair_make(
    nvRigidBody *a,
    nvRigidBody *b
) {
    // Constraints can connect to world (NULL body), so set 0 as the id.
    nv_uint32 id_a = 0;
    if (a) id_a = a->id;
    nv_uint32 id_b = 0;
    if (b) id_b = b->id;

    // Commutativity
    if (id_a > id_b) {
        nv_uint32 tmp = id_a;
        id_a = id_b;
        id_b = tmp;
    }

    return (nvIgnoredConstraintPair){
        .lower_id = id_a,
        .higher_id = id_b,
        .ref_count = 1
    };
}

/**
 * @brief Ignored constraint pair hasher.
 */
static inline nv_uint64 nvIgnoredConstraintPair_hasher(void *item) {
    nvIgnoredConstraintPair *pair = (nvIgnoredConstraintPair *)item;

    nv_uint64 combined = nv_pair_u32_to_u64(pair->lower_id, pair->higher_id);
    return nv_hash_u64_to_u64(combined);
}

/**
 * @brief Ignored constraint pair comparer.
 */
static inline nv_bool nvIgnoredConstraintPair_comparer(void *a, void *b) {
    nvIgnoredConstraintPair *pair0 = (nvIgnoredConstraintPair *)a;
    nvIgnoredConstraintPair *pair1 = (nvIgnoredConstraintPair *)b;

    return (
        pair0->lower_id == pair1->lower_id &&
        pair0->higher_id == pair1->higher_id
    );
}


/**
 * @brief Constraint types.
 * 
 * Contact constraint is not included because it's handled internally by the engine.
 */
typedef enum {
    nvConstraintType_DISTANCE, /**< Distance constraint type. See @ref nvDistanceConstraint. */
    nvConstraintType_HINGE, /**< Hinge constraint type. See @ref nvHingeConstraint. */
    nvConstraintType_SPLINE /**< Spline constraint type. See @ref nvSplineConstraint. */
} nvConstraintType;


/**
 * @brief Base two-body constraint.
 */
typedef struct {
    nvConstraintType type; /**< Type of the constraint. */
    void *def; /**< Constraint definition class. (This needs to be casted) */
    nvRigidBody *a; /**< First body. */
    nvRigidBody *b; /**< Second body. */
    nv_bool ignore_collision; /**< Ignore collision of bodies connected with this constraint. */
} nvConstraint;

/**
 * @brief Free constraint.
 * 
 * It's safe to pass `NULL` to this function.
 * 
 * @param cons Constraint
 */
void nvConstraint_free(nvConstraint *cons);

/**
 * @brief Get collision ignoring property of constraint.
 * 
 * This disables collision of two bodies that this constraint joints.
 * 
 * @param cons Constraint.
 * @return nv_bool 
 */
nv_bool nvConstraint_get_ignore_collision(const nvConstraint *cons);

/**
 * @brief Set collision ignoring proeprty of constraint.
 * 
 * This disables collision of two bodies that this constraint joints.
 * 
 * Returns non-zero on error. Use @ref nv_get_error to get more information.
 * 
 * @param cons Constraint.
 * @param ignore_collision Ignore collision between constraint bodies. 
 */
int nvConstraint_set_ignore_collision(
    nvConstraint *cons,
    nv_bool ignore_collision
);

/**
 * @brief Prepare for solving.
 * 
 * @param space Space
 * @param cons Constraint
 * @param dt Delta time
 * @param inv_dt Inverse delta time
 */
void nvConstraint_presolve(
    struct nvSpace *space,
    nvConstraint *cons,
    nv_float dt,
    nv_float inv_dt
);

/**
 * @brief Warmstart / Accumulate impulses from last frame.
 * 
 * @param space Space
 * @param cons Constraint
 */
void nvConstraint_warmstart(struct nvSpace *space, nvConstraint *cons);

/**
 * @brief Solve constraint.
 * 
 * @param cons Constraint
 * @param inv_dt Inverse delta time
 */
void nvConstraint_solve(nvConstraint *cons, nv_float inv_dt);


#endif