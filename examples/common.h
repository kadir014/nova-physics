/*

  This file is a part of the Nova Physics Engine
  project and distributed under the MIT license.

  Copyright © Kadir Aksoy
  https://github.com/kadir014/nova-physics

*/

#ifndef NOVAPHYSICS_EXAMPLE_COMMON_H
#define NOVAPHYSICS_EXAMPLE_COMMON_H

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <math.h>
#include <time.h>

#include <glad/glad.h>
#include <GL/gl.h>

#define NK_INCLUDE_FIXED_TYPES
#define NK_INCLUDE_STANDARD_IO
#define NK_INCLUDE_STANDARD_VARARGS
#define NK_INCLUDE_DEFAULT_ALLOCATOR
#define NK_INCLUDE_VERTEX_BUFFER_OUTPUT
#define NK_INCLUDE_FONT_BAKING
#define NK_INCLUDE_DEFAULT_FONT
#define NK_IMPLEMENTATION
#define NK_SDL_GL3_IMPLEMENTATION
#include "nuklear/nuklear.h"
#include "nuklear/nuklear_sdl_gl3.h"

#define SDL_MAIN_HANDLED
#include "SDL.h"

#include "novaphysics/novaphysics.h"
#include "novaphysics/bvh.h"

#ifdef NV_WINDOWS
    #include <windows.h>
    #include <psapi.h> // To gather memory usage information
#endif


typedef struct {
    double r, g, b, a;
} FColor;

static inline FColor FColor_lerp(FColor a, FColor b, double t) {
    return (FColor){
        a.r + (b.r - a.r) * t,
        a.g + (b.g - a.g) * t,
        a.b + (b.b - a.b) * t,
        a.a + (b.a - a.a) * t,
    };
}


/**
 * @brief Mouse information.
 */
typedef struct {
    int x;
    int y;
    nv_bool left;
    nv_bool right;
    nv_bool middle;
} Mouse;

/**
 * @brief Example settings used for initialization.
 */
typedef struct {
    unsigned int window_width;
    unsigned int window_height;
} ExampleSettings;

typedef struct {
    FColor dynamic_body;
    FColor static_body;
    FColor distance_constraint;
    FColor hinge_constraint;
    FColor spline_constraint;
    FColor ui_accent;
    FColor ui_text;
} ExampleTheme;

/**
 * @brief Example context.
 */
typedef struct {
    SDL_Window *window;
    SDL_GLContext gl_ctx;
    struct nk_context *ui_ctx;
    unsigned int window_width;
    unsigned int window_height;
    nv_bool fullscreen;
    ExampleTheme theme;
    Mouse mouse;
    nvVector2 camera;
    double zoom;
    nvVector2 before_zoom;
    nvVector2 after_zoom;
    nvVector2 pan_start;
    nvSpace *space;
} ExampleContext;

typedef void ( *ExampleCallback)(ExampleContext *);

/**
 * @brief Example demo entry.
 */
typedef struct {
    char *name;
    ExampleCallback setup;
    ExampleCallback update;
} ExampleEntry;

#define EXAMPLE_MAX_ENTRIES 100
extern ExampleEntry example_entries[EXAMPLE_MAX_ENTRIES];
extern size_t example_count;
size_t current_example;

/**
 * @brief Register an example demo.
 */
void ExampleEntry_register(
    char *name,
    ExampleCallback setup,
    ExampleCallback update
);

/**
 * @brief Set the current example demo. 
 */
void ExampleEntry_set_current(char *name);


/**
 * @brief Return random nv_float in given range.
 * 
 * @param lower Min range
 * @param higher Max range
 * @return nv_float 
 */
static inline float frand(float lower, float higher) {
    float normal = rand() / (float)RAND_MAX;
    return lower + normal * (higher - lower);
}

/**
 * @brief Return random nv_uint32 in given range.
 * 
 * @param lower Min range
 * @param higher Max range
 * @return nv_uint32 
 */
static inline nv_uint32 u32rand(nv_uint32 lower, nv_uint32 higher) {
    return (rand() % (higher - lower + 1)) + lower;
}


/**
 * @brief Get current memory usage of this process in bytes.
 * 
 * Returns 0 if it fails to gather information.
 * 
 * @return size_t
 */
size_t get_current_memory_usage() {
    #ifdef NV_WINDOWS

        // https://learn.microsoft.com/en-us/windows/win32/psapi/collecting-memory-usage-information-for-a-process

        HANDLE current_process = GetCurrentProcess();
        PROCESS_MEMORY_COUNTERS_EX pmc;

        if (GetProcessMemoryInfo(current_process, (PROCESS_MEMORY_COUNTERS *)&pmc, sizeof(pmc))) {
            return pmc.WorkingSetSize;
        }
        else {
            return 0;
        }

    #else

        FILE *status = fopen("/proc/self/status", "r");
        
        if (status) {
            char line[128];
            while (fgets(line, 128, status) != NULL) {
                if (strncmp(line, "VmSize:", 7) == 0) {
                    char *val = line + 7;
                    fclose(status);
                    return strtoul(val, NULL, 10) * 1024;
                }
            }
        }
        
        fclose(status);
        return 0;

    #endif
}


