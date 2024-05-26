/*

  This file is a part of the Nova Physics Engine
  project and distributed under the MIT license.

  Copyright © Kadir Aksoy
  https://github.com/kadir014/nova-physics

*/

#include <stdlib.h>
#include "novaphysics/internal.h"
#include "novaphysics/body.h"
#include "novaphysics/array.h"
#include "novaphysics/math.h"
#include "novaphysics/aabb.h"
#include "novaphysics/constants.h"
#include "novaphysics/space.h"


/**
 * @file body.c
 * 
 * @brief Rigid body implementation.
 */


// Skip non-dynamic bodies
#define _NV_ONLY_DYNAMIC {if ((body)->type != nvRigidBodyType_DYNAMIC) return;}
#define _NV_ONLY_DYNAMIC0 {if ((body)->type != nvRigidBodyType_DYNAMIC) return 0;}


nvRigidBody *nvRigidBody_new(nvRigidBodyInitializer init) {
    nvRigidBody *body = NV_NEW(nvRigidBody);
    NV_MEM_CHECK(body);

    body->space = NULL;

    body->type = init.type;

    body->shapes = nvArray_new();
    if (!body->shapes) return NULL;

    body->position = init.position;
    body->angle = init.angle;

    body->linear_velocity = init.linear_velocity;
    body->angular_velocity = init.angular_velocity;

    body->linear_damping_scale = 1.0;
    body->angular_damping_scale = 1.0;

    body->force = nvVector2_zero;
    body->torque = 0.0;

    body->gravity_scale = 1.0;

    body->material = init.material;

    body->collision_enabled = true;
    body->collision_group = 0;
    body->collision_category = 0b11111111111111111111111111111111;
    body->collision_mask = 0b11111111111111111111111111111111;

    body->cache_aabb = false;
    body->cache_transform = false;
    body->cached_aabb = (nvAABB){0.0, 0.0, 0.0, 0.0};

    return body;
}

void nvRigidBody_free(void *body) {
    if (!body) return;

    // free shapes
    
    free(body);
}

nvSpace *nvRigidBody_get_space(const nvRigidBody *body) {
    return body->space;
}

nv_uint64 nvRigidBody_get_id(const nvRigidBody *body) {
    return body->id;
}

void nvRigidBody_set_type(nvRigidBody *body, nvRigidBodyType type) {
    body->type = type;
}

nvRigidBodyType nvRigidBody_get_type(const nvRigidBody *body) {
    return body->type;
}

void nvRigidBody_set_position(nvRigidBody *body, nvVector2 new_position) {
    body->position = new_position;
}

nvVector2 nvRigidBody_get_position(const nvRigidBody *body) {
    return body->position;
}

void nvRigidBody_set_angle(nvRigidBody *body, nv_float new_angle) {
    body->angle = new_angle;
}

nv_float nvRigidBody_get_angle(const nvRigidBody *body) {
    return body->angle;
}

void nvRigidBody_set_linear_velocity(nvRigidBody *body, nvVector2 new_velocity) {
    body->linear_velocity = new_velocity;
}

nvVector2 nvRigidBody_get_linear_velocity(const nvRigidBody *body) {
    return body->linear_velocity;
}

void nvRigidBody_set_angular_velocity(nvRigidBody *body, nv_float new_velocity) {
    body->angular_velocity = new_velocity;
}

nv_float nvRigidBody_get_angular_velocity(const nvRigidBody *body) {
    return body->angular_velocity;
}

void nvRigidBody_set_linear_damping_scale(nvRigidBody *body, nv_float scale) {
    body->linear_damping_scale = scale;
}

nv_float nvRigidBody_get_linear_damping_scale(const nvRigidBody *body) {
    return body->linear_damping_scale;
}

void nvRigidBody_set_angular_damping_scale(nvRigidBody *body, nv_float scale) {
    body->angular_damping_scale = scale;
}

nv_float nvRigidBody_get_angular_damping_scale(const nvRigidBody *body) {
    return body->angular_damping_scale;
}

