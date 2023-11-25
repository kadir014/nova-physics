/*

  This file is a part of the Nova Physics Engine
  project and distributed under the MIT license.

  Copyright © Kadir Aksoy
  https://github.com/kadir014/nova-physics

*/

#include <stdlib.h>
#include "novaphysics/body.h"
#include "novaphysics/array.h"
#include "novaphysics/math.h"
#include "novaphysics/aabb.h"
#include "novaphysics/constants.h"
#include "novaphysics/space.h"
#include "tracy/TracyC.h"


/**
 * @file body.c
 * 
 * @brief Body struct and methods.
 * 
 * This module defines body enums, Body struct and its methods and some helper
 * functions to create body objects.
 */


nv_Body *nv_Body_new(
    nv_BodyType type,
    nv_Shape *shape,
    nv_Vector2 position,
    nv_float angle,
    nv_Material material
) {
    nv_Body *body = NV_NEW(nv_Body);
    if (!body) return NULL;

    body->space = NULL;

    body->type = type;
    body->shape = shape;

    body->position = position;
    body->angle = angle;

    body->linear_velocity = nv_Vector2_zero;
    body->angular_velocity = 0.0;

    body->linear_pseudo = nv_Vector2_zero;
    body->angular_pseudo = 0.0;

    body->linear_damping = 0.002;
    body->angular_damping = 0.002;

    body->force = nv_Vector2_zero;
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
    body->_cached_aabb = (nv_AABB){0.0, 0.0, 0.0, 0.0};

    nv_Body_calc_mass_and_inertia(body);

    return body;
}

void nv_Body_free(void *body) {
    if (body == NULL) return;
    nv_Body *b = (nv_Body *)body;

    nv_Shape_free(b->shape);
    
    free(b);
}

void nv_Body_calc_mass_and_inertia(nv_Body *body) {
    // -Wmaybe-uninitialized
    body->mass = 0.0;
    body->inertia = 0.0;

    switch (body->type) {
        case nv_BodyType_DYNAMIC:
            switch (body->shape->type) {
                case nv_ShapeType_CIRCLE:
                    body->mass = nv_circle_area(body->shape->radius) * body->material.density;
                    body->inertia = nv_circle_inertia(body->mass, body->shape->radius);
                    break;

                case nv_ShapeType_POLYGON:
                    body->mass = nv_polygon_area(body->shape->vertices) * body->material.density;
                    body->inertia = nv_polygon_inertia(body->mass, body->shape->vertices);
                    break;
            }

            body->invmass = 1.0 / body->mass;
            body->invinertia = 1.0 / body->inertia;

            break;

        case nv_BodyType_STATIC:
            body->mass = 0.0;
            body->inertia = 0.0;
            body->invmass = 0.0;
            body->invinertia = 0.0;
            
            break;
    }
}

void nv_Body_set_mass(nv_Body *body, nv_float mass) {
    if (body->type == nv_BodyType_STATIC) return;

    if (mass == 0.0) NV_ERROR("Can't set mass of a dynamic body to 0\n");

    body->mass = mass;
    body->invmass = 1.0 / body->mass;

    switch (body->shape->type) {
        case nv_ShapeType_CIRCLE:
            body->inertia = nv_circle_inertia(body->mass, body->shape->radius);
            break;

        case nv_ShapeType_POLYGON:
            body->inertia = nv_polygon_inertia(body->mass, body->shape->vertices);
            break;
    }

    body->invinertia = 1.0 / body->inertia;
}

void nv_Body_set_inertia(nv_Body *body, nv_float inertia) {
    if (body->type == nv_BodyType_STATIC) return;

    if (inertia == 0.0) {
        body->inertia = 0.0;
        body->invinertia = 0.0;
    }
    else {
        body->inertia = inertia;
        body->invinertia = 1.0 / inertia;
    }
}

void nv_Body_reset_velocities(nv_Body *body) {
    body->linear_velocity = nv_Vector2_zero;
    body->angular_velocity = 0.0;
    body->linear_pseudo = nv_Vector2_zero;
    body->angular_pseudo = 0.0;
    body->force = nv_Vector2_zero;
    body->torque = 0.0;
}

