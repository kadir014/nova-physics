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
#include "novaphysics/determinism.h"
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
    nvSpace *space = NV_NEW_ZEROED(nvSpace);
    NV_MEM_CHECK(space);

    space->bodies = nvRefArray_new();
    if (!nvRefArray_valid(space->bodies)) {
        goto fail;
    }

    space->constraints = nvRefArray_new();
    if (!nvRefArray_valid(space->constraints)) {
        goto fail;
    }

    space->proxies = nvArray_new(sizeof(nvBVHProxy));
    if (!nvArray_valid(space->proxies)) {
        goto fail;
    }

    space->ignored_pairs = nvHashMap_new(
        sizeof(nvIgnoredConstraintPair),
        nvIgnoredConstraintPair_hasher,
        nvIgnoredConstraintPair_comparer
    );
    if (!nvHashMap_valid(space->ignored_pairs)) {
        goto fail;
    }

    space->broadphase_pairs = nvArray_new_ex(
        sizeof(nvBroadPhasePair), NV_BPH_POOL_INITIAL_SIZE, 2.0f
    );
    if (!nvArray_valid(space->broadphase_pairs)) {
        goto fail;
    }

    space->contacts = nvHashMap_new(
        sizeof(nvPersistentContactPair),
        nvPersistentContactPair_hasher,
        nvPersistentContactPair_comparer
    );
    if (!nvHashMap_valid(space->contacts)) {
        goto fail;
    }

    space->removed_contacts = nvHashMap_new(
        sizeof(nvPersistentContactPair),
        nvPersistentContactPair_hasher,
        nvPersistentContactPair_comparer
    );
    if (!nvHashMap_valid(space->removed_contacts)) {
        goto fail;
    }

    space->sorted_contacts = nvArray_new(sizeof(nvSortedContactPair));
    if (!nvArray_valid(space->sorted_contacts)) {
        goto fail;
    }

    space->bvh = NULL;
    space->bvh_traversed = nvRefArray_new();
    if (!nvRefArray_valid(space->bvh_traversed)) {
        goto fail;
    }
    space->bvh_context = (nvBVHContext){
        .nodes = NV_MALLOC(sizeof(nvBVHNode) * NV_BVH_NODES_INITIAL_SIZE),
        .node_max = NV_BVH_NODES_INITIAL_SIZE,
        .node_count = 0,
        .children = NV_MALLOC(sizeof(size_t) * 1),
        .proxies = space->proxies
    };
    if (!space->bvh_context.nodes || !space->bvh_context.children) {
        nv_set_error("Failed to allocate memory.");
        goto fail;
    }

    space->queued_contact_events = nvArray_new(sizeof(nvSpaceQueuedContactEvent));
    if (!nvArray_valid(space->queued_contact_events)) {
        goto fail;
    }

    // ID 0 is reserved for NULL
    space->id_counter = 1;

    nvSpace_set_gravity(space, NV_VECTOR2(0.0f, 9.81f));

    space->settings = (nvSpaceSettings){
        .baumgarte = 0.2f,
        .penetration_slop = 0.05f,
        .velocity_iterations = 8,
        .substeps = 1,
        .determinism = true,
        .linear_damping = 0.0005f,
        .angular_damping = 0.0005f,
        .warmstarting = true,
        .restitution_mix = nvCoefficientMix_SQRT,
        .friction_mix = nvCoefficientMix_SQRT
    };

    space->listener = NULL;
    space->listener_arg = NULL;

    space->visitor_poly_cb = NULL;
    space->visitor_circle_cb = NULL;

    nvProfiler_reset(&space->profiler);

    return space;

