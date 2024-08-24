/*

  This file is a part of the Nova Physics Engine
  project and distributed under the MIT license.

  Copyright © Kadir Aksoy
  https://github.com/kadir014/nova-physics

*/

#ifndef NOVAPHYSICS_BODY_H
#define NOVAPHYSICS_BODY_H

#include "novaphysics/internal.h"
#include "novaphysics/core/array.h"
#include "novaphysics/vector.h"
#include "novaphysics/aabb.h"
#include "novaphysics/material.h"
#include "novaphysics/math.h"
#include "novaphysics/shape.h"


/**
 * @file body.h
 * 
 * @brief Rigid body implementation.
 */


/**
 * @brief Rigid body type enumerator.
 */
typedef enum {
    nvRigidBodyType_STATIC, /**< Static bodies do not get affected or moved by any force in the simulation.
                            They behave like they have infinite mass.
                            Generally all terrain and ground objects are static bodies in games. */

    nvRigidBodyType_DYNAMIC /**< Dynamic bodies interact with all the other objects in the space and
                            are effected by all forces, gravity and collisions in the simulation.
                            Their mass is calculated by their shape, and unless you know what you're doing,
                            it's not recommended to change their mass manually.
                            However, if you want a dynamic body that can't rotate,
                            you can set it's inertia to 0. */
} nvRigidBodyType;


/**
 * @brief Rigid body struct.
 * 
 * A rigid body is a non deformable object with mass in space. It can be affected
 * by various forces and constraints depending on its type.
 * 
 * Few things to consider to keep the simulation accurate and stable:
 *  - If you want to move bodies in space, applying forces may be a better solution
 *    than changing velocities directly. Changing transforms (positions and angles)
 *    basically means teleporting them around so you should avoid it unless you
 *    know what you are doing.
 *  - In order to not lose floating point precision, it's best to not variate
 *    sizes of dynamic bodies too much. This of course depends on the application,
 *    but considering the penetration slop setting, keeping the size range around
 *    0.5 and 10.0 would be sufficient in a game.
 */
typedef struct {
    /*
        Private members
    */
    nv_bool cache_aabb;
    nv_bool cache_transform;
    nvAABB cached_aabb;

    // For BVH splitting
    nv_float bvh_median_x;
    nv_float bvh_median_y;

    // Accumulated forces
    nvVector2 force;
    nv_float torque;

    // Inverse masses
    nv_float invmass;
    nv_float invinertia;

    nvVector2 origin; /**< Body shape origin. */
    nvVector2 com; /**< Local center of mass. */

    /*
        Public members (setters & getters)
    */
    void *user_data;

    struct nvSpace *space;

    nv_uint32 id;

    nvRigidBodyType type;

    nvArray *shapes;

    nvVector2 position;
    nv_float angle;

    nvVector2 linear_velocity;
    nv_float angular_velocity;

    nv_float linear_damping_scale;
    nv_float angular_damping_scale;

    nv_float gravity_scale;
    
    nvMaterial material;

    nv_float mass;
    nv_float inertia;

    nv_bool collision_enabled;
    nv_uint32 collision_group;
    nv_uint32 collision_category;
    nv_uint32 collision_mask;
} nvRigidBody;


/**
 * @brief Rigid body initializer information.
 * 
 * This struct holds basic information for initializing bodies and can be reused
 * for multiple bodies.
 */
typedef struct {
    nvRigidBodyType type;
    nvVector2 position;
    nv_float angle;
    nvVector2 linear_velocity;
    nv_float angular_velocity;
    nvMaterial material;
    void *user_data;
} nvRigidBodyInitializer;

static const nvRigidBodyInitializer nvRigidBodyInitializer_default = {
    // It sucks that MSVC doesn't allow designated initializers here
    nvRigidBodyType_STATIC,
    {0.0, 0.0},
    0.0,
    {0.0, 0.0},
    0.0,
    {1.0, 0.1, 0.4},
    NULL
};


