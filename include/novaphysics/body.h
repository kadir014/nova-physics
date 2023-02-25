/*

  This file is a part of the Nova Physics Engine
  project and distributed under the MIT license.

  Copyright © Kadir Aksoy
  https://github.com/kadir014/nova-physics

*/

#ifndef NOVAPHYSICS_BODY_H
#define NOVAPHYSICS_BODY_H

#include <stdlib.h>
#include "novaphysics/internal.h"
#include "novaphysics/array.h"
#include "novaphysics/vector.h"
#include "novaphysics/aabb.h"
#include "novaphysics/material.h"


/**
 * body.h
 * 
 * Body, body array and helper functions
 */


/**
 * @brief Body shape enumerator
 * 
 * @param CIRCLE Circle shape
 * @param POLYGON Convex polygon shape
 */
typedef enum {
    nv_BodyShape_CIRCLE,
    nv_BodyShape_POLYGON
} nv_BodyShape;


/**
 * @brief Body type enumerator
 * 
 * @param STATIC Static body with infinite mass and inertia in theory
 * @param DYNAMIC Dsynamic body
 */
typedef enum {
    nv_BodyType_STATIC,
    nv_BodyType_DYNAMIC
} nv_BodyType;


/**
 * @brief Body struct. You should not create this manually,
 *        use helpers like nv_CircleBody_new or nv_PolygonBody_new
 * 
 * @param space Space object body is in
 * 
 * @param type Type of the body (static or dynamic)
 * @param shape Shape of the body
 * 
 * @param position Position of the body
 * @param angle Angle of the body in radians
 * 
 * @param linear_velocity Linear velocity of the body
 * @param angular_velocity Angular velocity of the body
 * 
 * @param linear_damping Linear velocity damping (reducing over time)
 * @param angular_damping Angular velocity damping (reducing over time)
 * 
 * @param force Force applied on the body
 * @param torque Torque applied on the body
 * 
 * @param material Material of the body
 * 
 * @param mass Mass of the body
 * @param mass Inverse mass of the body (1/mass)
 * @param inertia Moment of inertia of the body
 * @param invinertia Inverse moment of inertia of the body (1/inertia)
 * 
 * @param is_sleeping Is the body sleeping?
 * @param sleep_counter Amount of ticks passed since body's awakening
 * 
 * @param is_attractor Is the body an attractor?
 * 
 * @param radius Internal value for circle bodies
 * @param vertices Internal value for polygon bodies
 * @param trans_vertices Internal value for polygon bodies
 */
typedef struct {
    struct _nv_Space *space;

    nv_BodyType type;
    nv_BodyShape shape;

    nv_Vector2 position;
    nv_float angle;

    nv_Vector2 linear_velocity;
    nv_float angular_velocity;

    nv_float linear_damping;
    nv_float angular_damping;

    nv_Vector2 force;
    nv_float torque;
    
    nv_Material material;

    nv_float mass;
    nv_float invmass;
    nv_float inertia;
    nv_float invinertia;

    bool is_sleeping;
    int sleep_counter;

    bool is_attractor;

    bool collision;

    union {
        // For circle body
        nv_float radius;

        // For polygon body
        struct {
            nv_Array *vertices;
            nv_Array *trans_vertices;
        };
    };
} nv_Body;

/**
 * @brief Create a new body. You should not use this method manually,
 *        use helpers like nv_CircleBody_new or nv_PolygonBody_new
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
 * @return nv_Body* 
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
 * @brief Free body
 * 
 * @param body Body to free
 */
void nv_Body_free(void *body);

/**
 * @brief Calculate and update mass and moment of inertia of the body
 * 
 * @param body Body to calculate masses of
 */
void nv_Body_calc_mass_and_inertia(nv_Body *body);

/**
 * @brief Integrate linear & angular accelerations
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
 * @brief Integrate linear & angular velocities
 * 
 * @param body Body to integrate velocities of
 * @param dt Time step length (delta time)
 */
void nv_Body_integrate_velocities(nv_Body *body, nv_float dt);

/**
 * @brief Apply attractive force to body towards attractor bdoy
 * 
 * @param body Body
 * @param attractor Attractor body 
 * @param dt Time step length (delta time)
 */
void nv_Body_apply_attraction(nv_Body *body, nv_Body *attractor);

/**
 * @brief Apply force to body at its center of mass
 * 
 * @param body Body to apply force on
 * @param force Force
 */
void nv_Body_apply_force(nv_Body *body, nv_Vector2 force);

/**
 * @brief Apply force to body at some local point
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
 * @brief Sleep body
 * 
 * @param body Body
 */
void nv_Body_sleep(nv_Body *body);

/**
 * @brief Awake body
 * 
 * @param body Body
 */
void nv_Body_awake(nv_Body *body);

/**
 * @brief Get axis-aligned bounding box of body (in Joules)
 * 
 * @param body Body to get AABB of
 * @return nv_AABB 
 */
nv_AABB nv_Body_get_aabb(nv_Body *body);

/**
 * @brief Get kinetic energy of body (in Joules)
 * 
 * @param body Body
 * @return nv_float 
 */
nv_float nv_Body_get_kinetic_energy(nv_Body *body);

/**
 * @brief Get rotational kinetic energy of the body
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