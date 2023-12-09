/*

  This file is a part of the Nova Physics Engine
  project and distributed under the MIT license.

  Copyright © Kadir Aksoy
  https://github.com/kadir014/nova-physics

*/

#ifndef NOVAPHYSICS_BODY_H
#define NOVAPHYSICS_BODY_H

#include <stdlib.h>
#include <stdint.h>
#include "novaphysics/internal.h"
#include "novaphysics/array.h"
#include "novaphysics/vector.h"
#include "novaphysics/aabb.h"
#include "novaphysics/material.h"
#include "novaphysics/math.h"
#include "novaphysics/matrix.h"
#include "novaphysics/shape.h"


/**
 * @file body.h
 * 
 * @brief Body struct and methods.
 * 
 * This module defines body enums, Body struct and its methods and some helper
 * functions to create body objects.
 */


/**
 * @brief Body type enumerator.
 */
typedef enum {
    nvBodyType_STATIC, /**< Static bodies do not get affected or moved by any force in the simulation.
                             They behave like they have infinite mass.
                             Generally all terrain and ground objects are static bodies in games. */

    nvBodyType_DYNAMIC /**< Dynamic bodies interact with all the other objects in the space and
                             are effected by all forces, gravity and collisions in the simulation.
                             Their mass is calculated by their shape, and unless you know what you're doing,
                             it's not recommended to change their mass manually.
                             However, if you want a dynamic body that can't rotate,
                             you can set it's inertia to 0. */
} nvBodyType;


/**
 * @brief Body struct.
 * 
 * A rigid body is a non deformable object with mass in space. It can be affected
 * by various forces and constraints depending on its type.
 * 
 * Some things to keep in mind to keep the simulation accurate and stable:
 *  - If you want to move bodies in space, either apply forces (most stable option) or add 
 *    velocity. Changing their position directly means teleporting them around.
 *  - For moving (dynamic) bodies, do not create gigantic or really tiny bodies.
 *    This of course depends on the application but keeping the sizes between
 *    0.1 and 10.0 is a good range.
 *  - Make sure polygon shape vertices' centroid is the same as the body's cente position.
 *    Or else the center of gravity will be off and the rotations will not be accurate. 
 */
typedef struct {
    struct nvSpace *space; /**< Space object the body is in. */

    nv_uint16 id; /**< Unique identity number of the body. */

    nvBodyType type; /**< Type of the body. */
    nvShape *shape; /**< Shape of the body. */

    nvVector2 position; /**< Position of the body. */
    nv_float angle; /**< Rotation of the body in radians. */
    nvMat2x2 u;

    nvVector2 linear_velocity; /**< Linear velocity of the body. */
    nv_float angular_velocity; /**< Angular velocity of the bodyin radians/s. */

    nvVector2 linear_pseudo;
    nv_float angular_pseudo;

    nv_float linear_damping; /**< Amount of damping applied to linear velocity of the body. */
    nv_float angular_damping; /**< Amount of damping applied to angular velocity of the body. */

    nvVector2 force; /**< Force applied on the body. This is reset every space step. */
    nv_float torque; /**< Torque applied on the body. This is reset every space step. */

    nv_float gravity_scale; /**< Scale multiplier to the gravity applied to this body. 1.0 by default. */
    
    nvMaterial material; /**< Material of the body. */

    nv_float mass; /**< Mass of the body. */
    nv_float invmass; /**< Inverse mass of the body (1/M). Used in internal calculations. */
    nv_float inertia; /**< Moment of ineartia of the body. */
    nv_float invinertia; /**< Inverse moment of inertia of the body (1/I). Used in internal calculations. */

    bool is_sleeping; /**< Flag reporting if the body is sleeping. */
    int sleep_timer; /**< Internal sleep counter of the body. */

    bool is_attractor; /**< Flag reporting if the body is an attractor. */

    bool enable_collision; /**< Whether to collide this body with other bodies or not. */
    nv_uint32 collision_group; /**< Collision group of the body.
                                    Bodies that share the same non-zero group do not collide. */
    nv_uint32 collision_category; /**< Bitmask defining this body's collision category. */
    nv_uint32 collision_mask; /**< Bitmask defining this body's collision mask. */

    bool _cache_aabb;
    bool _cache_transform;
    nvAABB _cached_aabb;
} nvBody;

/**
 * @brief Create a new body.
 * 
 * @note Instead of using this method and creating the shape manually, you can
 *       use helper constructors @ref nv_Circle_new, @ref nv_Polygon_new or @ref nv_Rect_new.
 * 
 * @param type Type of the body
 * @param shape Shape of the body
 * @param position Position of the body
 * @param angle Angle of the body in radians
 * @param material Material of the body
 * 
 * @return nvBody * 
 */
nvBody *nvBody_new(
    nvBodyType type,
    nvShape *shape,
    nvVector2 position,
    nv_float angle,
    nvMaterial material
);

/**
 * @brief Free body.
 * 
 * @param body Body to free
 */
void nvBody_free(void *body);

/**
 * @brief Calculate and update mass and moment of inertia of the body.
 * 
 * @param body Body to calculate masses of
 */
void nvBody_calc_mass_and_inertia(nvBody *body);

