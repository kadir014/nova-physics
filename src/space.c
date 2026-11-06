/*

  This file is a part of the Nova Physics Engine
  project and distributed under the MIT license.

  Copyright © Kadir Aksoy
  https://github.com/kadir014/nova-physics

*/

#include "novaphysics/space.h"
#include "novaphysics/constants.h"
#include "novaphysics/body.h"
#include "novaphysics/collision.h"
#include "novaphysics/contact.h"
#include "novaphysics/math.h"
#include "novaphysics/narrowphase.h"
#include "novaphysics/core/array.h"
#include "novaphysics/constraints/distance_constraint.h"
#include "novaphysics/constraints/hinge_constraint.h"
#include "novaphysics/constraints/spline_constraint.h"


/**
 * @file space.c
 * 
 * @brief Space struct and its methods.
 */


#define ITER_BODIES(iter) for (size_t iter = 0; iter < space->bodies->size; iter++)


nvSpace *nvSpace_new() {
    nvSpace *space = NV_NEW(nvSpace);
    NV_MEM_CHECK(space);

    space->bodies = nvRefArray_new();
    if (!nvRefArray_valid(space->bodies)) return NULL;

    space->constraints = nvRefArray_new();
    if (!nvRefArray_valid(space->constraints)) return NULL;

    space->proxies = nvArray_new(sizeof(nvBVHProxy));
    if (!nvArray_valid(space->proxies)) return NULL;

    space->ignored_pairs = nvHashMap_new(
        sizeof(nvIgnoredConstraintPair),
        nvIgnoredConstraintPair_hasher,
        nvIgnoredConstraintPair_comparer
    );
    if (!nvHashMap_valid(space->ignored_pairs)) {
        return NULL;
    }

    nvSpace_set_gravity(space, NV_VECTOR2(0.0f, 9.81f));

    space->settings = (nvSpaceSettings){
        .baumgarte = 0.2f,
        .penetration_slop = 0.05f,
        .contact_position_correction = nvContactPositionCorrection_BAUMGARTE,
        .velocity_iterations = 8,
        .position_iterations = 4,
        .substeps = 1,
        .linear_damping = 0.0005f,
        .angular_damping = 0.0005f,
        .warmstarting = true,
        .restitution_mix = nvCoefficientMix_SQRT,
        .friction_mix = nvCoefficientMix_SQRT
    };

    space->broadphase_pairs = nvArray_new_ex(
        sizeof(nvBroadPhasePair), NV_BPH_POOL_INITIAL_SIZE, 2.0f
    );
    if (!nvArray_valid(space->broadphase_pairs)) return NULL;

    space->contacts = nvHashMap_new(
        sizeof(nvPersistentContactPair),
        nvPersistentContactPair_hasher,
        nvPersistentContactPair_comparer
    );
    space->removed_contacts = nvHashMap_new(
        sizeof(nvPersistentContactPair),
        nvPersistentContactPair_hasher,
        nvPersistentContactPair_comparer
    );
    if (
        !nvHashMap_valid(space->contacts) ||
        !nvHashMap_valid(space->removed_contacts)
    ) {
        return NULL;
    }

    space->bvh = NULL;
    space->bvh_traversed = nvRefArray_new();
    if (!nvRefArray_valid(space->bvh_traversed)) return NULL;
    space->bvh_context = (nvBVHContext){
        .nodes = NV_MALLOC(sizeof(nvBVHNode) * NV_BVH_NODES_INITIAL_SIZE),
        .node_max = NV_BVH_NODES_INITIAL_SIZE,
        .node_count = 0,
        .children = NV_MALLOC(sizeof(size_t) * 1),
        .proxies = space->proxies
    };
    NV_MEM_CHECK(space->bvh_context.nodes);
    NV_MEM_CHECK(space->bvh_context.children);

    space->listener = NULL;
    space->listener_arg = NULL;

    space->visitor_poly_cb = NULL;
    space->visitor_circle_cb = NULL;

    nvProfiler_reset(&space->profiler);

    space->queued_contact_events = nvArray_new(sizeof(nvSpaceQueuedContactEvent));
    if (!nvArray_valid(space->queued_contact_events)) {
        return NULL;
    }

    // ID 0 is reserved for NULL
    space->id_counter = 1;

    return space;
}

