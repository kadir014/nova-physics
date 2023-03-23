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


/**
 * @file body.h
 * 
 * @brief A rigid body is a non deformable object with mass in space.
 */


/**
 * @brief Body shape enumerator
 */
typedef enum {
    nv_BodyShape_CIRCLE, /**< Circle shape */
    nv_BodyShape_POLYGON /**< Polygon shape */
} nv_BodyShape;


/**
 * @brief Body type enumerator
 */
typedef enum {
    nv_BodyType_STATIC, /**< Static body has infinite mass and infinite moment
                             of inertia in theory. Its position or velocity
                             doesn't change throughout the simulation.*/

    nv_BodyType_DYNAMIC /**< Dynamic body has its mass and moment of inertia
                             calculated initially and it's not recommended to
                             change those values as it can result in inaccurate
                             simulation. It is affected by every force and constraint
                             in the space. */
} nv_BodyType;


/**
 * @brief Body struct.
 */
typedef struct {
    struct _nv_Space *space; /**< Space object the body is in. */

    nv_BodyType type; /**< Type of the body. */
    nv_BodyShape shape; /**< Shape of the body. */

    nv_Vector2 position; /**< Position of the body. */
    nv_float angle; /**< Rotation of the body in radians. */
    nv_Mat22 u;

    nv_Vector2 linear_velocity; /**< Linear velocity of the body. */
    nv_float angular_velocity; /**< Angular velocity of the bodyin radians/s. */

    nv_Vector2 linear_pseudo;
    nv_float angular_pseudo;

    nv_float linear_damping; /**< Amount of damping applied to linear velocity of the body. */
    nv_float angular_damping; /**< Amount of damping applied to angular velocity of the body. */

    nv_Vector2 force; /**< Force applied on the body. This is reset every space step. */
    nv_float torque; /**< Torque applied on the body. This is reset every space step. */
    
    nv_Material material; /**< Material of the body. */

    nv_float mass; /**< Mass of the body. */
    nv_float invmass; /**< Inverse mass of the body (1/M) This is used for internal calculations. */
    nv_float inertia; /**< Moment of ineartia of the body. */
    nv_float invinertia; /**< Inverse moment of inertia of the body (1/I) This is used for internal calculations. */

    bool is_sleeping; /**< Flag reporting if the body is sleeping. */
    int sleep_timer; /**< Internal sleep counter of the body. */

    bool is_attractor; /**< Flag reporting if the body is an attractor. */

    bool collision;

    nv_uint16 id; /**< Unique identity number of the body. */

    union {
        // For circle body
        nv_float radius;

        // For polygon body
        struct {
            nv_Array *vertices;
            nv_Array *trans_vertices;
            nv_Array *normals;
        };
    };
} nv_Body;

/**
 * @brief Create a new body.
 * 
 * @warning You should not use this method manually,
 *          use helpers like nv_CircleBody_new or nv_PolygonBody_new
 * 
 * @param type Type of the body
 * @param shape Shape of the body
 * 
 * @param position Position of the body
 * @param angle Angle of the body in radians
 * 
 * @param material Material of the body
 * @param area Area of the body shape
 * 
 * @param radius Radius of the body if the shape is circle, else NULL
 * @param vertices Vertices of the body if the shape is polygon, else NULL
 * 
 * @return nv_Body * 
 */
nv_Body *nv_Body_new(
    nv_BodyType type,
    nv_BodyShape shape,
    nv_Vector2 position,
    nv_float angle,
    nv_Material material,
    nv_float radius,
    nv_Array *vertices
);

/**
 * @brief Free body.
 * 
 * @param body Body to free
 */
void nv_Body_free(void *body);

/**
 * @brief Calculate and update mass and moment of inertia of the body.
 * 
 * @param body Body to calculate masses of
 */
void nv_Body_calc_mass_and_inertia(nv_Body *body);

/**
 * @brief Set mass (and moment of inertia) of the body.
 * 
 * @param body Body
 * @param mass Mass
 */
void nv_Body_set_mass(nv_Body *body, nv_float mass);

/**
 * @brief Integrate linear & angular accelerations.
 * 
 * @param body Body to integrate accelerations of
 * @param dt Time step length (delta time)
 */