void nvRigidBody_set_gravity_scale(nvRigidBody *body, nv_float scale) {
    body->gravity_scale = scale;
}

nv_float nvRigidBody_get_gravity_scale(const nvRigidBody *body) {
    return body->gravity_scale;
}

void nvRigidBody_set_material(nvRigidBody *body, nvMaterial material) {
    body->material = material;
}

nvMaterial nvRigidBody_get_material(const nvRigidBody *body) {
    return body->material;
}

int nvRigidBody_set_mass(nvRigidBody *body, nv_float mass) {
    _NV_ONLY_DYNAMIC0;

    if (mass == 0.0) {
        nv_set_error("Can't set mass of a dynamic body to 0. Use a static body instead.");
        return 1;
    }

    body->mass = mass;
    body->invmass = 1.0 / body->mass;

    // TODO: Recalculate inertia with updated mass
    //
    // switch (body->shape->type) {
    //     case nvShapeType_CIRCLE:
    //         body->inertia = nv_circle_inertia(body->mass, body->shape->radius);
    //         break;

    //     case nvShapeType_POLYGON:
    //         body->inertia = nv_polygon_inertia(body->mass, body->shape->vertices);
    //         break;
    // }

    body->invinertia = 1.0 / body->inertia;

    return 0;
}

nv_float nvRigidBody_get_mass(const nvRigidBody *body) {
    return body->mass;
}

void nvRigidBody_set_inertia(nvRigidBody *body, nv_float inertia) {
    _NV_ONLY_DYNAMIC;

    if (inertia == 0.0) {
        body->inertia = 0.0;
        body->invinertia = 0.0;
    }
    else {
        body->inertia = inertia;
        body->invinertia = 1.0 / inertia;
    }
}

nv_float nvRigidBody_get_inertia(const nvRigidBody *body) {
    return body->inertia;
}

void nvRigidBody_set_collision_group(nvRigidBody *body, nv_uint32 group) {
    body->collision_group = group;
}

nv_uint32 nvRigidBody_get_collision_group(const nvRigidBody *body) {
    return body->collision_group;
}

void nvRigidBody_set_collision_category(nvRigidBody *body, nv_uint32 category) {
    body->collision_category = category;
}

nv_uint32 nvRigidBody_get_collision_category(const nvRigidBody *body) {
    return body->collision_category;
}

void nvRigidBody_set_collision_mask(nvRigidBody *body, nv_uint32 mask) {
    body->collision_mask = mask;
}

nv_uint32 nvRigidBody_get_collision_mask(const nvRigidBody *body) {
    return body->collision_mask;
}

int nvRigidBody_add_shape(nvRigidBody *body, nvShape *shape) {
    if (nvArray_add(body->shapes, shape)) return 1;

    if (body->type == nvRigidBodyType_DYNAMIC) {
        body->mass = 1.0;
        body->invmass = 1.0;
        body->inertia = 5.0;
        body->invinertia = 1.0 / 5.0;
    }
    else {
        body->mass = 0.0;
        body->invmass = 0.0;
        body->inertia = 0.0;
        body->invinertia = 0.0;
    }
}

void nvRigidBody_apply_force(nvRigidBody *body, nvVector2 force) {
    _NV_ONLY_DYNAMIC;

    body->force = nvVector2_add(body->force, force);
}

void nvRigidBody_apply_force_at(
    nvRigidBody *body,
    nvVector2 force,
    nvVector2 position
) {
    _NV_ONLY_DYNAMIC;

    body->force = nvVector2_add(body->force, force);
    body->torque += nvVector2_cross(position, force);
}

void nvRigidBody_apply_torque(nvRigidBody *body, nv_float torque) {
    _NV_ONLY_DYNAMIC;

    body->torque += torque;
}

void nvRigidBody_apply_impulse(
    nvRigidBody *body,
    nvVector2 impulse,
    nvVector2 position
) {
    _NV_ONLY_DYNAMIC;

    /*
        v -= J * (1/M)
        w -= rᴾ ⨯ J * (1/I)
    */

    body->linear_velocity = nvVector2_add(
        body->linear_velocity, nvVector2_mul(impulse, body->invmass));

    body->angular_velocity += nvVector2_cross(position, impulse) * body->invinertia;
}