/**
 * @brief Create a new body.
 * 
 * When you add the rigid body to a space, space is responsible for the memory management.
 * When you call @ref nvSpace_free it releases all resources it has.
 * But if you removed the body or never added it in the first place, you have to
 * manage the memory.
 * Same thing applies to shapes, if you didn't attach a shape to a body you have to
 * free it yourself.
 * 
 * Returns `NULL` on error. Use @ref nv_get_error to get more information.
 * 
 * @param init Initializer info
 * @return nvRigidBody *
 */
nvRigidBody *nvRigidBody_new(nvRigidBodyInitializer init);

/**
 * @brief Free body.
 * 
 * It's safe to pass `NULL` to this function.
 * 
 * @param body Body to free
 */
void nvRigidBody_free(nvRigidBody *body);

/**
 * @brief Set user data.
 * 
 * @param body Body
 * @param data Void pointer to user data
 */
void nvRigidBody_set_user_data(nvRigidBody *body, void *data);

/**
 * @brief Get user data.
 * 
 * @param body Body
 * @return void *
 */
void *nvRigidBody_get_user_data(const nvRigidBody *body);

/**
 * @brief Get the space instance body belongs to.
 * 
 * @param body Body
 * @return nvSpace *
 */
struct nvSpace *nvRigidBody_get_space(const nvRigidBody *body);

/**
 * @brief Get unique identity number of the body.
 * 
 * @param body 
 * @return nv_uint32
 */
nv_uint32 nvRigidBody_get_id(const nvRigidBody *body);

/**
 * @brief Set motion type of the body.
 * 
 * Returns non-zero on error. Use @ref nv_get_error to get more information.
 * 
 * @param body Body
 * @param type Type
 */
int nvRigidBody_set_type(nvRigidBody *body, nvRigidBodyType type);

/**
 * @brief Get motion type of the body.
 * 
 * @param body Body
 * @return nvRigidBodyType 
 */
nvRigidBodyType nvRigidBody_get_type(const nvRigidBody *body);

/**
 * @brief Set position (center of mass) of body in space.
 * 
 * @param body Body
 * @param new_position New position vector 
 */
void nvRigidBody_set_position(nvRigidBody *body, nvVector2 new_position);

/**
 * @brief Get position (center of mass) of body in space.
 * 
 * @param body Body
 * @return nvVector2 Position vector
 */
nvVector2 nvRigidBody_get_position(const nvRigidBody *body);

/**
 * @brief Set angle (rotation) of body in radians.
 * 
 * If you want to rotate dynamic bodies in a physically accurate manner, applying
 * torques should be the preferred approach.
 * See @ref nvRigidBody_apply_torque
 * 
 * @param body 
 * @param new_angle 
 */
void nvRigidBody_set_angle(nvRigidBody *body, nv_float new_angle);

/**
 * @brief Get angle (rotation) of body in radians.
 * 
 * @param body 
 * @return nv_float 
 */
nv_float nvRigidBody_get_angle(const nvRigidBody *body);

/**
 * @brief Set linear velocity of body.
 * 
 * @param body Body
 * @param new_position New velocity vector
 */
void nvRigidBody_set_linear_velocity(nvRigidBody *body, nvVector2 new_velocity);

/**
 * @brief Get linear velocity of body.
 * 
 * @param body Body
 * @return nvVector2 Velocity vector 
 */
nvVector2 nvRigidBody_get_linear_velocity(const nvRigidBody *body);

/**
 * @brief Set angular velocity of body.
 * 
 * If you want to rotate dynamic bodies in a physically accurate manner, applying
 * torques should be the preferred approach.
 * See @ref nvRigidBody_apply_torque
 * 
 * @param body Body
 * @param new_velocity New velocity 
 */
void nvRigidBody_set_angular_velocity(nvRigidBody *body, nv_float new_velocity);

/**
 * @brief Get angular velocity of body.
 * 
 * @param body Body
 * @return nv_float 
 */
nv_float nvRigidBody_get_angular_velocity(const nvRigidBody *body);

/**
 * @brief Set body's linear velocity damping factor.
 * 
 * The default value 1.0 (100%) means the velocity damping applied to body is not affected.
 * 
 * @param body Body
 * @param scale Scaling factor
 */
void nvRigidBody_set_linear_damping_scale(nvRigidBody *body, nv_float scale);

