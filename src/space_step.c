/*

  This file is a part of the Nova Physics Engine
  project and distributed under the MIT license.

  Copyright © Kadir Aksoy
  https://github.com/kadir014/nova-physics

*/

#include "novaphysics/internal.h"
#include "novaphysics/space_step.h"
#include "novaphysics/space.h"


/**
 * @file space_step.c
 * 
 * @brief Internal functions the space uses in a simulation step.
 */


nv_uint64 _nvSpace_resolution_hash(void *item) {
    nvResolution *res = (nvResolution *)item;
    if (res->a == NULL || res->b == NULL) return 0;
    return (nv_uint64)nv_hash(nv_pair(res->a->id, res->b->id));
}

nv_uint64 _nvSpace_broadphase_pair_hash(void *item) {
    nvBroadPhasePair *pair = item;
    return (nv_uint64)nv_hash(pair->id_pair);
}


void _nvSpace_integrate_accelerations(
    nvSpace *space,
    nv_float dt,
    size_t i
) {
    nvBody *body = (nvBody *)space->bodies->data[i];

    if (body->type != nvBodyType_STATIC) {
        body->_cache_aabb = false;
        body->_cache_transform = false;
    }

    if (space->sleeping && body->is_sleeping) return;

    // Apply attractive forces
    for (size_t j = 0; j < space->attractors->size; j++) {
        nvBody *attractor = (nvBody *)space->attractors->data[j];
        
        if (body == attractor) return;

        nvBody_apply_attraction(body, attractor);
    }
    
    nvBody_integrate_accelerations(body, space->gravity, dt);
}


void _nvSpace_integrate_velocities(
    nvSpace *space,
    nv_float dt,
    size_t i
)  {
    nvBody *body = (nvBody *)space->bodies->data[i];

    if (space->sleeping && body->is_sleeping) return;
    
    nvBody_integrate_velocities(body, dt);

    // Since most kill boundaries in games are going to be out of the
    // display area just checking for body's center position
    // should be sufficient.
    if (
        space->use_kill_bounds &&
        !nv_collide_aabb_x_point(space->kill_bounds, body->position)
    )
        nvSpace_kill(space, body);
}