void nvSpace_free(nvSpace *space) {
    if (!space) return;

    nvSpace_clear(space, true, false);

    nvRefArray_free(space->bodies);
    nvRefArray_free(space->constraints);

    nvArray_free(space->proxies);

    nvHashMap_free(space->ignored_pairs);

    nvArray_free(space->broadphase_pairs);

    nvHashMap_free(space->contacts);
    nvHashMap_free(space->removed_contacts);

    nvRefArray_free(space->bvh_traversed);
    NV_FREE(space->bvh_context.nodes);
    NV_FREE(space->bvh_context.children);
    
    if (space->listener)
        NV_FREE(space->listener);

    nvArray_free(space->queued_contact_events);

    NV_FREE(space);
}

void nvSpace_set_gravity(nvSpace *space, nvVector2 gravity) {
    space->gravity = gravity;
}

nvVector2 nvSpace_get_gravity(const nvSpace *space) {
    return space->gravity;
}

nvSpaceSettings *nvSpace_get_settings(nvSpace *space) {
    return &space->settings;
}

nvProfiler nvSpace_get_profiler(const nvSpace *space) {
    return space->profiler;
}

int nvSpace_set_contact_listener(
    nvSpace *space,
    nvContactListener listener,
    void *user_arg
) {
    space->listener = NV_NEW(nvContactListener);
    NV_MEM_CHECKI(space->listener);
    *space->listener = listener;
    space->listener_arg = user_arg;
    return 0;
}

nvContactListener *nvSpace_get_contact_listener(const nvSpace *space) {
    return space->listener;
}

void nvSpace_set_geometry_visitor_callbacks(
    nvSpace *space,
    nvSpacePolygonVisitorCallback polygon_visitor_callback,
    nvSpaceCircleVisitorCallback circle_visitor_callback
) {
    space->visitor_poly_cb = polygon_visitor_callback;
    space->visitor_circle_cb = circle_visitor_callback;
}

void nvSpace_visit_geometry(nvSpace *space, void *user_arg) {
    if (!space->visitor_circle_cb && !space->visitor_poly_cb) return;

    nvRigidBody *body;
    size_t body_iter = 0;

    while (nvSpace_iter_bodies(space, &body, &body_iter)) {
        nvShape *shape;
        size_t shape_iter = 0;

        while (nvRigidBody_iter_shapes(body, &shape, &shape_iter)) {
            nvTransform xform = nvRigidBody_get_local_transform(body);

            if (shape->type == nvShapeType_POLYGON && space->visitor_poly_cb) {
                nvPolygon_transform(shape, xform);

                space->visitor_poly_cb(
                    shape->polygon.xvertices,
                    shape->polygon.num_vertices,
                    space,
                    body,
                    shape,
                    user_arg
                );
            }
            else if (space->visitor_circle_cb) {
                nvVector2 center = shape->circle.center;
                center = nvVector2_add(nvVector2_rotate(center, xform.angle), xform.position);

                space->visitor_circle_cb(
                    center,
                    shape->circle.radius,
                    space,
                    body,
                    shape,
                    user_arg
                );
            }
        }
    }
    
}

int nvSpace_clear(nvSpace *space, nv_bool free_all, nv_bool shrink) {
    if (free_all) {
        nvRefArray_clear(space->bodies, (void (*)(void *))nvRigidBody_free);
        nvRefArray_clear(space->constraints, (void (*)(void *))nvConstraint_free);
    }
    else {
        nvRefArray_clear(space->bodies, NULL);
        nvRefArray_clear(space->constraints, NULL);
    }

    nvHashMap_clear(space->ignored_pairs);
    nvArray_clear(space->broadphase_pairs);
    nvHashMap_clear(space->contacts);

    NV_FREE(space->bvh_context.children);
    space->bvh_context.children = NV_MALLOC(sizeof(size_t) * 1);
    NV_MEM_CHECKI(space->bvh_context.children);

    space->bvh_context.node_count = 0;
    space->bvh = NULL;

    nvArray_clear(space->proxies);

    if (shrink) {
        nvHashMap_free(space->contacts);
        space->contacts = nvHashMap_new(
            sizeof(nvPersistentContactPair),
            nvPersistentContactPair_hasher,
            nvPersistentContactPair_comparer
        );
        if (!nvHashMap_valid(space->contacts)) {
            return 1;
        }

        nvHashMap_free(space->ignored_pairs);
        space->ignored_pairs = nvHashMap_new(
            sizeof(nvIgnoredConstraintPair),
            nvIgnoredConstraintPair_hasher,
            nvIgnoredConstraintPair_comparer
        );
        if (!nvHashMap_valid(space->ignored_pairs)) {
            return 1;
        }
    }

    return 0;
}

