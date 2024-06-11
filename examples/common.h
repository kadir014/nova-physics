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

#ifdef NV_WINDOWS
    #include <windows.h>
    #include <psapi.h> // To gather memory usage information
#endif


typedef struct {
    double r, g, b, a;
} FColor;


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


#endif