#ifdef NV_AVX

    void _nvSpace_integrate_accelerations_AVX(
            nvSpace *space,
            nv_float dt
    ) {
        /*
            Notes:
            ------
            It is very unlikely that damping coefficients will change so we can
            just use one pow operation. Or somehow vectorize it.
            Altough there isn't a pow intrinsic...

            Use fused multiply-add for integrations.

            Angular integration can be skipped if the torque is 0?
        */

        #ifdef NV_USE_FLOAT

            __m256 vps_dt = NV_AVX_VECTOR_FROM_FLOAT((float)dt);
            __m256 vps_gravity_x = NV_AVX_VECTOR_FROM_FLOAT((float)(space->gravity.x));
            __m256 vps_gravity_y = NV_AVX_VECTOR_FROM_FLOAT((float)(space->gravity.y));

            size_t n = space->bodies->size;
            size_t vector_n = n / 8 * 8;
                
            for (size_t i = 7; i < vector_n; i += 8) {
                nvBody *body0 = space->bodies->data[i - 7];
                nvBody *body1 = space->bodies->data[i - 6];
                nvBody *body2 = space->bodies->data[i - 5];
                nvBody *body3 = space->bodies->data[i - 4];
                nvBody *body4 = space->bodies->data[i - 3];
                nvBody *body5 = space->bodies->data[i - 2];
                nvBody *body6 = space->bodies->data[i - 1];
                nvBody *body7 = space->bodies->data[i];

                float kv = nv_pow(0.98, body7->linear_damping);
                float ka = nv_pow(0.98, body7->angular_damping);
                __m256 v_kv = NV_AVX_VECTOR_FROM_FLOAT(kv);
                __m256 v_ka = NV_AVX_VECTOR_FROM_FLOAT(ka);

                __m256 v_linear_velocity_x = _mm256_set_ps(
                    body7->linear_velocity.x,
                    body6->linear_velocity.x,
                    body5->linear_velocity.x,
                    body4->linear_velocity.x,
                    body3->linear_velocity.x,
                    body2->linear_velocity.x,
                    body1->linear_velocity.x,
                    body0->linear_velocity.x
                );

                __m256 v_linear_velocity_y = _mm256_set_ps(
                    body7->linear_velocity.y,
                    body6->linear_velocity.y,
                    body5->linear_velocity.y,
                    body4->linear_velocity.y,
                    body3->linear_velocity.y,
                    body2->linear_velocity.y,
                    body1->linear_velocity.y,
                    body0->linear_velocity.y
                );

                __m256 v_force_x = _mm256_set_ps(
                    body7->force.x,
                    body6->force.x,
                    body5->force.x,
                    body4->force.x,
                    body3->force.x,
                    body2->force.x,
                    body1->force.x,
                    body0->force.x
                );

                __m256 v_force_y = _mm256_set_ps(
                    body7->force.y,
                    body6->force.y,
                    body5->force.y,
                    body4->force.y,
                    body3->force.y,
                    body2->force.y,
                    body1->force.y,
                    body0->force.y
                );

                __m256 v_invmass = _mm256_set_ps(
                    body7->invmass,
                    body6->invmass,
                    body5->invmass,
                    body4->invmass,
                    body3->invmass,
                    body2->invmass,
                    body1->invmass,
                    body0->invmass
                );

                // Integrate linear acceleration
                v_linear_velocity_x = _mm256_add_ps(
                    v_linear_velocity_x, 
                    _mm256_mul_ps(
                        _mm256_add_ps(
                            _mm256_mul_ps(v_force_x, v_invmass),
                            vps_gravity_x
                        ),
                        vps_dt
                    )
                );
                v_linear_velocity_y = _mm256_add_ps(
                    v_linear_velocity_y,
                    _mm256_mul_ps(
                        _mm256_add_ps(
                            _mm256_mul_ps(v_force_y, v_invmass),
                            vps_gravity_y
                        ),
                        vps_dt
                    )
                );

                // Apply damping
                v_linear_velocity_x = _mm256_mul_ps(v_linear_velocity_x, v_kv);
                v_linear_velocity_y = _mm256_mul_ps(v_linear_velocity_y, v_kv);

                NV_ALIGNED_AS(32) float final_linear_velocity_x[8];
                NV_ALIGNED_AS(32) float final_linear_velocity_y[8];
                _mm256_store_ps(final_linear_velocity_x, v_linear_velocity_x);
                _mm256_store_ps(final_linear_velocity_y, v_linear_velocity_y);

                body0->linear_velocity.x = final_linear_velocity_x[0];
                body1->linear_velocity.x = final_linear_velocity_x[1];
                body2->linear_velocity.x = final_linear_velocity_x[2];
                body3->linear_velocity.x = final_linear_velocity_x[3];
                body4->linear_velocity.x = final_linear_velocity_x[4];
                body5->linear_velocity.x = final_linear_velocity_x[5];
                body6->linear_velocity.x = final_linear_velocity_x[6];
                body7->linear_velocity.x = final_linear_velocity_x[7];

                body0->linear_velocity.y = final_linear_velocity_y[0];
                body1->linear_velocity.y = final_linear_velocity_y[1];
                body2->linear_velocity.y = final_linear_velocity_y[2];
                body3->linear_velocity.y = final_linear_velocity_y[3];
                body4->linear_velocity.y = final_linear_velocity_y[4];
                body5->linear_velocity.y = final_linear_velocity_y[5];
                body6->linear_velocity.y = final_linear_velocity_y[6];
                body7->linear_velocity.y = final_linear_velocity_y[7];

                __m256 v_angular_velocity = _mm256_set_ps(
                    body7->angular_velocity,
                    body6->angular_velocity,
                    body5->angular_velocity,
                    body4->angular_velocity,
                    body3->angular_velocity,
                    body2->angular_velocity,
                    body1->angular_velocity,
                    body0->angular_velocity
                );

                __m256 v_torque = _mm256_set_ps(
                    body7->torque,
                    body6->torque,
                    body5->torque,
                    body4->torque,
                    body3->torque,
                    body2->torque,
                    body1->torque,
                    body0->torque
                );

                __m256 v_invinertia = _mm256_set_ps(
                    body7->invinertia,
                    body6->invinertia,
                    body5->invinertia,
                    body4->invinertia,
                    body3->invinertia,
                    body2->invinertia,
                    body1->invinertia,
                    body0->invinertia
                );

                // Integrate angular acceleration
                v_angular_velocity = _mm256_add_ps(
                    v_angular_velocity,
                    _mm256_mul_ps(
                        _mm256_mul_ps(v_torque, v_invinertia),
                        vps_dt
                    )
                );

                // Apply damping
                v_angular_velocity = _mm256_mul_ps(v_angular_velocity, v_ka);

                NV_ALIGNED_AS(32) float final_angular_velocity[8];
                _mm256_store_ps(final_angular_velocity, v_angular_velocity);

                body0->angular_velocity = final_angular_velocity[0];
                body1->angular_velocity = final_angular_velocity[1];
                body2->angular_velocity = final_angular_velocity[2];
                body3->angular_velocity = final_angular_velocity[3];
                body4->angular_velocity = final_angular_velocity[4];
                body5->angular_velocity = final_angular_velocity[5];
                body6->angular_velocity = final_angular_velocity[6];
                body7->angular_velocity = final_angular_velocity[7];

                // Cache AABBs and vertex transforms

                if (body0->type == nvBodyType_STATIC) {
                    nvBody_reset_velocities(body0);
                }
                else {
                    body0->_cache_aabb = false;
                    body0->_cache_transform = false;
                }

                if (body1->type == nvBodyType_STATIC) {
                    nvBody_reset_velocities(body1);
                }
                else {
                    body1->_cache_aabb = false;
                    body1->_cache_transform = false;
                }

                if (body2->type == nvBodyType_STATIC) {
                    nvBody_reset_velocities(body2);
                }
                else {
                    body2->_cache_aabb = false;
                    body2->_cache_transform = false;
                }

                if (body3->type == nvBodyType_STATIC) {
                    nvBody_reset_velocities(body3);
                }
                else {
                    body3->_cache_aabb = false;
                    body3->_cache_transform = false;
                }

                if (body4->type == nvBodyType_STATIC) {
                    nvBody_reset_velocities(body4);
                }
                else {
                    body4->_cache_aabb = false;
                    body4->_cache_transform = false;
                }

                if (body5->type == nvBodyType_STATIC) {
                    nvBody_reset_velocities(body5);
                }
                else {
                    body5->_cache_aabb = false;
                    body5->_cache_transform = false;
                }

                if (body6->type == nvBodyType_STATIC) {
                    nvBody_reset_velocities(body6);
                }
                else {
                    body6->_cache_aabb = false;
                    body6->_cache_transform = false;
                }

                if (body7->type == nvBodyType_STATIC) {
                    nvBody_reset_velocities(body7);
                }
                else {
                    body7->_cache_aabb = false;
                    body7->_cache_transform = false;
                }
            }

        #else

            __m256d vpd_dt = NV_AVX_VECTOR_FROM_DOUBLE(dt);
            __m256d vpd_gravity_x = NV_AVX_VECTOR_FROM_DOUBLE(space->gravity.x);
            __m256d vpd_gravity_y = NV_AVX_VECTOR_FROM_DOUBLE(space->gravity.y);

            size_t n = space->bodies->size;
            size_t vector_n = n / 4 * 4;
                
            for (size_t i = 3; i < vector_n; i += 4) {
                nvBody *body0 = space->bodies->data[i - 3];
                nvBody *body1 = space->bodies->data[i - 2];
                nvBody *body2 = space->bodies->data[i - 1];
                nvBody *body3 = space->bodies->data[i];

                double kv = nv_pow(0.98, body3->linear_damping);
                double ka = nv_pow(0.98, body3->angular_damping);
                __m256d v_kv = NV_AVX_VECTOR_FROM_DOUBLE(kv);
                __m256d v_ka = NV_AVX_VECTOR_FROM_DOUBLE(ka);

                __m256d v_linear_velocity_x = _mm256_set_pd(
                    body3->linear_velocity.x,
                    body2->linear_velocity.x,
                    body1->linear_velocity.x,
                    body0->linear_velocity.x
                );

                __m256d v_linear_velocity_y = _mm256_set_pd(
                    body3->linear_velocity.y,
                    body2->linear_velocity.y,
                    body1->linear_velocity.y,
                    body0->linear_velocity.y
                );

                __m256d v_force_x = _mm256_set_pd(
                    body3->force.x,
                    body2->force.x,
                    body1->force.x,
                    body0->force.x
                );

                __m256d v_force_y = _mm256_set_pd(
                    body3->force.y,
                    body2->force.y,
                    body1->force.y,
                    body0->force.y
                );

                __m256d v_invmass = _mm256_set_pd(
                    body3->invmass,
                    body2->invmass,
                    body1->invmass,
                    body0->invmass
                );

                // Integrate linear acceleration
                v_linear_velocity_x = _mm256_add_pd(
                    v_linear_velocity_x, 
                    _mm256_mul_pd(
                        _mm256_add_pd(
                            _mm256_mul_pd(v_force_x, v_invmass),
                            vpd_gravity_x
                        ),
                        vpd_dt
                    )
                );
                v_linear_velocity_y = _mm256_add_pd(
                    v_linear_velocity_y,
                    _mm256_mul_pd(
                        _mm256_add_pd(
                            _mm256_mul_pd(v_force_y, v_invmass),
                            vpd_gravity_y
                        ),
                        vpd_dt
                    )
                );

                // Apply damping
                v_linear_velocity_x = _mm256_mul_pd(v_linear_velocity_x, v_kv);
                v_linear_velocity_y = _mm256_mul_pd(v_linear_velocity_y, v_kv);

                NV_ALIGNED_AS(32) double final_linear_velocity_x[4];
                NV_ALIGNED_AS(32) double final_linear_velocity_y[4];
                _mm256_store_pd(final_linear_velocity_x, v_linear_velocity_x);
                _mm256_store_pd(final_linear_velocity_y, v_linear_velocity_y);

                body0->linear_velocity.x = final_linear_velocity_x[0];
                body1->linear_velocity.x = final_linear_velocity_x[1];
                body2->linear_velocity.x = final_linear_velocity_x[2];
                body3->linear_velocity.x = final_linear_velocity_x[3];

                body0->linear_velocity.y = final_linear_velocity_y[0];
                body1->linear_velocity.y = final_linear_velocity_y[1];
                body2->linear_velocity.y = final_linear_velocity_y[2];
                body3->linear_velocity.y = final_linear_velocity_y[3];

                __m256d v_angular_velocity = _mm256_set_pd(
                    body3->angular_velocity,
                    body2->angular_velocity,
                    body1->angular_velocity,
                    body0->angular_velocity
                );

                __m256d v_torque = _mm256_set_pd(
                    body3->torque,
                    body2->torque,
                    body1->torque,
                    body0->torque
                );

                __m256d v_invinertia = _mm256_set_pd(
                    body3->invinertia,
                    body2->invinertia,
                    body1->invinertia,
                    body0->invinertia
                );

                // Integrate angular acceleration
                v_angular_velocity = _mm256_add_pd(
                    v_angular_velocity,
                    _mm256_mul_pd(
                        _mm256_mul_pd(v_torque, v_invinertia),
                        vpd_dt
                    )
                );

                // Apply damping
                v_angular_velocity = _mm256_mul_pd(v_angular_velocity, v_ka);

                NV_ALIGNED_AS(32) double final_angular_velocity[4];
                _mm256_store_pd(final_angular_velocity, v_angular_velocity);

                body0->angular_velocity = final_angular_velocity[0];
                body1->angular_velocity = final_angular_velocity[1];
                body2->angular_velocity = final_angular_velocity[2];
                body3->angular_velocity = final_angular_velocity[3];

                // Cache AABBs and vertex transforms

                if (body0->type == nvBodyType_STATIC) {
                    nvBody_reset_velocities(body0);
                }
                else {
                    body0->_cache_aabb = false;
                    body0->_cache_transform = false;
                }

                if (body1->type == nvBodyType_STATIC) {
                    nvBody_reset_velocities(body1);
                }
                else {
                    body1->_cache_aabb = false;
                    body1->_cache_transform = false;
                }

                if (body2->type == nvBodyType_STATIC) {
                    nvBody_reset_velocities(body2);
                }
                else {
                    body2->_cache_aabb = false;
                    body2->_cache_transform = false;
                }

                if (body3->type == nvBodyType_STATIC) {
                    nvBody_reset_velocities(body3);
                }
                else {
                    body3->_cache_aabb = false;
                    body3->_cache_transform = false;
                }
            }

        #endif

        // Integrate the rest of the bodies

        for (size_t i = vector_n; i < n; i++) {
            _nvSpace_integrate_accelerations(space, dt, i);
        }
    }


    void _nvSpace_integrate_velocities_AVX(
        nvSpace *space,
        nv_float dt
    ) {
        #ifdef NV_USE_FLOAT

            __m256 vps_dt = NV_AVX_VECTOR_FROM_FLOAT(dt);

            size_t n = space->bodies->size;
            size_t vector_n = n / 8 * 8;
                
            for (size_t i = 7; i < vector_n; i += 8) {
                nvBody *body0 = space->bodies->data[i - 7];
                nvBody *body1 = space->bodies->data[i - 6];
                nvBody *body2 = space->bodies->data[i - 5];
                nvBody *body3 = space->bodies->data[i - 4];
                nvBody *body4 = space->bodies->data[i - 3];
                nvBody *body5 = space->bodies->data[i - 2];
                nvBody *body6 = space->bodies->data[i - 1];
                nvBody *body7 = space->bodies->data[i];

                __m256 v_position_x = _mm256_set_ps(
                    body7->position.x,
                    body6->position.x,
                    body5->position.x,
                    body4->position.x,
                    body3->position.x,
                    body2->position.x,
                    body1->position.x,
                    body0->position.x
                );

                __m256 v_position_y = _mm256_set_ps(
                    body7->position.y,
                    body6->position.y,
                    body5->position.y,
                    body4->position.y,
                    body3->position.y,
                    body2->position.y,
                    body1->position.y,
                    body0->position.y
                );

                __m256 v_linear_velocity_x = _mm256_set_ps(
                    body7->linear_velocity.x,
                    body6->linear_velocity.x,
                    body5->linear_velocity.x,
                    body4->linear_velocity.x,
                    body3->linear_velocity.x,
                    body2->linear_velocity.x,
                    body1->linear_velocity.x,
                    body0->linear_velocity.x
                );

                __m256 v_linear_velocity_y = _mm256_set_ps(
                    body7->linear_velocity.y,
                    body6->linear_velocity.y,
                    body5->linear_velocity.y,
                    body4->linear_velocity.y,
                    body3->linear_velocity.y,
                    body2->linear_velocity.y,
                    body1->linear_velocity.y,
                    body0->linear_velocity.y
                );

                __m256 v_linear_pseudo_x = _mm256_set_ps(
                    body7->linear_pseudo.x,
                    body6->linear_pseudo.x,
                    body5->linear_pseudo.x,
                    body4->linear_pseudo.x,
                    body3->linear_pseudo.x,
                    body2->linear_pseudo.x,
                    body1->linear_pseudo.x,
                    body0->linear_pseudo.x
                );

                __m256 v_linear_pseudo_y = _mm256_set_ps(
                    body7->linear_pseudo.y,
                    body6->linear_pseudo.y,
                    body5->linear_pseudo.y,
                    body4->linear_pseudo.y,
                    body3->linear_pseudo.y,
                    body2->linear_pseudo.y,
                    body1->linear_pseudo.y,
                    body0->linear_pseudo.y
                );

                v_position_x = _mm256_add_ps(v_position_x, _mm256_mul_ps(_mm256_add_ps(v_linear_velocity_x, v_linear_pseudo_x), vps_dt));
                v_position_y = _mm256_add_ps(v_position_y, _mm256_mul_ps(_mm256_add_ps(v_linear_velocity_y, v_linear_pseudo_y), vps_dt));

                NV_ALIGNED_AS(32) float final_position_x[8];
                NV_ALIGNED_AS(32) float final_position_y[8];
                _mm256_store_ps(final_position_x, v_position_x);
                _mm256_store_ps(final_position_y, v_position_y);

                body0->position.x = final_position_x[0];
                body1->position.x = final_position_x[1];
                body2->position.x = final_position_x[2];
                body3->position.x = final_position_x[3];
                body4->position.x = final_position_x[4];
                body5->position.x = final_position_x[5];
                body6->position.x = final_position_x[6];
                body7->position.x = final_position_x[7];

                body0->position.y = final_position_y[0];
                body1->position.y = final_position_y[1];
                body2->position.y = final_position_y[2];
                body3->position.y = final_position_y[3];
                body4->position.y = final_position_y[4];
                body5->position.y = final_position_y[5];
                body6->position.y = final_position_y[6];
                body7->position.y = final_position_y[7];

                __m256 v_angle = _mm256_set_ps(
                    body7->angle,
                    body6->angle,
                    body5->angle,
                    body4->angle,
                    body3->angle,
                    body2->angle,
                    body1->angle,
                    body0->angle
                );

                __m256 v_angular_velocity = _mm256_set_ps(
                    body7->angular_velocity,
                    body6->angular_velocity,
                    body5->angular_velocity,
                    body4->angular_velocity,
                    body3->angular_velocity,
                    body2->angular_velocity,
                    body1->angular_velocity,
                    body0->angular_velocity
                );

                __m256 v_angular_pseudo = _mm256_set_ps(
                    body7->angular_pseudo,
                    body6->angular_pseudo,
                    body5->angular_pseudo,
                    body4->angular_pseudo,
                    body3->angular_pseudo,
                    body2->angular_pseudo,
                    body1->angular_pseudo,
                    body0->angular_pseudo
                );

                v_angle = _mm256_add_ps(v_angle, _mm256_mul_ps(_mm256_add_ps(v_angular_velocity, v_angular_pseudo), vps_dt));

                NV_ALIGNED_AS(32) float final_angle[8];
                _mm256_store_ps(final_angle, v_angle);

                body0->angle = final_angle[0];
                body1->angle = final_angle[1];
                body2->angle = final_angle[2];
                body3->angle = final_angle[3];
                body4->angle = final_angle[4];
                body5->angle = final_angle[5];
                body6->angle = final_angle[6];
                body7->angle = final_angle[7];

                // Reset pseudo-velocities
                body0->linear_pseudo = nvVector2_zero;
                body1->linear_pseudo = nvVector2_zero;
                body2->linear_pseudo = nvVector2_zero;
                body3->linear_pseudo = nvVector2_zero;
                body4->linear_pseudo = nvVector2_zero;
                body5->linear_pseudo = nvVector2_zero;
                body6->linear_pseudo = nvVector2_zero;
                body7->linear_pseudo = nvVector2_zero;
                body0->angular_pseudo = 0.0;
                body1->angular_pseudo = 0.0;
                body2->angular_pseudo = 0.0;
                body3->angular_pseudo = 0.0;
                body4->angular_pseudo = 0.0;
                body5->angular_pseudo = 0.0;
                body6->angular_pseudo = 0.0;
                body7->angular_pseudo = 0.0;

                // Reset forces
                body0->force = nvVector2_zero;
                body1->force = nvVector2_zero;
                body2->force = nvVector2_zero;
                body3->force = nvVector2_zero;
                body4->force = nvVector2_zero;
                body5->force = nvVector2_zero;
                body6->force = nvVector2_zero;
                body7->force = nvVector2_zero;
                body0->torque = 0.0;
                body1->torque = 0.0;
                body2->torque = 0.0;
                body3->torque = 0.0;
                body4->torque = 0.0;
                body5->torque = 0.0;
                body6->torque = 0.0;
                body7->torque = 0.0;

                // Check out-of-bound bodies

                if (space->use_kill_bounds) {
                    if (!nv_collide_aabb_x_point(space->kill_bounds, body0->position))
                        nvSpace_kill(space, body0);

                    if (!nv_collide_aabb_x_point(space->kill_bounds, body1->position))
                        nvSpace_kill(space, body1);

                    if (!nv_collide_aabb_x_point(space->kill_bounds, body2->position))
                        nvSpace_kill(space, body2);

                    if (!nv_collide_aabb_x_point(space->kill_bounds, body3->position))
                        nvSpace_kill(space, body3);

                    if (!nv_collide_aabb_x_point(space->kill_bounds, body4->position))
                        nvSpace_kill(space, body4);

                    if (!nv_collide_aabb_x_point(space->kill_bounds, body5->position))
                        nvSpace_kill(space, body5);

                    if (!nv_collide_aabb_x_point(space->kill_bounds, body6->position))
                        nvSpace_kill(space, body6);

                    if (!nv_collide_aabb_x_point(space->kill_bounds, body7->position))
                        nvSpace_kill(space, body7);
                }
                
            }

        #else

            __m256d vpd_dt = NV_AVX_VECTOR_FROM_DOUBLE(dt);

            size_t n = space->bodies->size;
            size_t vector_n = n / 4 * 4;
                
            for (size_t i = 3; i < vector_n; i += 4) {
                nvBody *body0 = space->bodies->data[i - 3];
                nvBody *body1 = space->bodies->data[i - 2];
                nvBody *body2 = space->bodies->data[i - 1];
                nvBody *body3 = space->bodies->data[i];

                __m256d v_position_x = _mm256_set_pd(
                    body3->position.x,
                    body2->position.x,
                    body1->position.x,
                    body0->position.x
                );

                __m256d v_position_y = _mm256_set_pd(
                    body3->position.y,
                    body2->position.y,
                    body1->position.y,
                    body0->position.y
                );

                __m256d v_linear_velocity_x = _mm256_set_pd(
                    body3->linear_velocity.x,
                    body2->linear_velocity.x,
                    body1->linear_velocity.x,
                    body0->linear_velocity.x
                );

                __m256d v_linear_velocity_y = _mm256_set_pd(
                    body3->linear_velocity.y,
                    body2->linear_velocity.y,
                    body1->linear_velocity.y,
                    body0->linear_velocity.y
                );

                __m256d v_linear_pseudo_x = _mm256_set_pd(
                    body3->linear_pseudo.x,
                    body2->linear_pseudo.x,
                    body1->linear_pseudo.x,
                    body0->linear_pseudo.x
                );

                __m256d v_linear_pseudo_y = _mm256_set_pd(
                    body3->linear_pseudo.y,
                    body2->linear_pseudo.y,
                    body1->linear_pseudo.y,
                    body0->linear_pseudo.y
                );

                v_position_x = _mm256_add_pd(v_position_x, _mm256_mul_pd(_mm256_add_pd(v_linear_velocity_x, v_linear_pseudo_x), vpd_dt));
                v_position_y = _mm256_add_pd(v_position_y, _mm256_mul_pd(_mm256_add_pd(v_linear_velocity_y, v_linear_pseudo_y), vpd_dt));

                NV_ALIGNED_AS(32) double final_position_x[4];
                NV_ALIGNED_AS(32) double final_position_y[4];
                _mm256_store_pd(final_position_x, v_position_x);
                _mm256_store_pd(final_position_y, v_position_y);

                body0->position.x = final_position_x[0];
                body1->position.x = final_position_x[1];
                body2->position.x = final_position_x[2];
                body3->position.x = final_position_x[3];

                body0->position.y = final_position_y[0];
                body1->position.y = final_position_y[1];
                body2->position.y = final_position_y[2];
                body3->position.y = final_position_y[3];

                __m256d v_angle = _mm256_set_pd(
                    body3->angle,
                    body2->angle,
                    body1->angle,
                    body0->angle
                );

                __m256d v_angular_velocity = _mm256_set_pd(
                    body3->angular_velocity,
                    body2->angular_velocity,
                    body1->angular_velocity,
                    body0->angular_velocity
                );

                __m256d v_angular_pseudo = _mm256_set_pd(
                    body3->angular_pseudo,
                    body2->angular_pseudo,
                    body1->angular_pseudo,
                    body0->angular_pseudo
                );

                v_angle = _mm256_add_pd(v_angle, _mm256_mul_pd(_mm256_add_pd(v_angular_velocity, v_angular_pseudo), vpd_dt));

                NV_ALIGNED_AS(32) double final_angle[4];
                _mm256_store_pd(final_angle, v_angle);

                body0->angle = final_angle[0];
                body1->angle = final_angle[1];
                body2->angle = final_angle[2];
                body3->angle = final_angle[3];

                // Reset pseudo-velocities
                body0->linear_pseudo = nvVector2_zero;
                body1->linear_pseudo = nvVector2_zero;
                body2->linear_pseudo = nvVector2_zero;
                body3->linear_pseudo = nvVector2_zero;
                body0->angular_pseudo = 0.0;
                body1->angular_pseudo = 0.0;
                body2->angular_pseudo = 0.0;
                body3->angular_pseudo = 0.0;

                // Reset forces
                body0->force = nvVector2_zero;
                body1->force = nvVector2_zero;
                body2->force = nvVector2_zero;
                body3->force = nvVector2_zero;
                body0->torque = 0.0;
                body1->torque = 0.0;
                body2->torque = 0.0;
                body3->torque = 0.0;

                // Check out-of-bound bodies

                if (space->use_kill_bounds) {
                    if (!nv_collide_aabb_x_point(space->kill_bounds, body0->position))
                        nvSpace_kill(space, body0);

                    if (!nv_collide_aabb_x_point(space->kill_bounds, body1->position))
                        nvSpace_kill(space, body1);

                    if (!nv_collide_aabb_x_point(space->kill_bounds, body2->position))
                        nvSpace_kill(space, body2);

                    if (!nv_collide_aabb_x_point(space->kill_bounds, body3->position))
                        nvSpace_kill(space, body3);
                }
                
            }

        #endif

        // Integrate the rest of the bodies

        for (size_t i = vector_n; i < n; i++) {
            _nvSpace_integrate_velocities(space, dt, i);
        }
    }

#endif