void nvRigidBody_enable_collisions(nvRigidBody *body) {
    body->collision_enabled = true;
}

void nvRigidBody_disable_collisions(nvRigidBody *body) {
    body->collision_enabled = false;
}

void nvRigidBody_reset_velocities(nvRigidBody *body) {
    nvRigidBody_set_linear_velocity(body, nvVector2_zero);
    nvRigidBody_set_angular_velocity(body, 0.0);
    body->force = nvVector2_zero;
    body->torque = 0.0;
}

nvAABB nvRigidBody_get_aabb(nvRigidBody *body) {
    NV_TRACY_ZONE_START;

    if (body->cache_aabb) {
        NV_TRACY_ZONE_END;
        return body->cached_aabb;
    }

    body->cache_aabb = true;

    nvTransform xform = (nvTransform){body->position, body->angle};
    nvAABB total_aabb = nvShape_get_aabb(body->shapes->data[0], xform);
    for (size_t i = 1; i < body->shapes->size; i++) {
        total_aabb = nvAABB_merge(total_aabb, nvShape_get_aabb(body->shapes->data[i], xform));
    }

    // TODO: aabb extension constant instead of hardcoded value
    nv_float extension = 0.15;
    total_aabb = nvAABB_inflate(total_aabb, extension);
    body->cached_aabb = total_aabb;

    NV_TRACY_ZONE_END;
    return total_aabb;
}

nv_float nvRigidBody_get_kinetic_energy(const nvRigidBody *body) {
    // 1/2 * M * v²
    return 0.5 * body->mass * nvVector2_len2(body->linear_velocity);
}

nv_float nvRigidBody_get_rotational_energy(const nvRigidBody *body) {
    // 1/2 * I * ω²
    return 0.5 * body->inertia * nv_fabs(body->angular_velocity);
}

void nvRigidBody_integrate_accelerations(
    nvRigidBody *body,
    nvVector2 gravity,
    nv_float dt
) {
    if (body->type == nvRigidBodyType_STATIC) {
        nvRigidBody_reset_velocities(body);
        return;
    }
    NV_TRACY_ZONE_START;

    // Semi-Implicit Euler Integration
    
    /*
        Integrate linear acceleration

        a = F * (1/M) + g
        v = a * Δt
    */
    nvVector2 linear_acceleration = nvVector2_add(
        nvVector2_mul(body->force, body->invmass), nvVector2_mul(gravity, body->gravity_scale));

    body->linear_velocity = nvVector2_add(
        body->linear_velocity, nvVector2_mul(linear_acceleration, dt));

    /*
        Integrate angular acceleration
        
        α = T * (1/I)
        ω = α * Δt
    */
    nv_float angular_acceleration = body->torque * body->invinertia;
    body->angular_velocity += angular_acceleration * dt;

    // Dampen velocities
    nv_float kv = nv_pow(0.98, body->linear_damping_scale * body->space->settings.linear_damping);
    nv_float ka = nv_pow(0.98, body->angular_damping_scale * body->space->settings.angular_damping);
    body->linear_velocity = nvVector2_mul(body->linear_velocity, kv);
    body->angular_velocity *= ka;

    NV_TRACY_ZONE_END;
}

void nvRigidBody_integrate_velocities(nvRigidBody *body, nv_float dt) {
    if (body->type == nvRigidBodyType_STATIC) {
        nvRigidBody_reset_velocities(body);
        return;
    }
    NV_TRACY_ZONE_START;

    // Semi-Implicit Euler Integration

    /*
        Integrate linear velocity

        x = v * Δt
    */
    body->position = nvVector2_add(body->position, nvVector2_mul(body->linear_velocity, dt));

    /*
        Integrate angular velocity

        θ = ω * Δt
    */
    body->angle += body->angular_velocity * dt;

    body->force = nvVector2_zero;
    body->torque = 0.0;

    NV_TRACY_ZONE_END;
}