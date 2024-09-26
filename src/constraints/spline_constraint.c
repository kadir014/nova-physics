/*

  This file is a part of the Nova Physics Engine
  project and distributed under the MIT license.

  Copyright © Kadir Aksoy
  https://github.com/kadir014/nova-physics

*/

#include "novaphysics/constraints/spline_constraint.h"
#include "novaphysics/space.h"


/**
 * @file constraints/spline_constraint.c
 * 
 * @brief Spline constraint solver.
 */


nvConstraint *nvSplineConstraint_new(nvSplineConstraintInitializer init) {
    nvConstraint *cons = NV_NEW(nvConstraint);
    NV_MEM_CHECK(cons);

    if (!init.body) {
        nv_set_error("The body can't be NULL.");
        NV_FREE(cons);
        return NULL;
    }

    cons->a = init.body;
    cons->b = NULL;
    cons->type = nvConstraintType_SPLINE;
    cons->ignore_collision = false;

    cons->def = NV_NEW(nvSplineConstraint);
    if (!cons->def) {
        nv_set_error("Failed to allocate memory.");
        NV_FREE(cons);
        return NULL; 
    }
    nvSplineConstraint *spline_cons = (nvSplineConstraint *)cons->def;

    spline_cons->anchor_a = nvVector2_sub(init.anchor, init.body->position);
    spline_cons->anchor_b = init.anchor;
    spline_cons->max_force = init.max_force;

    spline_cons->xanchor_a = nvVector2_zero;
    spline_cons->xanchor_b = nvVector2_zero;
    spline_cons->normal = nvVector2_zero;
    spline_cons->bias = 0.0;
    spline_cons->mass = 0.0;
    spline_cons->impulse = 0.0;
    spline_cons->max_impulse = 0.0;

    return cons;
}

nvRigidBody *nvSplineConstraint_get_body(const nvConstraint *cons) {
    return cons->a;
}

void nvSplineConstraint_set_anchor(nvConstraint *cons, nvVector2 anchor) {
    nvSplineConstraint *spline_cons = (nvSplineConstraint *)cons->def;
    spline_cons->anchor = anchor;

    spline_cons->anchor_a = nvVector2_sub(spline_cons->anchor, cons->a->position);
    spline_cons->anchor_b = spline_cons->anchor;
}

nvVector2 nvSplineConstraint_get_anchor(const nvConstraint *cons) {
    nvSplineConstraint *spline_cons = (nvSplineConstraint *)cons->def;
    return spline_cons->anchor;
}

void nvSplineConstraint_set_max_force(nvConstraint *cons, nv_float max_force) {
    nvSplineConstraint *spline_cons = (nvSplineConstraint *)cons->def;
    spline_cons->max_force = max_force;
}

nv_float nvSplineConstraint_get_max_force(const nvConstraint *cons) {
    nvSplineConstraint *spline_cons = (nvSplineConstraint *)cons->def;
    return spline_cons->max_force;
}

int nvSplineConstraint_set_control_points(
    nvConstraint *cons,
    nvVector2 *points,
    size_t num_points
) {
    nvSplineConstraint *spline_cons = (nvSplineConstraint *)cons->def;

    if (num_points < 4) {
        nv_set_error("Spline path needs at least 4 control points.");
        return 1;
    }

    spline_cons->num_controls = num_points;
    for (size_t i = 0; i < num_points; i++) {
        spline_cons->controls[i] = points[i];
    }

    return 0;
}

nvVector2 *nvSplineConstraint_get_control_points(const nvConstraint *cons) {
    nvSplineConstraint *spline_cons = (nvSplineConstraint *)cons->def;
    return spline_cons->controls;
}

size_t nvSplineConstraint_get_number_of_control_points(const nvConstraint *cons) {
    nvSplineConstraint *spline_cons = (nvSplineConstraint *)cons->def;
    return spline_cons->num_controls;
}

static inline nvVector2 catmull_rom(
    nvVector2 p0,
    nvVector2 p1,
    nvVector2 p2,
    nvVector2 p3,
    double t
) {
    nv_float t2 = t * t;
    nv_float t3 = t2 * t;

    double x = 0.5 * ((2.0 * p1.x) +
               (-p0.x + p2.x) * t +
               (2.0 * p0.x - 5.0 * p1.x + 4.0 * p2.x - p3.x) * t2 +
               (-p0.x + 3.0 * p1.x - 3.0 * p2.x + p3.x) * t3);

    double y = 0.5 * ((2 * p1.y) +
               (-p0.y + p2.y) * t +
               (2.0 * p0.y - 5.0 * p1.y + 4.0 * p2.y - p3.y) * t2 +
               (-p0.y + 3.0 * p1.y - 3.0 * p2.y + p3.y) * t3);

    return NV_VECTOR2(x, y);
}

static inline double gss_for_t(
    nvVector2 p0,
    nvVector2 p1,
    nvVector2 p2,
    nvVector2 p3,
    nvVector2 p, 
    double tolerance
) {
    /*
        Perform Golden-section search to find the closest t value to desired
        point on the spline function.
        https://en.wikipedia.org/wiki/Golden-section_search
     */

    // Start t range at [0, 1] and search iteratively
    double a = 0.0;
    double b = 1.0;
    double t1 = b - (b - a) * NV_INV_PHI;
    double t2 = a + (b - a) * NV_INV_PHI;
    
    while (fabs(b - a) > tolerance) {
        nvVector2 v1 = catmull_rom(p0, p1, p2, p3, t1);
        nvVector2 v2 = catmull_rom(p0, p1, p2, p3, t2);
        
        if (nvVector2_dist2(v1, p) < nvVector2_dist2(v2, p)) {
            b = t2;
        } else {
            a = t1;
        }
        
        t1 = b - (b - a) * NV_INV_PHI;
        t2 = a + (b - a) * NV_INV_PHI;
    }
    
    return (a + b) / 2.0;
}

