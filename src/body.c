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
 * @brief Body struct and methods.
 * 
 * This module defines body enums, body struct and its methods.
 */


nvBody *nvBody_new(
    nvBodyType type,
    nvShape *shape,
    nvVector2 position,
    nv_float angle,
    nvMaterial material
) {
    nvBody *body = NV_NEW(nvBody);
    if (!body) return NULL;

    body->space = NULL;

    body->type = type;
    body->shape = shape;

    body->position = position;
    body->angle = angle;

    body->linear_velocity = nvVector2_zero;
    body->angular_velocity = 0.0;

    body->linear_damping = 0.002;
    body->angular_damping = 0.002;

    body->force = nvVector2_zero;
    body->torque = 0.0;

    body->gravity_scale = 1.0;

    body->material = material;

    body->is_sleeping = false;
    body->sleep_timer = 0;

    body->is_attractor = false;

    body->enable_collision = true;
    body->collision_group = 0;
    body->collision_category = 0b11111111111111111111111111111111;
    body->collision_mask = 0b11111111111111111111111111111111;

    body->_cache_aabb = false;
    body->_cache_transform = false;
    body->_cached_aabb = (nvAABB){0.0, 0.0, 0.0, 0.0};

    nvBody_calc_mass_and_inertia(body);

    return body;
}

void nvBody_free(void *body) {
    if (body == NULL) return;
    nvBody *b = (nvBody *)body;

    nvShape_free(b->shape);
    
    free(b);
}

void nvBody_calc_mass_and_inertia(nvBody *body) {
    // -Wmaybe-uninitialized
    body->mass = 0.0;
    body->inertia = 0.0;

    switch (body->type) {
        case nvBodyType_DYNAMIC:
            switch (body->shape->type) {
                case nvShapeType_CIRCLE:
                    body->mass = nv_circle_area(body->shape->radius) * body->material.density;
                    body->inertia = nv_circle_inertia(body->mass, body->shape->radius);
                    break;

                case nvShapeType_POLYGON:
                    body->mass = nv_polygon_area(body->shape->vertices) * body->material.density;
                    body->inertia = nv_polygon_inertia(body->mass, body->shape->vertices);
                    break;
            }

            body->invmass = 1.0 / body->mass;
            body->invinertia = 1.0 / body->inertia;

            break;

        case nvBodyType_STATIC:
            body->mass = 0.0;
            body->inertia = 0.0;
            body->invmass = 0.0;
            body->invinertia = 0.0;
            
            break;
    }
}

void nvBody_set_mass(nvBody *body, nv_float mass) {
    if (body->type == nvBodyType_STATIC) return;

    if (mass == 0.0) NV_ERROR("Can't set mass of a dynamic body to 0\n");

    body->mass = mass;
    body->invmass = 1.0 / body->mass;

    switch (body->shape->type) {
        case nvShapeType_CIRCLE:
            body->inertia = nv_circle_inertia(body->mass, body->shape->radius);
            break;

        case nvShapeType_POLYGON:
            body->inertia = nv_polygon_inertia(body->mass, body->shape->vertices);
            break;
    }

    body->invinertia = 1.0 / body->inertia;
}

void nvBody_set_inertia(nvBody *body, nv_float inertia) {
    if (body->type == nvBodyType_STATIC) return;

    if (inertia == 0.0) {
        body->inertia = 0.0;
        body->invinertia = 0.0;
    }
    else {
        body->inertia = inertia;
        body->invinertia = 1.0 / inertia;
    }
}

void nvBody_reset_velocities(nvBody *body) {
    body->linear_velocity = nvVector2_zero;
    body->angular_velocity = 0.0;
    body->force = nvVector2_zero;
    body->torque = 0.0;
}

void nvBody_integrate_accelerations(
    nvBody *body,
    nvVector2 gravity,
    nv_float dt
) {
    if (body->type == nvBodyType_STATIC) {
        nvBody_reset_velocities(body);
        return;
    }
    NV_TRACY_ZONE_START;
    
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

    /*
        Dampen velocities

        v *= kᵥ (linear damping)
        ω *= kₐ (angular damping)
    */
    nv_float kv = nv_pow(0.98, body->linear_damping);
    nv_float ka = nv_pow(0.98, body->angular_damping);
    body->linear_velocity = nvVector2_mul(body->linear_velocity, kv);
    body->angular_velocity *= ka;

    NV_TRACY_ZONE_END;
}

void nvBody_integrate_velocities(nvBody *body, nv_float dt) {
    if (body->type == nvBodyType_STATIC) {
        nvBody_reset_velocities(body);
        return;
    }
    NV_TRACY_ZONE_START;

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

    // Reset forces
    body->force = nvVector2_zero;
    body->torque = 0.0;

    NV_TRACY_ZONE_END;
}