fail:
    nvArray_free(space->queued_contact_events);
    NV_FREE(space->bvh_context.children);
    NV_FREE(space->bvh_context.nodes);
    nvRefArray_free(space->bvh_traversed);
    nvArray_free(space->sorted_contacts);
    nvHashMap_free(space->removed_contacts);
    nvHashMap_free(space->contacts);
    nvArray_free(space->broadphase_pairs);
    nvHashMap_free(space->ignored_pairs);
    nvArray_free(space->proxies);
    nvRefArray_free(space->constraints);
    nvRefArray_free(space->bodies);
    NV_FREE(space);
    return NULL;
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
    nvArray_free(space->sorted_contacts);

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
    if (!space->listener) {
        nvContactListener *alloc = NV_NEW(nvContactListener);
        NV_MEM_CHECKI(alloc);
        space->listener = alloc;
    }

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
    for (size_t i = 0; i < space->bodies->size; i++) {
        nvRigidBody *body = space->bodies->data[i];

        body->space = NULL;
    }

    if (free_all) {
        nvRefArray_clear(space->bodies, (void (*)(void *))nvRigidBody_free);
        nvRefArray_clear(space->constraints, (void (*)(void *))nvConstraint_free);

        // ID counter can wrap back to 1 when all bodies are deleted
        // If they're not freed, wrapping back is unsafe
        space->id_counter = 1;
    }
    else {
        nvRefArray_clear(space->bodies, NULL);
        nvRefArray_clear(space->constraints, NULL);
    }

    nvHashMap_clear(space->ignored_pairs);
    nvArray_clear(space->broadphase_pairs);
    nvHashMap_clear(space->contacts);
    nvHashMap_clear(space->removed_contacts);
    nvArray_clear(space->sorted_contacts);

    NV_FREE(space->bvh_context.children);
    space->bvh_context.children = NV_MALLOC(sizeof(size_t) * 1);
    NV_MEM_CHECKI(space->bvh_context.children);

    space->bvh_context.node_count = 0;
    space->bvh = NULL;

    nvArray_clear(space->proxies);

    if (shrink) {
        nvHashMap *new_contacts = nvHashMap_new(
            sizeof(nvPersistentContactPair),
            nvPersistentContactPair_hasher,
            nvPersistentContactPair_comparer
        );
        if (!nvHashMap_valid(new_contacts)) {
            return 1;
        }

        nvHashMap_free(space->contacts);
        space->contacts = new_contacts;

        nvHashMap *new_removed_contacts = nvHashMap_new(
            sizeof(nvPersistentContactPair),
            nvPersistentContactPair_hasher,
            nvPersistentContactPair_comparer
        );
        if (!nvHashMap_valid(new_removed_contacts)) {
            nvHashMap_free(new_contacts);
            return 1;
        }

        nvHashMap_free(space->removed_contacts);
        space->removed_contacts = new_removed_contacts;

        nvHashMap *new_ignored_pairs = nvHashMap_new(
            sizeof(nvIgnoredConstraintPair),
            nvIgnoredConstraintPair_hasher,
            nvIgnoredConstraintPair_comparer
        );
        if (!nvHashMap_valid(new_ignored_pairs)) {
            nvHashMap_free(new_removed_contacts);
            nvHashMap_free(new_contacts);
            return 1;
        }

        nvHashMap_free(space->ignored_pairs);
        space->ignored_pairs = new_ignored_pairs;

        nvArray *new_sorted_contacts = nvArray_new(sizeof(nvSortedContactPair));
        if (!nvArray_valid(new_sorted_contacts)) {
            nvHashMap_free(new_ignored_pairs);
            nvHashMap_free(new_removed_contacts);
            nvHashMap_free(new_contacts);
            return 1;
        }

        nvArray_free(space->sorted_contacts);
        space->sorted_contacts = new_sorted_contacts;
    }

    return 0;
}

int nvSpace_resize_bvh(nvSpace *space) {
    // TODO: Buraya proxies size 0 nasıl gelebilir araştır
    if (space->proxies->size == 0)
        return 0;

    size_t *new_children = NV_REALLOC(
        space->bvh_context.children,
        sizeof(size_t) * space->proxies->size
    );
    NV_MEM_CHECKI(new_children);

    space->bvh_context.children = new_children;

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
        body->cache_aabb = false;
        shape->aabb_cache = false;
        shape->xform_cache = false;
    }

    return 0;
}