void nv_Body_integrate_accelerations(
    nv_Body *body,
    nv_Vector2 gravity,
    nv_float dt
);

/**
 * @brief Integrate linear & angular velocities.
 * 
 * @param body Body to integrate velocities of
 * @param dt Time step length (delta time)
 */
void nv_Body_integrate_velocities(nv_Body *body, nv_float dt);

/**
 * @brief Apply attractive force to body towards attractor body.
 * 
 * @param body Body
 * @param attractor Attractor body 
 * @param dt Time step length (delta time)
 */
void nv_Body_apply_attraction(nv_Body *body, nv_Body *attractor);

/**
 * @brief Apply force to body at its center of mass.
 * 
 * @param body Body to apply force on
 * @param force Force
 */
void nv_Body_apply_force(nv_Body *body, nv_Vector2 force);

/**
 * @brief Apply force to body at some local point.
 * 
 * @param body Body to apply force on
 * @param force Force
 * @param position Local point to apply force at
 */
void nv_Body_apply_force_at(
    nv_Body *body,
    nv_Vector2 force,
    nv_Vector2 position
);

/**
 * @brief Apply impulse to body at some local point.
 * 
 * @param body Body to apply impulse on
 * @param impulse Impulse
 * @param position Local point to apply impulse at
 */
void nv_Body_apply_impulse(
    nv_Body *body,
    nv_Vector2 impulse,
    nv_Vector2 position
);

/**
 * @brief Apply pseudo-impulse to body at some local point.
 * 
 * @param body Body to apply impulse on
 * @param impulse Pseudo-impulse
 * @param position Local point to apply impulse at
 */
void nv_Body_apply_pseudo_impulse(
    nv_Body *body,
    nv_Vector2 impulse,
    nv_Vector2 position
);

/**
 * @brief Sleep body.
 * 
 * @param body Body
 */
void nv_Body_sleep(nv_Body *body);

/**
 * @brief Awake body.
 * 
 * @param body Body
 */
void nv_Body_awake(nv_Body *body);

/**
 * @brief Get AABB (Axis-Aligned Bounding Box) of the body.
 * 
 * @param body Body to get AABB of
 * @return nv_AABB 
 */
nv_AABB nv_Body_get_aabb(nv_Body *body);

/**
 * @brief Get kinetic energy of the body in joules.
 * 
 * @param body Body
 * @return nv_float 
 */
nv_float nv_Body_get_kinetic_energy(nv_Body *body);

/**
 * @brief Get rotational kinetic energy of the body in joules.
 * 
 * @param body Body
 * @return nv_float 
 */
nv_float nv_Body_get_rotational_energy(nv_Body *body);

/**
 * @brief Set whether the body is attractor or not 
 * 
 * @param body Body
 * @param is_attractor Is attractor?
 */
void nv_Body_set_is_attractor(nv_Body *body, bool is_attractor);

/**
 * @brief Get whether the body is attractor or not
 * 
 * @param body Body
 * @return bool
 */
bool nv_Body_get_is_attractor(nv_Body *body);


/**
 * @brief Helper function to create a circle body
 * 
 * @param type Type of the body
 * @param position Position of the body
 * @param angle Angle of the body in radians
 * @param material Material of the body
 * @param radius Radius of the body
 * @return nv_Body * 
 */
nv_Body *nv_Circle_new(
    nv_BodyType type,
    nv_Vector2 position,
    nv_float angle,
    nv_Material material,
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
 * @return nv_Body * 
 */
nv_Body *nv_Polygon_new(
    nv_BodyType type,
    nv_Vector2 position,
    nv_float angle,
    nv_Material material,
    nv_Array *vertices
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
 * @return nv_Body * 
 */
nv_Body *nv_Rect_new(
    nv_BodyType type,
    nv_Vector2 position,
    nv_float angle,
    nv_Material material,
    nv_float width,
    nv_float height
);

/**
 * @brief Transform polygon's vertices from local (model) space to world space
 * 
 * @param polygon Polygon to transform its vertices
 */
void nv_Polygon_model_to_world(nv_Body *polygon);


#endif