int nvSpace_resize_bvh(nvSpace *space) {
    space->bvh_context.children = NV_REALLOC(
        space->bvh_context.children,
        sizeof(size_t) * space->proxies->size
    );
    NV_MEM_CHECKI(space->bvh_context.children);

    return 0;
}

static int nvSpace_add_proxies(nvSpace *space, nvRigidBody *body) {
    if (body->shapes->size == 0) return 0;

    nvShape *shape = NULL;
    size_t shape_i = 0;
    while (nvRigidBody_iter_shapes(body, &shape, &shape_i)) {
        nvBVHProxy proxy = {
            .aabb = nvShape_get_aabb(shape, nvRigidBody_get_local_transform(body)),
            .centroid = nvVector2_zero,
            .body = body,
            .shape = shape
        };

        if (nvArray_add(space->proxies, &proxy))
            return 1;

        // Reset caches in case the body was added in an event or mid-step.
        shape->aabb_cache = false;
        shape->xform_cache = false;
    }

    return 0;
}

int nvSpace_add_rigidbody(nvSpace *space, nvRigidBody *body) {
    if (body->space == space) {
        nv_set_error("Can't add same body to same space more than once.");
        return 2;
    }

    if (nvRefArray_add(space->bodies, body))
        return 1;

    if (nvSpace_add_proxies(space, body))
        return 1;

    if (nvSpace_resize_bvh(space))
        return 1;

    body->space = space;
    body->id = space->id_counter++;

    return 0;
}

static int nvSpace_remove_proxies(nvSpace *space, nvRigidBody *body) {
    for (size_t i = space->proxies->size - 1; i > 0; i--) {
        nvBVHProxy *proxy = NV_ARRAY_PTR_AT(space->proxies, i, nvBVHProxy);

        if (proxy->body->id == body->id) {
            if (!nvArray_pop(space->proxies, i))
                return 1;
        }
    }

    return 0;
}