void nvBody_apply_attraction(nvBody *body, nvBody *attractor, nv_float dt) {
    nv_float distance = nvVector2_dist2(body->position, attractor->position);
    nvVector2 direction = nvVector2_sub(attractor->position, body->position);
    direction = nvVector2_normalize(direction);

    // Fg = (G * Mᴬ * Mᴮ) / d²
    nv_float G = NV_GRAV_CONST * NV_GRAV_SCALE;
    nv_float force_mag = (G * body->mass * attractor->mass) / distance;
    nvVector2 force = nvVector2_mul(direction, force_mag * dt);

    nvBody_apply_force(body, force);
}

void nvBody_apply_force(nvBody *body, nvVector2 force) {
    if (body->type == nvBodyType_STATIC) return;

    body->force = nvVector2_add(body->force, force);

    nvBody_awake(body);
}

void nvBody_apply_force_at(
    nvBody *body,
    nvVector2 force,
    nvVector2 position
) {
    if (body->type == nvBodyType_STATIC) return;

    body->force = nvVector2_add(body->force, force);
    body->torque += nvVector2_cross(position, force);

    nvBody_awake(body);
}

void nvBody_apply_impulse(
    nvBody *body,
    nvVector2 impulse,
    nvVector2 position
) {
    if (body->type == nvBodyType_STATIC) return;

    /*
        v -= J * (1/M)
        w -= rᴾ ⨯ J * (1/I)
    */

    body->linear_velocity = nvVector2_add(
        body->linear_velocity, nvVector2_mul(impulse, body->invmass));

    body->angular_velocity += nvVector2_cross(position, impulse) * body->invinertia;
}

void nvBody_sleep(nvBody *body) {
    if (body->type != nvBodyType_STATIC) {
        body->is_sleeping = true;
        body->linear_velocity = nvVector2_zero;
        body->angular_velocity = 0.0;
        body->force = nvVector2_zero;
        body->torque = 0.0;
    }
}

void nvBody_awake(nvBody *body) {
    body->is_sleeping = false;
    body->sleep_timer = 0;
}

nvAABB nvBody_get_aabb(nvBody *body) {
    NV_TRACY_ZONE_START;

    if (body->_cache_aabb) {
        NV_TRACY_ZONE_END;
        return body->_cached_aabb;
    }

    else {
        body->_cache_aabb = true;

        nv_float min_x;
        nv_float min_y;
        nv_float max_x;
        nv_float max_y;

        switch (body->shape->type) {
            case nvShapeType_CIRCLE:
                body->_cached_aabb = (nvAABB){
                    body->position.x - body->shape->radius,
                    body->position.y - body->shape->radius,
                    body->position.x + body->shape->radius,
                    body->position.y + body->shape->radius
                };

                NV_TRACY_ZONE_END;
                return body->_cached_aabb;

            case nvShapeType_POLYGON:
                min_x = NV_INF;
                min_y = NV_INF;
                max_x = -NV_INF;
                max_y = -NV_INF;

                nvBody_local_to_world(body);

                for (size_t i = 0; i < body->shape->trans_vertices->size; i++) {
                    nvVector2 v = NV_TO_VEC2(body->shape->trans_vertices->data[i]);
                    if (v.x < min_x) min_x = v.x;
                    if (v.x > max_x) max_x = v.x;
                    if (v.y < min_y) min_y = v.y;
                    if (v.y > max_y) max_y = v.y;
                }

                body->_cached_aabb = (nvAABB){min_x, min_y, max_x, max_y};

                NV_TRACY_ZONE_END;
                return body->_cached_aabb;

            default:
                NV_TRACY_ZONE_END;
                NV_ERROR("Unknown shape type.");
                return (nvAABB){0.0, 0.0, 0.0, 0.0};
        }
    }

    NV_TRACY_ZONE_END;
}

nv_float nvBody_get_kinetic_energy(nvBody *body) {
    // 1/2 * M * v²
    return 0.5 * body->mass * nvVector2_len2(body->linear_velocity);
}

nv_float nvBody_get_rotational_energy(nvBody *body) {
    // 1/2 * I * ω²
    return 0.5 * body->inertia * fabs(body->angular_velocity);
}

void nvBody_set_is_attractor(nvBody *body, bool is_attractor) {
    if (body->is_attractor != is_attractor) {
        body->is_attractor = is_attractor;
        
        if (body->is_attractor) {
            nvArray_add(body->space->attractors, body);
        }
        else {
            nvArray_remove(body->space->attractors, body);
        }
    }
}

bool nvBody_get_is_attractor(nvBody *body) {
    return body->is_attractor;
}

void nvBody_local_to_world(nvBody *body) {
    NV_TRACY_ZONE_START;

    if (body->_cache_transform) {
        NV_TRACY_ZONE_END;
        return;
    }

    else {
        body->_cache_transform = true;

        for (size_t i = 0; i < body->shape->vertices->size; i++) {
            nvVector2 new = nvVector2_add(body->position,
                nvVector2_rotate(
                    NV_TO_VEC2(body->shape->vertices->data[i]),
                    body->angle
                    )
                );

            nvVector2 *trans = NV_TO_VEC2P(body->shape->trans_vertices->data[i]);
            trans->x = new.x;
            trans->y = new.y;
        }
    }

    NV_TRACY_ZONE_END;
}