/**
 * @brief Generate n-cornered star shape.
 * 
 * @param body Body to add the shapes to
 * @param n Corner count
 * @param r Radius
 */
void add_star_shape(nvRigidBody *body, nv_uint32 n, nv_float r) {
    nv_float base = r * (nv_float)tanf(NV_PI / (nv_float)n);

    nvVector2 p0 = NV_VECTOR2(-base * 0.5, 0.0);
    nvVector2 p1 = NV_VECTOR2(base * 0.5, 0.0);
    nvVector2 p2 = NV_VECTOR2(0.0, r);

    for (nv_uint32 i = 0; i < n; i++) {
        nv_float an = (nv_float)i * (2.0 * NV_PI / (nv_float)n);

        nvVector2 t0 = nvVector2_rotate(p0, an);
        nvVector2 t1 = nvVector2_rotate(p1, an);
        nvVector2 t2 = nvVector2_rotate(p2, an);

        nvShape *tri = nvPolygonShape_new((nvVector2[3]){t0, t1, t2}, 3, nvVector2_zero);
        nvRigidBody_add_shape(body, tri);
    }
}

/**
 * @brief Generate a circular softbody with spring distance constraints.
 * 
 * @param example 
 * @param center 
 * @param n 
 * @param radius 
 * @param particle_radius 
 */
void create_circle_softbody(
    ExampleContext *example,
    nvVector2 center,
    size_t n,
    nv_float radius,
    nv_float particle_radius
) {
    nvVector2 arm = NV_VECTOR2(radius, 0.0);
    nvRigidBody **particles = NV_MALLOC(sizeof(nvRigidBody *) * n);

    // Create particles
    for (size_t i = 0; i < n; i++) {
        arm = nvVector2_rotate(arm, 2.0 * NV_PI / (nv_float)n);

        nvRigidBodyInitializer particle_init = nvRigidBodyInitializer_default;
        particle_init.type = nvRigidBodyType_DYNAMIC;
        particle_init.position = nvVector2_add(center, arm);
        particle_init.material = (nvMaterial){.density=1.0, .restitution=0.0, .friction=0.2};
        nvRigidBody *particle = nvRigidBody_new(particle_init);

        nvShape *shape = nvCircleShape_new(nvVector2_zero, particle_radius);
        nvRigidBody_add_shape(particle, shape);

        //nvRigidBody_set_inertia(particle, 0.0);
        nvSpace_add_rigidbody(example->space, particle);
        particles[i] = particle;
    }

    nvDistanceConstraintInitializer spring_init = nvDistanceConstraintInitializer_default;
    spring_init.spring = true;
    spring_init.hertz = 0.6;
    spring_init.damping = 0.07;

    // Create edge links
    for (size_t i = 0; i < n; i++) {
        nvRigidBody *a = particles[i];
        nvRigidBody *b = particles[(i + 1) % n];
        spring_init.a = a;
        spring_init.b = b;

        nvVector2 dir_a = nvVector2_normalize(nvVector2_sub(center, nvRigidBody_get_position(a)));
        nvVector2 dir_b = nvVector2_normalize(nvVector2_sub(center, nvRigidBody_get_position(b)));

        nvVector2 anchor_a0 = nvVector2_mul(dir_a, particle_radius);
        nvVector2 anchor_a1 = nvVector2_mul(dir_a, -particle_radius);

        nvVector2 anchor_b0 = nvVector2_mul(dir_b, particle_radius);
        nvVector2 anchor_b1 = nvVector2_mul(dir_b, -particle_radius);

        nvVector2 anchor_a0_world = nvVector2_add(nvRigidBody_get_position(a), anchor_a0);
        nvVector2 anchor_a1_world = nvVector2_add(nvRigidBody_get_position(a), anchor_a1);
        nvVector2 anchor_b0_world = nvVector2_add(nvRigidBody_get_position(b), anchor_b0);
        nvVector2 anchor_b1_world = nvVector2_add(nvRigidBody_get_position(b), anchor_b1);

        nv_float length = nvVector2_len(nvVector2_sub(anchor_a0_world, anchor_b0_world));
        spring_init.length = length;
        spring_init.anchor_a = anchor_a0;
        spring_init.anchor_b = anchor_b0;
        nvSpace_add_constraint(example->space, nvDistanceConstraint_new(spring_init));

        length = nvVector2_len(nvVector2_sub(anchor_a1_world, anchor_b1_world));
        spring_init.length = length;
        spring_init.anchor_a = anchor_a1;
        spring_init.anchor_b = anchor_b1;
        nvSpace_add_constraint(example->space, nvDistanceConstraint_new(spring_init));

        length = nvVector2_len(nvVector2_sub(anchor_a0_world, anchor_b1_world));
        spring_init.length = length;
        spring_init.anchor_a = anchor_a0;
        spring_init.anchor_b = anchor_b1;
        nvSpace_add_constraint(example->space, nvDistanceConstraint_new(spring_init));

        length = nvVector2_len(nvVector2_sub(anchor_a1_world, anchor_b0_world));
        spring_init.length = length;
        spring_init.anchor_a = anchor_a1;
        spring_init.anchor_b = anchor_b0;
        nvSpace_add_constraint(example->space, nvDistanceConstraint_new(spring_init));
    }

    spring_init.hertz *= 0.6;

    // Create inner links
    for (size_t i = 0; i < n; i++) {
        for (size_t j = i + 2; j < n; j++) {
            nvRigidBody *a = particles[i];
            nvRigidBody *b = particles[j];

            nv_float length = nvVector2_len(nvVector2_sub(nvRigidBody_get_position(b), nvRigidBody_get_position(a)));
            spring_init.a = a;
            spring_init.b = b;
            spring_init.length = length;
            spring_init.anchor_a = nvVector2_zero;
            spring_init.anchor_b = nvVector2_zero;

            nvSpace_add_constraint(example->space, nvDistanceConstraint_new(spring_init));
        }
    }

    NV_FREE(particles);
}