/**
 * @brief Get body's linear velocity damping factor.
 * 
 * The default value 1.0 (100%) means the velocity damping applied to body is not affected.
 * 
 * @param body Body
 * @return nv_float 
 */
nv_float nvRigidBody_get_linear_damping_scale(const nvRigidBody *body);

/**
 * @brief Set body's angular velocity damping factor.
 * 
 * The default value 1.0 (100%) means the velocity damping applied to body is not affected.
 * 
 * @param body Body
 * @param scale Scaling factor
 */
void nvRigidBody_set_angular_damping_scale(nvRigidBody *body, nv_float scale);

/**
 * @brief Get body's angular velocity damping factor.
 * 
 * The default value 1.0 (100%) means the velocity damping applied to body is not affected.
 * 
 * @param body Body
 * @return nv_float 
 */
nv_float nvRigidBody_get_angular_damping_scale(const nvRigidBody *body);

/**
 * @brief Set gravity scaling factor of body.
 * 
 * The default value 1.0 (100%) means the global gravity applied to body is not affected.
 * 
 * @param body Body
 * @param scale Scaling factor
 */
void nvRigidBody_set_gravity_scale(nvRigidBody *body, nv_float scale);

/**
 * @brief get gravity scaling factor of body.
 * 
 * The default value 1.0 (100%) means the global gravity applied to body is not affected.
 * 
 * @param body Body
 * @return nv_float
 */
nv_float nvRigidBody_get_gravity_scale(const nvRigidBody *body);

/**
 * @brief Set material of body.
 * 
 * @param body Body
 * @param material Material
 */
void nvRigidBody_set_material(nvRigidBody *body, nvMaterial material);

/**
 * @brief Get material of body
 * 
 * @param body Body
 * @return nvMaterial 
 */
nvMaterial nvRigidBody_get_material(const nvRigidBody *body);

/**
 * @brief Set mass of the body.
 * 
 * Ideally you wouldn't need to set mass manually because it is calculated as
 * you add shapes to the body.
 * 
 * Returns non-zero on error. Use @ref nv_get_error to get more information.
 * 
 * @note Currently this doesn't change inertia with the new mass, so use at your own risk.
 * 
 * @param body Body
 * @param mass Mass
 * @return int Status
 */
int nvRigidBody_set_mass(nvRigidBody *body, nv_float mass);

/**
 * @brief Get mass of the body.
 * 
 * @param body Body
 * @return nv_float 
 */
nv_float nvRigidBody_get_mass(const nvRigidBody *body);

/**
 * @brief Set inertia of the body.
 * 
 * If you want to disable rotation you can set inertia to 0.
 * 
 * @param body Body
 * @param inertia Moment of inertia
 */
void nvRigidBody_set_inertia(nvRigidBody *body, nv_float inertia);

/**
 * @brief Get inertia of the body.
 * 
 * @param body Body
 * @return nv_float 
 */
nv_float nvRigidBody_get_inertia(const nvRigidBody *body);

/**
 * @brief Set collision group of body.
 * 
 * Bodies that share the same non-zero group do not collide.
 * 
 * @param body 
 * @param group 
 */
void nvRigidBody_set_collision_group(nvRigidBody *body, nv_uint32 group);

/**
 * @brief Get collision group of body.
 * 
 * Bodies that share the same non-zero group do not collide.
 * 
 * @param body Body
 * @return nv_uint32 
 */
nv_uint32 nvRigidBody_get_collision_group(const nvRigidBody *body);

/**
 * @brief Set collision category of body.
 * 
 * This is a bitmask defining this body's collision category.
 * 
 * @param body Body
 * @param category Category bitmask
 */
void nvRigidBody_set_collision_category(nvRigidBody *body, nv_uint32 category);

/**
 * @brief Get collision category of body.
 * 
 * This is a bitmask defining this body's collision category.
 * 
 * @param body Body
 * @return nv_uint32 
 */
nv_uint32 nvRigidBody_get_collision_category(const nvRigidBody *body);

/**
 * @brief Set collision mask of body.
 * 
 * This is a bitmask defining this body's collision mask.
 * 
 * @param body Body
 * @param category Mask
 */
void nvRigidBody_set_collision_mask(nvRigidBody *body, nv_uint32 mask);