static nvVector2 spline_closest(
    nvSplineConstraint *spline,
    nvVector2 point
) {
    nvVector2 *controls = spline->controls;
    size_t num_controls = spline->num_controls;
    size_t num_segments = num_controls - 3;

    size_t sample_per_segment = NV_SPLINE_CONSTRAINT_SAMPLES / num_segments;

    // Segments will always be initialized in the loop
    nvVector2 segment0 = nvVector2_zero;
    nvVector2 segment1 = nvVector2_zero;
    nvVector2 segment2 = nvVector2_zero;
    nvVector2 segment3 = nvVector2_zero;
    nv_float min_dist = NV_INF;

    // Find the closest segment with sampling

    for (size_t i = 0; i < num_segments; i++) {
        for (size_t j = 0; j < sample_per_segment; j++) {
            double t = (double)j / (double)(sample_per_segment - 1);
            nvVector2 p0 = controls[i];
            nvVector2 p1 = controls[i + 1];
            nvVector2 p2 = controls[i + 2];
            nvVector2 p3 = controls[i + 3];
            nvVector2 p = catmull_rom(p0, p1, p2, p3, t);
            
            nv_float dist = nvVector2_dist2(p, point);
            if (dist < min_dist) {
                min_dist = dist;
                segment0 = p0;
                segment1 = p1;
                segment2 = p2;
                segment3 = p3;
            }
        }
    }

    // Find the closest point with golden-section search on the segment

    double t = gss_for_t(segment0, segment1, segment2, segment3, point, NV_SPLINE_CONSTRAINT_TOLERANCE);
    return catmull_rom(segment0, segment1, segment2, segment3, t);
}

void nvSplineConstraint_presolve(
    nvSpace *space,
    nvConstraint *cons,
    nv_float dt,
    nv_float inv_dt
) {
    nvSplineConstraint *spline_cons = (nvSplineConstraint *)cons->def;
    nvRigidBody *a = cons->a;

    // Transformed anchor points
    nvVector2 rpa, rpb;
    nv_float invmass_a, invmass_b, invinertia_a, invinertia_b;

    spline_cons->xanchor_a = nvVector2_rotate(spline_cons->anchor_a, a->angle);
    rpa = nvVector2_add(spline_cons->xanchor_a, a->position);
    invmass_a = a->invmass;
    invinertia_a = a->invinertia;

    nvVector2 spline_point = spline_closest(spline_cons, rpa);

    spline_cons->xanchor_b = nvVector2_zero;
    rpb = spline_point;
    invmass_b = invinertia_b = 0.0;

    // If delta is 0 point constraint is ensured
    nvVector2 delta = nvVector2_sub(rpb, rpa);
    if (nvVector2_len2(delta) == 0.0) spline_cons->normal = nvVector2_zero;
    else spline_cons->normal = nvVector2_normalize(delta);
    nv_float offset = nvVector2_len(delta);

    // Baumgarte stabilization bias
    spline_cons->bias = space->settings.baumgarte * inv_dt * offset;

    // Constraint effective mass
    spline_cons->mass = 1.0 / nv_calc_mass_k(
        spline_cons->normal,
        spline_cons->xanchor_a, spline_cons->xanchor_b,
        invmass_a, invmass_b,
        invinertia_a, invinertia_b
    );

    spline_cons->max_impulse = spline_cons->max_force * dt;
}

void nvSplineConstraint_warmstart(nvSpace *space, nvConstraint *cons) {
    nvSplineConstraint *spline_cons = (nvSplineConstraint *)cons->def;

    if (space->settings.warmstarting) {
        nvVector2 impulse = nvVector2_mul(spline_cons->normal, spline_cons->impulse);

        nvRigidBody_apply_impulse(cons->a, nvVector2_neg(impulse), spline_cons->xanchor_a);
    }
    else {
        spline_cons->impulse = 0.0;
    }
}

void nvSplineConstraint_solve(nvConstraint *cons) {
    nvSplineConstraint *spline_cons = (nvSplineConstraint *)cons->def;
    nvRigidBody *a = cons->a;

    // Skip if constraint is already ensured
    if (nvVector2_is_zero(spline_cons->normal))
        return;

    nvVector2 linear_velocity_a, linear_velocity_b;
    nv_float angular_velocity_a, angular_velocity_b;

    linear_velocity_a = a->linear_velocity;
    angular_velocity_a = a->angular_velocity;

    linear_velocity_b = nvVector2_zero;
    angular_velocity_b = 0.0;

    nvVector2 rv = nv_calc_relative_velocity(
        linear_velocity_a, angular_velocity_a, spline_cons->xanchor_a,
        linear_velocity_b, angular_velocity_b, spline_cons->xanchor_b
    );

    nv_float vn = nvVector2_dot(rv, spline_cons->normal);

    // Constraint impulse magnitude
    nv_float lambda = -(spline_cons->bias + vn) * spline_cons->mass;

    // Accumulate impulse
    nv_float limit = spline_cons->max_impulse;
    nv_float lambda0 = spline_cons->impulse;
    spline_cons->impulse = nv_fclamp(lambda0 + lambda, -limit, limit);
    lambda = spline_cons->impulse - lambda0;

    nvVector2 impulse = nvVector2_mul(spline_cons->normal, lambda);

    // Apply constraint impulse
    nvRigidBody_apply_impulse(a, nvVector2_neg(impulse), spline_cons->xanchor_a);
}