void nv_Body_integrate_accelerations(
    nv_Body *body,
    nv_Vector2 gravity,
    nv_float dt
) {
    if (body->type == nv_BodyType_STATIC) {
        nv_Body_reset_velocities(body);
        return;
    }
    
    /*
        Integrate linear acceleration

        a = F * (1/M) + g
        v = a * Δt
    */
    nv_Vector2 linear_acceleration = nv_Vector2_add(
        nv_Vector2_mul(body->force, body->invmass), nv_Vector2_mul(gravity, body->gravity_scale));

    body->linear_velocity = nv_Vector2_add(
        body->linear_velocity, nv_Vector2_mul(linear_acceleration, dt));

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
    body->linear_velocity = nv_Vector2_mul(body->linear_velocity, kv);
    body->angular_velocity *= ka;
}

void nv_Body_integrate_velocities(nv_Body *body, nv_float dt) {
    if (body->type == nv_BodyType_STATIC) {
        nv_Body_reset_velocities(body);
        return;
    }
    TracyCZone(profiled_func_zone, true);

    /*
        Integrate linear velocity

        x = v * Δt
    */
    nv_Vector2 linear_velocity = nv_Vector2_add(body->linear_velocity, body->linear_pseudo);
    body->position = nv_Vector2_add(body->position, nv_Vector2_mul(linear_velocity, dt));

    /*
        Integrate angular velocity

        θ = ω * Δt
    */
    nv_float angular_velocity = body->angular_velocity + body->angular_pseudo;
    body->angle += angular_velocity * dt;

    // Reset pseudo-velocities
    body->linear_pseudo = nv_Vector2_zero;
    body->angular_pseudo = 0.0;

    // Reset forces
    body->force = nv_Vector2_zero;
    body->torque = 0.0;

    TracyCZoneEnd(profiled_func_zone);
}

void nv_Body_apply_attraction(nv_Body *body, nv_Body *attractor) {
    nv_float distance = nv_Vector2_dist2(body->position, attractor->position);
    nv_Vector2 direction = nv_Vector2_sub(attractor->position, body->position);
    direction = nv_Vector2_normalize(direction);

    // Fg = (G * Mᴬ * Mᴮ) / d²
    nv_float G = NV_GRAV_CONST * NV_GRAV_SCALE;
    nv_float force_mag = (G * body->mass * attractor->mass) / distance;
    nv_Vector2 force = nv_Vector2_mul(direction, force_mag);

    nv_Body_apply_force(body, force);
}

void nv_Body_apply_force(nv_Body *body, nv_Vector2 force) {
    if (body->type == nv_BodyType_STATIC) return;

    body->force = nv_Vector2_add(body->force, force);

    nv_Body_awake(body);
}

void nv_Body_apply_force_at(
    nv_Body *body,
    nv_Vector2 force,
    nv_Vector2 position
) {
    if (body->type == nv_BodyType_STATIC) return;

    body->force = nv_Vector2_add(body->force, force);
    body->torque += nv_Vector2_cross(position, force);

    nv_Body_awake(body);
}

void nv_Body_apply_impulse(
    nv_Body *body,
    nv_Vector2 impulse,
    nv_Vector2 position
) {
    if (body->type == nv_BodyType_STATIC) return;

    /*
        v -= J * (1/M)
        w -= rᴾ ⨯ J * (1/I)
    */

    body->linear_velocity = nv_Vector2_add(
        body->linear_velocity, nv_Vector2_mul(impulse, body->invmass));

    body->angular_velocity += nv_Vector2_cross(position, impulse) * body->invinertia;
}

void nv_Body_apply_pseudo_impulse(
    nv_Body *body,
    nv_Vector2 impulse,
    nv_Vector2 position
) {
    if (body->type == nv_BodyType_STATIC) return;

    /*
        v -= Jb * (1/M)
        w -= rᴾ ⨯ Jb * (1/I)
    */

    body->linear_pseudo = nv_Vector2_add(
        body->linear_pseudo, nv_Vector2_mul(impulse, body->invmass));

    body->angular_pseudo += nv_Vector2_cross(position, impulse) * body->invinertia;
}

void nv_Body_sleep(nv_Body *body) {
    if (body->type != nv_BodyType_STATIC) {
        body->is_sleeping = true;
        body->linear_velocity = nv_Vector2_zero;
        body->angular_velocity = 0.0;
        body->linear_pseudo = nv_Vector2_zero;
        body->angular_pseudo = 0.0;
        body->force = nv_Vector2_zero;
        body->torque = 0.0;
    }
}