nvVector2 catmull_rom(nvVector2 p0, nvVector2 p1, nvVector2 p2, nvVector2 p3, nv_float t) {
    nv_float t2 = t * t;
    nv_float t3 = t2 * t;

    nv_float x = 0.5 * ((2.0 * p1.x) +
               (-p0.x + p2.x) * t +
               (2.0 * p0.x - 5.0 * p1.x + 4.0 * p2.x - p3.x) * t2 +
               (-p0.x + 3.0 * p1.x - 3.0 * p2.x + p3.x) * t3);

    nv_float y = 0.5 * ((2 * p1.y) +
               (-p0.y + p2.y) * t +
               (2.0 * p0.y - 5.0 * p1.y + 4.0 * p2.y - p3.y) * t2 +
               (-p0.y + 3.0 * p1.y - 3.0 * p2.y + p3.y) * t3);

    return NV_VECTOR2(x, y);
}

void sample_spline(nvSplineConstraint *spline, nvVector2 *sample_points, size_t num_samples) {
    nvVector2 *controls = spline->controls;
    size_t num_controls = spline->num_controls;
    size_t num_segments = num_controls - 3;

    size_t sample_per_segment = num_samples / num_segments;

    size_t sample_i = 0;
    for (size_t i = 0; i < num_segments; i++) {
        for (size_t j = 0; j < sample_per_segment; j++) {
            nv_float t = (nv_float)j / (nv_float)(sample_per_segment - 1);
            nvVector2 p0 = controls[i];
            nvVector2 p1 = controls[i + 1];
            nvVector2 p2 = controls[i + 2];
            nvVector2 p3 = controls[i + 3];
            nvVector2 p = catmull_rom(p0, p1, p2, p3, t);
            sample_points[sample_i++] = p;
        }
    }
}


void bvh_calc_depth(nvBVHNode *node, size_t depth) {
    // if (!node) return;
    // node->depth = depth;
    // nvBVHNode *left_node = &node->nodes[node->left];
    // nvBVHNode *right_node = &node->nodes[node->right];
    // bvh_calc_depth(left_node, depth + 1);
    // bvh_calc_depth(right_node, depth + 1);
}

static inline nv_int64 max3(nv_int64 a, nv_int64 b, nv_int64 c) {
    nv_int64 max_value = a;

    if (b > max_value) {
        max_value = b;
    }

    if (c > max_value) {
        max_value = c;
    }

    return max_value;
}

nv_int64 bvh_max_depth(nvBVHNode *node) {
    return 0;
    // if (!node)
    //     return -1;

    // if (node->is_leaf)
    //     return node->depth;

    // nvBVHNode *left_node = &node->nodes[node->left];
    // nvBVHNode *right_node = &node->nodes[node->right];

    // nv_int64 left_max_depth = bvh_max_depth(left_node);
    // nv_int64 right_max_depth = bvh_max_depth(right_node);

    // return max3(node->depth, left_max_depth, right_max_depth);
}


#endif