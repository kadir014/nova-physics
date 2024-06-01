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
#include "novaphysics/novaphysics.h"

#ifdef NV_WINDOWS
    #include <psapi.h> // To gather memory usage information
#endif


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

/**
 * @brief Example context.
 */
typedef struct {
    SDL_Window *window;
    SDL_GLContext gl_ctx;
    struct nk_context *ui_ctx;
    unsigned int window_width;
    unsigned int window_height;
    Mouse mouse;
    nvVector2 camera;
    float zoom;
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

#define REGISTER_DEMO(name, setup, n) (( \
    example_entries[n] = (ExampleEntry){ \
        .name = name, \
        .setup = setup, \
        .update = NULL \
    }; \
) || ( \
    example_count = n \
)

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
float frand(float lower, float higher) {
    float normal = rand() / (float)RAND_MAX;
    return lower + normal * (higher - lower);
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


#endif