int nvSpace_remove_rigidbody(
    nvSpace *space,
    nvRigidBody *body,
    nv_bool free_resources
) {
    if (nvRefArray_remove(space->bodies, body) == NV_INVALID_INDEX_Z) return 1;

    // Remove broadphase pairs
    // TODO: Since contact events are called after broadphase & narrowphase, this
    //       doesn't change anything, maybe i can remove this?
    for (size_t i = 0; i < space->broadphase_pairs->size; i++) {
        nvBroadPhasePair *pair = NV_ARRAY_PTR_AT(space->broadphase_pairs, i, nvBroadPhasePair);

        nvBVHProxy *proxy_a = pair->a;
        nvBVHProxy *proxy_b = pair->b;

        nvRigidBody *body_a = proxy_a->body;
        nvRigidBody *body_b = proxy_b->body;

        if (!body_a || !body_b) continue;

        // TODO: Instead of setting to NULL, remove pair from candidates array
        if (body_a->id == body->id || body_b->id == body->id) {
            pair->a = NULL;
            pair->b = NULL;
        }
    }

    // Remove proxies after cleaning boradphase pairs, because they refer to proxies
    if (nvSpace_remove_proxies(space, body))
        return 1;

    // Remove contact events
    nvArray *removed_events_idx = nvArray_new(sizeof(size_t));
    for (size_t i = 0; i < space->queued_contact_events->size; i++) {
        nvSpaceQueuedContactEvent *event = NV_ARRAY_PTR_AT(
            space->queued_contact_events,
            i,
            nvSpaceQueuedContactEvent
        );

        if (
            event->event.body_a->id == body->id ||
            event->event.body_b->id == body->id
        ) {
            nvArray_add(removed_events_idx, &i);
        }
    }
    for (size_t i = 0; i < removed_events_idx->size; i++) {
        size_t idx = NV_ARRAY_AT(removed_events_idx, i, size_t);
        nvArray_pop(space->queued_contact_events, idx);
    }
    nvArray_free(removed_events_idx);

    // Remove contacts
    void *map_val;
    size_t map_iter = 0;
    while (nvHashMap_iter(space->contacts, &map_iter, &map_val)) {
        nvPersistentContactPair *pcp = map_val;

        if (pcp->body_a->id == body->id || pcp->body_b->id == body->id) {
            nvPersistentContactPair_remove(space, pcp);
            map_iter = 0;
            continue;
        }
    }

    // Remove constraints
    nvRefArray *removed_constraints = nvRefArray_new();
    for (size_t i = 0; i < space->constraints->size; i++) {
        nvConstraint *cons = space->constraints->data[i];

        nv_uint32 id_a = nvRigidBody_get_id(cons->a);
        nv_uint32 id_b = nvRigidBody_get_id(cons->b);
        if (id_a == body->id || id_b == body->id) {
            nvRefArray_add(removed_constraints, cons);
        }

        if (cons->ignore_collision) {
            nvIgnoredConstraintPair search = nvIgnoredConstraintPair_make(cons->a, cons->b);
            nvIgnoredConstraintPair *pair = nvHashMap_get(space->ignored_pairs, &search);

            if (!pair) return 2;

            if (pair->ref_count <= 1) {
                if (!nvHashMap_remove(space->ignored_pairs, &search)) {
                    return 3;
                }
            }
            else {
                pair->ref_count--;
            }
        }
    }
    for (size_t i = 0; i < removed_constraints->size; i++) {
        nvRefArray_remove(space->constraints, removed_constraints->data[i]);

        if (free_resources) {
            nvConstraint_free(removed_constraints->data[i]);
        }
    }

    nvRefArray_free(removed_constraints);

    body->space = NULL;

    if (free_resources) {
        nvRigidBody_free(body);
    }

    if (nvSpace_resize_bvh(space))
        return 1;

    return 0;
}