void nv_Body_awake(nv_Body *body) {
    body->is_sleeping = false;
    body->sleep_timer = 0;
}

nv_AABB nv_Body_get_aabb(nv_Body *body) {
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
            case nv_ShapeType_CIRCLE:
                body->_cached_aabb = (nv_AABB){
                    body->position.x - body->shape->radius,
                    body->position.y - body->shape->radius,
                    body->position.x + body->shape->radius,
                    body->position.y + body->shape->radius
                };

                NV_TRACY_ZONE_END;
                return body->_cached_aabb;

            case nv_ShapeType_POLYGON:
                min_x = NV_INF;
                min_y = NV_INF;
                max_x = -NV_INF;
                max_y = -NV_INF;

                nv_Polygon_model_to_world(body);

                for (size_t i = 0; i < body->shape->trans_vertices->size; i++) {
                    nv_Vector2 v = NV_TO_VEC2(body->shape->trans_vertices->data[i]);
                    if (v.x < min_x) min_x = v.x;
                    if (v.x > max_x) max_x = v.x;
                    if (v.y < min_y) min_y = v.y;
                    if (v.y > max_y) max_y = v.y;
                }

                body->_cached_aabb = (nv_AABB){min_x, min_y, max_x, max_y};

                NV_TRACY_ZONE_END;
                return body->_cached_aabb;

            default:
                NV_TRACY_ZONE_END;
                NV_ERROR("Unknown shape type.");
                return (nv_AABB){0.0, 0.0, 0.0, 0.0};
        }
    }

    NV_TRACY_ZONE_END;
}

nv_float nv_Body_get_kinetic_energy(nv_Body *body) {
    // 1/2 * M * v²
    return 0.5 * body->mass * nv_Vector2_len2(body->linear_velocity);
}

nv_float nv_Body_get_rotational_energy(nv_Body *body) {
    // 1/2 * I * ω²
    return 0.5 * body->inertia * fabs(body->angular_velocity);
}

void nv_Body_set_is_attractor(nv_Body *body, bool is_attractor) {
    if (body->is_attractor != is_attractor) {
        body->is_attractor = is_attractor;
        
        if (body->is_attractor) {
            nv_Array_add(body->space->attractors, body);
        }
        else {
            nv_Array_remove(body->space->attractors, body);
        }
    }
}

bool nv_Body_get_is_attractor(nv_Body *body) {
    return body->is_attractor;
}


nv_Body *nv_Circle_new(
    nv_BodyType type,
    nv_Vector2 position,
    nv_float angle,
    nv_Material material,
    nv_float radius
) {
    return nv_Body_new(
        type,
        nv_CircleShape_new(radius),
        position,
        angle,
        material
    );
}

nv_Body *nv_Polygon_new(
    nv_BodyType type,
    nv_Vector2 position,
    nv_float angle,
    nv_Material material,
    nv_Array *vertices
) {
    return nv_Body_new(
        type,
        nv_PolygonShape_new(vertices),
        position,
        angle,
        material
    );
}

nv_Body *nv_Rect_new(
    nv_BodyType type,
    nv_Vector2 position,
    nv_float angle,
    nv_Material material,
    nv_float width,
    nv_float height
) {
    nv_float w = width / 2.0;
    nv_float h = height / 2.0;

    nv_Array *vertices = nv_Array_new();    
    nv_Array_add(vertices, NV_VEC2_NEW(-w, -h));
    nv_Array_add(vertices, NV_VEC2_NEW( w, -h));
    nv_Array_add(vertices, NV_VEC2_NEW( w,  h));
    nv_Array_add(vertices, NV_VEC2_NEW(-w,  h));

    return nv_Polygon_new(
        type,
        position,
        angle,
        material,
        vertices
    );
}

void nv_Polygon_model_to_world(nv_Body *polygon) {
    for (size_t i = 0; i < polygon->shape->vertices->size; i++) {
        nv_Vector2 new = nv_Vector2_add(polygon->position,
            nv_Vector2_rotate(
                NV_TO_VEC2(polygon->shape->vertices->data[i]),
                polygon->angle
                )
            );

        nv_Vector2 *trans = NV_TO_VEC2P(polygon->shape->trans_vertices->data[i]);
        trans->x = new.x;
        trans->y = new.y;
    }
}