int nvSpace_add_rigidbody(nvSpace *space, nvRigidBody *body) {
    if (body->space) {
        nv_set_error("This rigid body is already attached to a space.");
        return 2;
    }

    if (nvSpace_add_proxies(space, body))
        return 1;

    if (nvSpace_resize_bvh(space))
        return 1;

    // Officially add to space after proxies are successfully created
    if (nvRefArray_add(space->bodies, body)) {
        nvSpace_remove_proxies(space, body);
        return 1;
    }

    body->space = space;
    body->id = space->id_counter;

    space->id_counter++;
    // This is practically impossible, but in case the ID counter gets exhausted,
    // overflow to 1 because ID 0 is reserved for NULL.
    if (space->id_counter == NV_INVALID_INDEX_U32) {
        space->id_counter = 1;
    }

    return 0;
}

int nvSpace_add_rigidbody_rebuild(nvSpace *space, nvRigidBody *body) {
    int ret = nvSpace_add_rigidbody(space, body);
    if (ret != 0) return ret;

    nv_broadphase_build_BVH(space);

    return 0;
}

static int nvSpace_remove_proxies(nvSpace *space, nvRigidBody *body) {
    for (size_t i = space->proxies->size; i-- > 0;) {
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
    for (size_t i = space->broadphase_pairs->size; i-- > 0;) {
        nvBroadPhasePair *pair = NV_ARRAY_PTR_AT(space->broadphase_pairs, i, nvBroadPhasePair);

        nvBVHProxy *proxy_a = pair->a;
        nvBVHProxy *proxy_b = pair->b;

        nvRigidBody *body_a = proxy_a->body;
        nvRigidBody *body_b = proxy_b->body;

        if (!body_a || !body_b) continue;

        if (body_a->id == body->id || body_b->id == body->id) {
            nvArray_pop(space->broadphase_pairs, i);
        }
    }

    // Remove proxies after cleaning boradphase pairs, because they refer to proxies
    if (nvSpace_remove_proxies(space, body))
        return 1;

    // Remove contacts
    void *contact_v = NULL;
    size_t contact_i = 0;
    while (nvHashMap_iter(space->contacts, &contact_i, &contact_v)) {
        nvPersistentContactPair *pcp = contact_v;

        if (pcp->body_a->id == body->id || pcp->body_b->id == body->id) {
            nvPersistentContactPair_remove(space, pcp);

            contact_i = 0;
            continue;
        }
    }

    // Remove contact events AFTER removing contacts, because that can add new events to queue
    // also reverse loop in order to avoid a second array allocation
    for (size_t i = space->queued_contact_events->size; i-- > 0;) {
        nvSpaceQueuedContactEvent *event = NV_ARRAY_PTR_AT(
            space->queued_contact_events,
            i,
            nvSpaceQueuedContactEvent
        );

        if (
            event->event.body_a->id == body->id ||
            event->event.body_b->id == body->id
        ) {
            nvArray_pop(space->queued_contact_events, i);
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
        else {
            // Do not decrement ignore_collision reference if body is not attached to this constraint
            continue;
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

    nv_broadphase_BVH(space);
    // TODO: broadphase_finalize as well?

    return 0;
}

int nvSpace_add_constraint(nvSpace *space, nvConstraint *cons) {
    // TODO: This function doesn't check whether the constraint was attached to
    //       another space like `nvSpace_add_rigidbody` does. 

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
    if (!space || !body || !index) {
        return false;
    }

    if (space->bodies->size == 0) {
        *body = NULL;
        *index = 0;
        return false;
    }

    if (*index >= space->bodies->size) {
        return false;
    }

    *body = space->bodies->data[(*index)++];

    return true;
}

nv_bool nvSpace_iter_constraints(nvSpace *space, nvConstraint **cons, size_t *index) {
    if (!space || !cons || !index) {
        return false;
    }

    if (space->constraints->size == 0) {
        *cons = NULL;
        *index = 0;
        return false;
    }

    if (*index >= space->constraints->size) {
        return false;
    }

    *cons = space->constraints->data[(*index)++];

    return true;
}

void nvSpace_step(nvSpace *space, nv_float dt) {
    if (
        !space ||
        dt <= 0.0f ||
        space->settings.substeps == 0
    ) {
        return;
    }

    // Copy settings before simulation in case they get altered with callbacks
    nvSpaceSettings settings = space->settings;

    dt /= (nv_float)(settings.substeps);
    nv_float inv_dt = 1.0f / dt;

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

    for (nv_uint32 substep = 0; substep < settings.substeps; substep++) {
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
            Sort Contacts
            -------------
            Persistent contact pairs are stored in a hashmap, so the lookup is
            not guaranteed to be by creation order.
            Sort by contacts by IDs for deterministic solver results.
        */
        NV_PROFILER_START(timer);
        nvArray_clear(space->sorted_contacts);

        contact_v = NULL;
        contact_i = 0;
        while (nvHashMap_iter(space->contacts, &contact_i, &contact_v)) {
            nvPersistentContactPair *pcp = contact_v;

            nvSortedContactPair sorted_contact = {
                .id = pcp->id,
                .pcp = pcp
            };
            if (nvArray_add(space->sorted_contacts, &sorted_contact) != 0) {
                // TODO: Handle error here
            }
        }

        if (settings.determinism) {
            nv_sort_contacts(space);
        }
        NV_PROFILER_STOP(timer, space->profiler.contact_sort);

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
                (nvConstraint *)space->constraints->data[i],
                dt,
                inv_dt,
                settings.baumgarte
            );
        }

        for (size_t pcp_i = 0; pcp_i < space->sorted_contacts->size; pcp_i++) {
            nvPersistentContactPair *pcp = NV_ARRAY_PTR_AT(space->sorted_contacts, pcp_i, nvSortedContactPair)->pcp;
            nv_contact_presolve(pcp, inv_dt, &settings);
        }
        NV_PROFILER_STOP(timer, space->profiler.presolve);

        // Warmstart constraints
        NV_PROFILER_START(timer);
        for (size_t i = 0; i < space->constraints->size; i++) {
            nvConstraint_warmstart(
                (nvConstraint *)space->constraints->data[i],
                settings.warmstarting
            );
        }

        for (size_t pcp_i = 0; pcp_i < space->sorted_contacts->size; pcp_i++) {
            nvPersistentContactPair *pcp = NV_ARRAY_PTR_AT(space->sorted_contacts, pcp_i, nvSortedContactPair)->pcp;
            nv_contact_warmstart(pcp, settings.warmstarting);
        }
        NV_PROFILER_STOP(timer, space->profiler.warmstart);

        // Solve constraints iteratively
        NV_PROFILER_START(timer);
        for (size_t i = 0; i < settings.velocity_iterations; i++) {
            for (size_t j = 0; j < space->constraints->size; j++) {
                nvConstraint_solve(
                    (nvConstraint *)space->constraints->data[j],
                    inv_dt
                );
            }

            for (size_t pcp_i = 0; pcp_i < space->sorted_contacts->size; pcp_i++) {
                nvPersistentContactPair *pcp = NV_ARRAY_PTR_AT(space->sorted_contacts, pcp_i, nvSortedContactPair)->pcp;
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
    if (!space) return NULL;

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

    if (nvVector2_is_zero(delta) || !nvVector2_is_valid(delta)) {
        goto exit;
    }

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
        // One body might have multiple shapes (proxies in this case)
        for (size_t i = 0; i < space->proxies->size; i++) {
            nvRefArray_add(collided, NV_ARRAY_PTR_AT(space->proxies, i, nvBVHProxy));
        }
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
        for (size_t i = 0; i < space->proxies->size; i++) {
            nvRefArray_add(collided, NV_ARRAY_PTR_AT(space->proxies, i, nvBVHProxy));
        }
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
        for (size_t i = 0; i < space->proxies->size; i++) {
            nvRefArray_add(collided, NV_ARRAY_PTR_AT(space->proxies, i, nvBVHProxy));
        }
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

    // nvSpace->contacts & nvSpace->removed_contacts & nvSpace->sorted_contacts
    size += nvHashMap_total_memory_used(space->contacts);
    size += nvHashMap_total_memory_used(space->removed_contacts);
    size += nvArray_total_memory_used(space->sorted_contacts);

    // nvSpace->broadphase_pairs
    size += nvArray_total_memory_used(space->broadphase_pairs);

    size += nvRefArray_total_memory_used(space->bvh_traversed);
    size += sizeof(nvBVHNode) * space->bvh_context.node_max;
    size += sizeof(size_t) * space->bodies->size; // children indices

    return size;
}

nv_uint128 nvSpace_get_state_hash(const nvSpace *space) {
    nvArray *buffer = nvArray_new(sizeof(nv_uint64));
    if (!nvArray_valid(buffer)) {
        return nv_uint128_zero;
    }

    /*
        TODO: -0 normalization?

        if (f == 0.0f) f = 0.0f;

        This can hide noise, but MIGHT also break some float operations..?

        The local variabla names is not obfuscation, just to protect macros scopes.
    */
    #define ADD_NV_FLOAT(_h_v) do {            \
        nv_float _h_f = (_h_v);                \
        nv_uint64 _h_bits = 0;                 \
        memcpy(&_h_bits, &_h_f, sizeof(_h_f)); \
        nvArray_add(buffer, &_h_bits);         \
    } while (0)

    #define ADD_NV_VECTOR2(_h_v2) do { \
        nvVector2 _h_f2 = (_h_v2);     \
        ADD_NV_FLOAT(_h_f2.x);         \
        ADD_NV_FLOAT(_h_f2.y);         \
    } while (0)

    #define ADD_INTEGER(_h_v, _h_type) do {       \
        _h_type _h_f = (_h_v);                    \
        nv_uint64 _h_bits = 0;                    \
        memcpy(&_h_bits, &_h_f, sizeof(_h_type)); \
        nvArray_add(buffer, &_h_bits);            \
    } while (0)

    // Serialize *significative* simulation state into the nv_uint64 buffer
    // No recalculatable stuff or caches

    for (size_t i = 0; i < space->bodies->size; i++) {
        nvRigidBody *body = space->bodies->data[i];

        ADD_NV_VECTOR2(nvRigidBody_get_position(body));
        ADD_NV_VECTOR2(nvRigidBody_get_linear_velocity(body));
        ADD_NV_FLOAT(nvRigidBody_get_angle(body));
        ADD_NV_FLOAT(nvRigidBody_get_angular_velocity(body));
        ADD_NV_FLOAT(nvRigidBody_get_mass(body));
        ADD_NV_FLOAT(nvRigidBody_get_inertia(body));
        ADD_NV_FLOAT(nvRigidBody_get_linear_damping_scale(body));
        ADD_NV_FLOAT(nvRigidBody_get_angular_damping_scale(body));
        ADD_NV_FLOAT(nvRigidBody_get_gravity_scale(body));
        ADD_NV_FLOAT(nvRigidBody_get_material(body).friction);
        ADD_NV_FLOAT(nvRigidBody_get_material(body).restitution);
        ADD_INTEGER(nvRigidBody_get_collision_filter(body).enabled, nv_bool);
        ADD_INTEGER(nvRigidBody_get_collision_filter(body).group, nv_uint32);
        ADD_INTEGER(nvRigidBody_get_collision_filter(body).category, nv_uint32);
        ADD_INTEGER(nvRigidBody_get_collision_filter(body).mask, nv_uint32);

        for (size_t j = 0; j < body->shapes->size; j++) {
            nvShape *shape = body->shapes->data[j];

            ADD_INTEGER(shape->type, int);
            if (shape->type == nvShapeType_CIRCLE) {
                ADD_NV_VECTOR2(shape->circle.center);
                ADD_NV_FLOAT(shape->circle.radius);
            }
            else if (shape->type == nvShapeType_POLYGON) {
                ADD_INTEGER(shape->polygon.num_vertices, size_t);
                for (size_t k = 0; k < shape->polygon.num_vertices; k++) {
                    ADD_NV_VECTOR2(shape->polygon.vertices[k]);
                }
            }
        }
    }

    for (size_t i = 0; i < space->sorted_contacts->size; i++) {
        nvSortedContactPair *sorted_pair = NV_ARRAY_PTR_AT(space->sorted_contacts, i, nvSortedContactPair);
        nvPersistentContactPair *pcp = sorted_pair->pcp;

        ADD_INTEGER(pcp->contact_count, nv_uint32);
        ADD_NV_VECTOR2(pcp->normal);

        for (size_t j = 0; j < pcp->contact_count; j++) {
            nvContact *contact = &pcp->contacts[j];

            ADD_INTEGER(contact->id, nv_uint64);
            ADD_INTEGER(contact->is_persisted, nv_bool);
            ADD_NV_FLOAT(contact->separation);
            ADD_NV_VECTOR2(contact->anchor_a);
            ADD_NV_VECTOR2(contact->anchor_b);
            ADD_NV_FLOAT(contact->solver_info.normal_impulse);
            ADD_NV_FLOAT(contact->solver_info.tangent_impulse);
        }
    }

    for (size_t i = 0; i < space->constraints->size; i++) {
        nvConstraint *cons = space->constraints->data[i];

        ADD_INTEGER(cons->type, int);
        ADD_INTEGER(cons->ignore_collision, nv_bool);

        if (cons->type == nvConstraintType_DISTANCE) {
            nvDistanceConstraint *def = cons->def;

            ADD_NV_VECTOR2(def->anchor_a);
            ADD_NV_VECTOR2(def->anchor_b);
            ADD_NV_FLOAT(def->length);
            ADD_NV_FLOAT(def->max_force);
            ADD_NV_FLOAT(def->impulse);

            if (def->soft) {
                ADD_NV_FLOAT(def->damping);
                ADD_NV_FLOAT(def->hertz);
            }
        }
        else if (cons->type == nvConstraintType_HINGE) {
            nvHingeConstraint *def = cons->def;

            ADD_NV_VECTOR2(def->anchor);
            ADD_NV_FLOAT(def->reference_angle);
            ADD_NV_FLOAT(def->max_force);
            ADD_NV_FLOAT(def->impulse);

            if (def->enable_limits) {
                ADD_NV_FLOAT(def->upper_limit);
                ADD_NV_FLOAT(def->lower_limit);
                ADD_NV_FLOAT(def->upper_impulse);
                ADD_NV_FLOAT(def->lower_impulse);
            }
        }
        else if (cons->type == nvConstraintType_SPLINE) {
            nvSplineConstraint *def = cons->def;

            ADD_NV_VECTOR2(def->anchor);
            ADD_INTEGER(def->num_controls, size_t);
            ADD_NV_FLOAT(def->max_force);
            ADD_NV_FLOAT(def->impulse);

            // TODO: Serialize more spline constraint fields
        }
    }

    ADD_NV_FLOAT(space->settings.baumgarte);
    ADD_NV_FLOAT(space->settings.linear_damping);
    ADD_NV_FLOAT(space->settings.angular_damping);
    ADD_NV_FLOAT(space->settings.penetration_slop);
    ADD_INTEGER(space->settings.determinism, nv_bool);
    ADD_INTEGER(space->settings.warmstarting, nv_bool);
    ADD_INTEGER(space->settings.velocity_iterations, nv_uint32);
    ADD_INTEGER(space->settings.substeps, nv_uint32);
    ADD_INTEGER(space->settings.friction_mix, int);
    ADD_INTEGER(space->settings.restitution_mix, int);
    ADD_NV_VECTOR2(nvSpace_get_gravity(space));

    nv_uint128 state_hash = nv_murmurhash3_u128(
        buffer->data,
        (int)(buffer->size * sizeof(nv_uint64)), // nv_uint64 -> 8 bytes
        0
    );

    nvArray_free(buffer);

    return state_hash;

    #undef ADD_NV_FLOAT
    #undef ADD_NV_VECTOR2
    #undef ADD_INTEGER
}