/**
 * @brief Get collision mask of body.
 * 
 * This is a bitmask defining this body's collision mask.
 * 
 * @param body Body
 * @return nv_uint32 
 */
nv_uint32 nvRigidBody_get_collision_mask(const nvRigidBody *body);

/**
 * @brief Add a shape to the body.
 * 
 * Returns non-zero on error. Use @ref nv_get_error to get more information.
 * 
 * @param body Body
 * @param shape Shape
 * @return int Status
 */
int nvRigidBody_add_shape(nvRigidBody *body, nvShape *shape);

/**
 * @brief Remove a shape from the body.
 * 
 * Returns non-zero on error. Use @ref nv_get_error to get more information.
 * 
 * @param body Body
 * @param shape Shape
 * @return int Status
 */
int nvRigidBody_remove_shape(nvRigidBody *body, nvShape *shape);

/**
 * @brief Iterate over this rigid body's shapes.
 * 
 * Make sure to reset the index if you alter the shapes in any way while iterating.
 * 
 * @param body Body
 * @param cons Pointer to shape
 * @param index Pointer to iteration index
 * @return nv_bool 
 */
nv_bool nvRigidBody_iter_shapes(nvRigidBody *body, nvShape **shape, size_t *index);

/**
 * @brief Apply force to body at its center of mass.
 * 
 * @param body Body to apply force on
 * @param force Force
 */
void nvRigidBody_apply_force(nvRigidBody *body, nvVector2 force);

/**
 * @brief Apply force to body at some local point.
 * 
 * @param body Body to apply force on
 * @param force Force
 * @param position Local point to apply force at
 */
void nvRigidBody_apply_force_at(
    nvRigidBody *body,
    nvVector2 force,
    nvVector2 position
);

/**
 * @brief Apply torque to body.
 * 
 * @param body Body to apply torque on
 * @param torque Torque
 */
void nvRigidBody_apply_torque(nvRigidBody *body, nv_float torque);

/**
 * @brief Apply impulse to body at some local point.
 * 
 * An impulse is a sudden change of velocity.
 * Reason of this function existing is mainly for internal use.
 * 
 * @param body Body to apply impulse on
 * @param impulse Impulse
 * @param position Local point to apply impulse at
 */
void nvRigidBody_apply_impulse(
    nvRigidBody *body,
    nvVector2 impulse,
    nvVector2 position
);

/**
 * @brief Enable collisions for this body.
 * 
 * If this is disabled, the body doesn't collide with anything at all.
 * 
 * @param body Body
 */
void nvRigidBody_enable_collisions(nvRigidBody *body);

/**
 * @brief Disable collisions for this body.
 * 
 * If this is disabled, the body doesn't collide with anything at all.
 * 
 * @param body Body
 */
void nvRigidBody_disable_collisions(nvRigidBody *body);

/**
 * @brief Set all velocities and forces of the body to 0.
 * 
 * @param body Body
 */
void nvRigidBody_reset_velocities(nvRigidBody *body);

/**
 * @brief Get AABB (Axis-Aligned Bounding Box) of the body.
 * 
 * @param body Body
 * @return nvAABB 
 */
nvAABB nvRigidBody_get_aabb(nvRigidBody *body);

/**
 * @brief Get kinetic energy of the body in joules.
 * 
 * @param body Body
 * @return nv_float 
 */
nv_float nvRigidBody_get_kinetic_energy(const nvRigidBody *body);

/**
 * @brief Get rotational kinetic energy of the body in joules.
 * 
 * @param body Body
 * @return nv_float 
 */
nv_float nvRigidBody_get_rotational_energy(const nvRigidBody *body);

/**
 * @brief Integrate linear & angular accelerations.
 * 
 * @param body Body
 * @param dt Time step size (delta time)
 */
void nvRigidBody_integrate_accelerations(
    nvRigidBody *body,
    nvVector2 gravity,
    nv_float dt
);

/**
 * @brief Integrate linear & angular velocities.
 * 
 * @param body Body
 * @param dt Time step size (delta time)
 */
void nvRigidBody_integrate_velocities(nvRigidBody *body, nv_float dt);


#endif