int nvSpace_add_constraint(nvSpace *space, nvConstraint *cons) {
    // TODO: This is inefficient
    for (size_t i = 0; i < space->constraints->size; i++) {
        nvConstraint *lcons = space->constraints->data[i];

        if (lcons == cons) {
            nv_set_error("Can't add same constraint to same space more than once.");
            return 2;
        }
    }

    if (cons->ignore_collision) {
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

    return nvRefArray_add(space->constraints, cons);
}

int nvSpace_remove_constraint(nvSpace *space, nvConstraint *cons) {
    if (nvRefArray_remove(space->constraints, cons) == NV_INVALID_INDEX_Z) {
        return 1;
    }

    if (cons->ignore_collision) {
        nvIgnoredConstraintPair search = nvIgnoredConstraintPair_make(cons->a, cons->b);
        nvIgnoredConstraintPair *pair = nvHashMap_get(space->ignored_pairs, &search);

        if (!pair) return 2;

        if (pair->ref_count <= 1) {
            if (!nvHashMap_remove(space->ignored_pairs, &search)) {
                return 3;
            }
        }
        else {
            pair->ref_count--;
        }
    }

    return 0;
}

nv_bool nvSpace_iter_bodies(nvSpace *space, nvRigidBody **body, size_t *index) {
    *body = space->bodies->data[(*index)++];
    return (*index <= space->bodies->size);
}

nv_bool nvSpace_iter_constraints(nvSpace *space, nvConstraint **cons, size_t *index) {
    *cons = space->constraints->data[(*index)++];
    return (*index <= space->constraints->size);
}

void nvSpace_step(nvSpace *space, nv_float dt) {
    if (dt <= 0.0 || space->settings.substeps <= 0) return;
    nv_uint32 substeps = space->settings.substeps;
    nv_uint32 velocity_iters = space->settings.velocity_iterations;

    /*
        Simulation route
        ----------------
        1. Integrate accelerations
        2. Broadphase
        3. Narrowphase
        4. Solve constraints (PGS + Baumgarte)
        5. Integrate velocities
        6. Contact position correction (NGS)
    */

    NV_TRACY_ZONE_START;

    nvPrecisionTimer step_timer, timer;
    NV_PROFILER_START(step_timer);

    // Reset timings out simulation step
    space->profiler.casts = 0.0;
    space->profiler.queries = 0.0;

    // For iterating contacts hashmap
    size_t contact_i = 0;
    void *contact_v = NULL;

    dt /= (nv_float)substeps;
    nv_float inv_dt = 1.0f / dt;

    for (nv_uint32 substep = 0; substep < substeps; substep++) {
        nvArray_clear(space->queued_contact_events);

        /*
            Integrate accelerations
            -----------------------
            Apply forces, gravity, integrate accelerations (update velocities) and apply damping.
            We do this step first to reset body caches.
        */
        NV_PROFILER_START(timer);
        ITER_BODIES(body_i) {
            nvRigidBody *body = (nvRigidBody *)space->bodies->data[body_i];

            nvRigidBody_integrate_accelerations(body, space->gravity, dt);
        }
        NV_PROFILER_STOP(timer, space->profiler.integrate_accelerations);

        /*
            Broadphase
            ----------
            Generate possible collision pairs with the choosen broadphase algorithm.
        */
        NV_PROFILER_START(timer);
        nv_broadphase_BVH(space);
        NV_PROFILER_STOP(timer, space->profiler.broadphase);

        NV_PROFILER_START(timer);
        nv_broadphase_finalize(space);
        NV_PROFILER_STOP(timer, space->profiler.broadphase_finalize);

        /*
            Narrowphase
            ------------
            Do narrow-phase checks between possible collision pairs and
            create & update contact pairs.
        */
        NV_PROFILER_START(timer);
        nv_narrow_phase(space);
        NV_PROFILER_STOP(timer, space->profiler.narrowphase);

        // Call contact callbacks before solving & integrating
        for (size_t event_i = 0; event_i < space->queued_contact_events->size; event_i++) {
            nvSpaceQueuedContactEvent event = NV_ARRAY_AT(
                space->queued_contact_events,
                event_i,
                nvSpaceQueuedContactEvent
            );

            event.callback(space, event.event, space->listener_arg);
        }

        /*
            Solve constraints (PGS + Baumgarte)
            -----------------------------------
            Prepare velocity constraints, warm-start and solve iteratively.
            Use baumgarte depending on the position correction setting.

            Sequential Impulses / PGS + Baumgarte:
            https://box2d.org/files/ErinCatto_SequentialImpulses_GDC2006.pdf
        */

        // Prepare constraints for solving
        NV_PROFILER_START(timer);
        for (size_t i = 0; i < space->constraints->size; i++) {
            nvConstraint_presolve(
                space,
                (nvConstraint *)space->constraints->data[i],
                dt,
                inv_dt
            );
        }

        contact_i = 0;
        while (nvHashMap_iter(space->contacts, &contact_i, &contact_v)) {
            nvPersistentContactPair *pcp = contact_v;
            nv_contact_presolve(space, pcp, inv_dt);
        }
        NV_PROFILER_STOP(timer, space->profiler.presolve);

        // Warmstart constraints
        NV_PROFILER_START(timer);
        for (size_t i = 0; i < space->constraints->size; i++) {
            nvConstraint_warmstart(
                space,
                (nvConstraint *)space->constraints->data[i]
            );
        }

        contact_i = 0;
        while (nvHashMap_iter(space->contacts, &contact_i, &contact_v)) {
            nvPersistentContactPair *pcp = contact_v;
            nv_contact_warmstart(space, pcp);
        }
        NV_PROFILER_STOP(timer, space->profiler.warmstart);

        // Solve constraints iteratively
        NV_PROFILER_START(timer);
        for (size_t i = 0; i < velocity_iters; i++) {
            for (size_t j = 0; j < space->constraints->size; j++) {
                nvConstraint_solve(
                    (nvConstraint *)space->constraints->data[j],
                    inv_dt
                );
            }

            contact_i = 0;
            while (nvHashMap_iter(space->contacts, &contact_i, &contact_v)) {
                nvPersistentContactPair *pcp = contact_v;
                nv_contact_solve_velocity(pcp);
            }
        }
        NV_PROFILER_STOP(timer, space->profiler.solve_velocities);

        /*
            Integrate velocities
            --------------------
            Integrate velocities (update positions) and check out-of-bound bodies.
        */
        NV_PROFILER_START(timer);
        ITER_BODIES(body_i) {
            nvRigidBody *body = (nvRigidBody *)space->bodies->data[body_i];

            nvRigidBody_integrate_velocities(body, dt);

            body->origin = nvVector2_sub(body->position, nvVector2_rotate(body->com, body->angle));

            // Reset caches
            if (body->type != nvRigidBodyType_STATIC) {
                body->cache_aabb = false;

                nvShape *shape = NULL;
                size_t shape_i = 0;
                while (nvRigidBody_iter_shapes(body, &shape, &shape_i)) {
                    shape->xform_cache = false;
                    shape->aabb_cache = false;
                }
            }
        }
        NV_PROFILER_STOP(timer, space->profiler.integrate_velocities);
    }
    
    NV_PROFILER_STOP(step_timer, space->profiler.step);

    NV_TRACY_ZONE_END;
    NV_TRACY_FRAMEMARK;
}

nvArray *nvSpace_cast_ray(
    nvSpace *space,
    nvVector2 from,
    nvVector2 to,
    size_t capacity,
    nvCollisionFilter filter
) {
    #ifdef NV_ENABLE_PROFILER

        nvPrecisionTimer timer;
        nvPrecisionTimer_start(&timer);

    #endif

    nvArray *final = nvArray_new_ex(sizeof(nvRayCastResult), capacity, 2.0f);
    nvRefArray *collided = nvRefArray_new();
    if (!nvArray_valid(final) || !nvRefArray_valid(collided)) {
        goto exit;
    }

    if (space->bodies->size == 0) {
        goto exit;
    }

    nvVector2 delta = nvVector2_sub(to, from);
    nvVector2 dir = nvVector2_normalize(delta);
    nvVector2 inv_dir = NV_VECTOR2(1.0f / dir.x, 1.0f / dir.y);
    nv_float maxsq = nvVector2_len2(delta);

    if (space->bodies->size > 1) {
        nvBVHNode_collide_ray(&space->bvh_context, space->bvh, from, inv_dir, collided);
        if (collided->size == 0) {
            goto exit;
        }
    }
    else {
        nvRefArray_add(collided, NV_ARRAY_PTR_AT(space->proxies, 0, nvBVHProxy));
    }

    for (size_t i = 0; i < collided->size; i++) {
        nvBVHProxy *proxy = collided->data[i];

        nvRigidBody *body = proxy->body;
        nvShape *shape = proxy->shape;

        // Do not use proxy.aabb in cast and query functions because they are decoupled from step()

        nvTransform xform = nvRigidBody_get_local_transform(body);
        nvAABB aabb = nvShape_get_aabb(shape, xform);

        if (!nv_collide_aabb_x_ray(aabb, from, inv_dir))
            continue;

        nv_float any_hit = false;

        nvRayCastResult result = {0};
        nv_bool hit = false;

        switch (shape->type) {
            case nvShapeType_CIRCLE:
                hit = nv_collide_ray_x_circle(&result, from, dir, maxsq, shape, xform);
                break;

            case nvShapeType_POLYGON:
                hit = nv_collide_ray_x_polygon(&result, from, dir, maxsq, shape, xform);
                break;
        }

        if (hit) {
            nv_bool filter_eval = nvCollisionFilter_eval(filter, body->collision_filter);

            if (filter_eval) {
                any_hit = true;
            }
        }

        if (any_hit) {
            if (final->size >= capacity) break;

            result.body = body;
            nvArray_add(final, &result);
        }
    }

    exit:

    nvRefArray_free(collided);

    #ifdef NV_ENABLE_PROFILER

        space->profiler.casts += nvPrecisionTimer_stop(&timer);

    #endif

    return final;
}

nvRefArray *nvSpace_query_point(
    nvSpace *space,
    nvVector2 point,
    nvCollisionFilter filter,
    nv_bool cheap
) {
    if (!space) return NULL;

    #ifdef NV_ENABLE_PROFILER

        nvPrecisionTimer timer;
        nvPrecisionTimer_start(&timer);

    #endif

    nvRefArray *collided = nvRefArray_new();
    nvRefArray *result = nvRefArray_new();
    if (!nvRefArray_valid(collided) || !nvRefArray_valid(result)) {
        goto exit;
    }

    if (space->bodies->size == 0) {
        goto exit;
    }

    if (space->bodies->size == 1) {
        nvBVHProxy *single = NV_ARRAY_PTR_AT(space->proxies, 0, nvBVHProxy);
        if (nv_collide_aabb_x_point(nvRigidBody_get_aabb(single->body), point)) {
            nvRefArray_add(collided, single);
        }
        if (collided->size == 0)
            goto exit;
    }
    else {
        nvAABB point_aabb = {point.x, point.y, point.x, point.y};
        nvBVHNode_collide_aabb(&space->bvh_context, space->bvh, point_aabb, collided);
    }

    nvRefArray *removed = nvRefArray_new();
    if (!nvRefArray_valid(removed)) {
        nvRefArray_free(collided);
        collided = NULL;
        goto exit;
    }

    // Remove bodies that do not pass narrowphase
    for (size_t i = 0; i < collided->size; i++) {
        nvBVHProxy *proxy = collided->data[i];

        nv_bool remove = true;

        nvTransform xform = nvRigidBody_get_local_transform(proxy->body);
        nvAABB proxy_aabb = nvShape_get_aabb(proxy->shape, xform);

        if (cheap) {
            if (nv_collide_aabb_x_point(proxy_aabb, point)) {
                remove = false;
            }
        }
        else {
            if (nv_collide_aabb_x_point(proxy_aabb, point)) {
                switch (proxy->shape->type) {
                    case nvShapeType_POLYGON:
                        if (nv_collide_polygon_x_point(proxy->shape, xform, point)) {
                            remove = false;
                        }
                        break;

                    case nvShapeType_CIRCLE:
                        if (nv_collide_circle_x_point(proxy->shape, xform, point)) {
                            remove = false;
                        }
                        break;
                }
            }
        }

        if (remove) {
            nvRefArray_add(removed, proxy);
        }
    }

    // Remove bodies that do not pass filters
    for (size_t i = 0; i < collided->size; i++) {
        nvBVHProxy *proxy = collided->data[i];

        nvRigidBody *body = proxy->body;

        if (!nvCollisionFilter_eval(filter, body->collision_filter)) {
            nvRefArray_add(removed, proxy);
        }
    }

    // Actually remove from results array
    for (size_t i = 0; i < removed->size; i++) {
        nvBVHProxy *proxy = removed->data[i];
        nvRefArray_remove(collided, proxy);
    }

    // Construct results array
    for (size_t i = 0; i < collided->size; i++) {
        nvBVHProxy *proxy = collided->data[i];

        nv_bool found = false;
        for (size_t j = 0; j < result->size; j++) {
            nvRigidBody *body = result->data[j];
            if (body->id == proxy->body->id) {
                found = true;
                break;
            }
        }

        if (!found) {
            nvRefArray_add(result, proxy->body);
        }
    }

    nvRefArray_free(removed);
    nvRefArray_free(collided);

    exit:
    #ifdef NV_ENABLE_PROFILER

        space->profiler.queries += nvPrecisionTimer_stop(&timer);

    #endif
    return result;
}

nvRefArray *nvSpace_query_aabb(
    nvSpace *space,
    nvAABB aabb,
    nvCollisionFilter filter,
    nv_bool cheap
) {
    if (!space) return NULL;

    #ifdef NV_ENABLE_PROFILER

        nvPrecisionTimer timer;
        nvPrecisionTimer_start(&timer);

    #endif

    nvRefArray *collided = nvRefArray_new();
    nvRefArray *result = nvRefArray_new();
    if (!nvRefArray_valid(collided) || !nvRefArray_valid(result)) {
        goto exit;
    }

    if (space->bodies->size == 0) {
        goto exit;
    }

    if (space->bodies->size == 1) {
        nvBVHProxy *single = NV_ARRAY_PTR_AT(space->proxies, 0, nvBVHProxy);
        if (nv_collide_aabb_x_aabb(nvRigidBody_get_aabb(single->body), aabb)) {
            nvRefArray_add(collided, single);
        }
        if (collided->size == 0)
            goto exit;
    }
    else {
        nvBVHNode_collide_aabb(&space->bvh_context, space->bvh, aabb, collided);
    }

    nvRefArray *removed = nvRefArray_new();
    if (!nvRefArray_valid(removed)) {
        nvRefArray_free(collided);
        collided = NULL;
        goto exit;
    }

    // Remove bodies that do not pass narrowphase
    for (size_t i = 0; i < collided->size; i++) {
        nvBVHProxy *proxy = collided->data[i];

        nv_bool remove = true;

        nvTransform xform = nvRigidBody_get_local_transform(proxy->body);
        nvAABB proxy_aabb = nvShape_get_aabb(proxy->shape, xform);

        if (cheap) {
            if (nv_collide_aabb_x_aabb(proxy_aabb, aabb)) {
                remove = false;
            }
        }
        else {
            if (nv_collide_aabb_x_aabb(proxy_aabb, aabb)) {
                switch (proxy->shape->type) {
                    case nvShapeType_POLYGON:
                        if (nv_collide_aabb_x_polygon(aabb, proxy->shape, xform)) {
                            remove = false;
                        }
                        break;

                    case nvShapeType_CIRCLE:
                        if (nv_collide_aabb_x_circle(aabb, proxy->shape, xform)) {
                            remove = false;
                        }
                        break;
                }
            }
        }

        if (remove) {
            nvRefArray_add(removed, proxy);
        }
    }

    // Remove bodies that do not pass filters
    for (size_t i = 0; i < collided->size; i++) {
        nvBVHProxy *proxy = collided->data[i];

        nvRigidBody *body = proxy->body;

        if (!nvCollisionFilter_eval(filter, body->collision_filter)) {
            nvRefArray_add(removed, proxy);
        }
    }

    // Actually remove from results array
    for (size_t i = 0; i < removed->size; i++) {
        nvBVHProxy *proxy = removed->data[i];
        nvRefArray_remove(collided, proxy);
    }

    // Construct results array
    for (size_t i = 0; i < collided->size; i++) {
        nvBVHProxy *proxy = collided->data[i];

        nv_bool found = false;
        for (size_t j = 0; j < result->size; j++) {
            nvRigidBody *body = result->data[j];
            if (body->id == proxy->body->id) {
                found = true;
                break;
            }
        }

        if (!found) {
            nvRefArray_add(result, proxy->body);
        }
    }

    nvRefArray_free(removed);
    nvRefArray_free(collided);

    exit:
    #ifdef NV_ENABLE_PROFILER

        space->profiler.queries += nvPrecisionTimer_stop(&timer);

    #endif
    return result;
}

size_t nvSpace_total_memory_used(nvSpace *space) {
    size_t size = 0;
    if (!space) return size;

    // nvSpace
    size += sizeof(nvSpace);

    // nvSpace->contact_listener
    if (space->listener)
        size += sizeof(nvContactListener);

    // nvSpace->bodies
    size += nvRefArray_total_memory_used(space->bodies);
    nvRigidBody *body;
    size_t body_iter = 0;
    while (nvSpace_iter_bodies(space, &body, &body_iter)) {
        size += nvRigidBody_total_memory_used(body);
    }

    // nvSpace->constraints
    size += nvRefArray_total_memory_used(space->constraints);
    nvConstraint *cons;
    size_t cons_iter = 0;
    while (nvSpace_iter_constraints(space, &cons,  &cons_iter)) {
        size += sizeof(nvConstraint);

        switch (cons->type) {
            case nvConstraintType_DISTANCE:
                size += sizeof(nvDistanceConstraint);
                break;

            case nvConstraintType_HINGE:
                size += sizeof(nvHingeConstraint);
                break;

            case nvConstraintType_SPLINE:
                size += sizeof(nvSplineConstraint);
                break;
        }

        // No need to add a and b into account,
        // rigid bodies are computed in above step. 
    }

    // nvSpace->queued_contact_events
    size += nvArray_total_memory_used(space->queued_contact_events);

    // nvSpace->ignored_pairs
    size += nvHashMap_total_memory_used(space->ignored_pairs);

    // nvSpace->contacts & nvSpace->removed_contacts
    size += nvHashMap_total_memory_used(space->contacts);
    size += nvHashMap_total_memory_used(space->removed_contacts);

    // nvSpace->broadphase_pairs
    size += nvArray_total_memory_used(space->broadphase_pairs);

    size += nvRefArray_total_memory_used(space->bvh_traversed);
    size += sizeof(nvBVHNode) * space->bvh_context.node_max;
    size += sizeof(size_t) * space->bodies->size; // children indices

    return size;
}