/**
 * @brief Set mass (and moment of inertia) of the body.
 * 
 * @param body Body
 * @param mass Mass
 */
void nvBody_set_mass(nvBody *body, nv_float mass);

/**
 * @brief Set moment of inertia of the body.
 * 
 * @param body Body
 * @param inertia Moment of inertia
 */
void nvBody_set_inertia(nvBody *body, nv_float inertia);

/**
 * @brief Set all velocities and forces of the body to 0.
 * 
 * @param body Body
 */
void nvBody_reset_velocities(nvBody *body);

/**
 * @brief Integrate linear & angular accelerations.
 * 
 * @param body Body to integrate accelerations of
 * @param dt Time step size (delta time)
 */
void nvBody_integrate_accelerations(
    nvBody *body,
    nvVector2 gravity,
    nv_float dt
);

/**
 * @brief Integrate linear & angular velocities.
 * 
 * @param body Body to integrate velocities of
 * @param dt Time step size (delta time)
 */
void nvBody_integrate_velocities(nvBody *body, nv_float dt);

/**
 * @brief Apply attractive force to body towards attractor body.
 * 
 * @param body Body
 * @param attractor Attractor body 
 * @param dt Time step size (delta time)
 */
void nvBody_apply_attraction(nvBody *body, nvBody *attractor);

/**
 * @brief Apply force to body at its center of mass.
 * 
 * @param body Body to apply force on
 * @param force Force
 */
void nvBody_apply_force(nvBody *body, nvVector2 force);

/**
 * @brief Apply force to body at some local point.
 * 
 * @param body Body to apply force on
 * @param force Force
 * @param position Local point to apply force at
 */
void nvBody_apply_force_at(
    nvBody *body,
    nvVector2 force,
    nvVector2 position
);

/**
 * @brief Apply impulse to body at some local point.
 * 
 * @note This method is mainly used internally by the engine.
 * 
 * @param body Body to apply impulse on
 * @param impulse Impulse
 * @param position Local point to apply impulse at
 */
void nvBody_apply_impulse(
    nvBody *body,
    nvVector2 impulse,
    nvVector2 position
);

/**
 * @brief Apply pseudo-impulse to body at some local point.
 * 
 * @note This method is mainly used internally by the engine.
 * 
 * @param body Body to apply impulse on
 * @param impulse Pseudo-impulse
 * @param position Local point to apply impulse at
 */
void nvBody_apply_pseudo_impulse(
    nvBody *body,
    nvVector2 impulse,
    nvVector2 position
);

/**
 * @brief Sleep body.
 * 
 * @param body Body
 */
void nvBody_sleep(nvBody *body);

/**
 * @brief Awake body.
 * 
 * @param body Body
 */
void nvBody_awake(nvBody *body);

/**
 * @brief Get AABB (Axis-Aligned Bounding Box) of the body.
 * 
 * @param body Body to get AABB of
 * @return nvAABB 
 */
nvAABB nvBody_get_aabb(nvBody *body);

/**
 * @brief Get kinetic energy of the body in joules.
 * 
 * @param body Body
 * @return nv_float 
 */
nv_float nvBody_get_kinetic_energy(nvBody *body);

/**
 * @brief Get rotational kinetic energy of the body in joules.
 * 
 * @param body Body
 * @return nv_float 
 */
nv_float nvBody_get_rotational_energy(nvBody *body);

/**
 * @brief Set whether the body is attractor or not 
 * 
 * @param body Body
 * @param is_attractor Is attractor?
 */
void nvBody_set_is_attractor(nvBody *body, bool is_attractor);

/**
 * @brief Get whether the body is attractor or not
 * 
 * @param body Body
 * @return bool
 */
bool nvBody_get_is_attractor(nvBody *body);


/**
 * @brief Helper function to create a circle body
 * 
 * @param type Type of the body
 * @param position Position of the body
 * @param angle Angle of the body in radians
 * @param material Material of the body
 * @param radius Radius of the body
 * @return nvBody * 
 */
nvBody *nv_Circle_new(
    nvBodyType type,
    nvVector2 position,
    nv_float angle,
    nvMaterial material,
    nv_float radius
);

/**
 * @brief Helper function to create a polygon body
 * 
 * @param type Type of the body
 * @param position Position of the body
 * @param angle Angle of the body in radians
 * @param material Material of the body
 * @param vertices Vertices of the body
 * @return nvBody * 
 */
nvBody *nv_Polygon_new(
    nvBodyType type,
    nvVector2 position,
    nv_float angle,
    nvMaterial material,
    nvArray *vertices
);

/**
 * @brief Helper function to create a rectangle body
 * 
 * @param type Type of the body
 * @param position Position of the body
 * @param angle Angle of the body in radians
 * @param material Material of the body
 * @param width Width of the body
 * @param height Height of the body
 * @return nvBody * 
 */
nvBody *nv_Rect_new(
    nvBodyType type,
    nvVector2 position,
    nv_float angle,
    nvMaterial material,
    nv_float width,
    nv_float height
);

/**
 * @brief Transform polygon's vertices from local (model) space to world space
 * 
 * @param polygon Polygon to transform its vertices
 */
void nv_Polygon_model_to_world(nvBody *polygon);


#endif