/*

  This file is a part of the Nova Physics Engine
  project and distributed under the MIT license.

  Copyright © Kadir Aksoy
  https://github.com/kadir014/nova-physics

*/

#ifndef NOVAPHYSICS_EXAMPLE_H
#define NOVAPHYSICS_EXAMPLE_H

#include <stdbool.h>
#include <stdint.h>
#include <time.h>
#include <math.h>
#include <string.h> // Required on OSX for some reason
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include "novaphysics/novaphysics.h"

#ifdef NV_WINDOWS

    #include <psapi.h> // To gather memory usage information

#endif


/**
 * @file example.h
 * 
 * @brief This header defines everything needed to setup & run a basic
 *        SDL2 application for Nova Physics example demos.
 * 
 * Utility functions:
 * ------------------
 * MAX
 * irand
 * frand
 * brand
 * hsv_to_rgb
 * fhsv_to_rgb
 * get_current_memory_usage
 * 
 * Drawing functions:
 * ------------------
 * draw_circle
 * fill_circle
 * draw_polygon
 * draw_aaline
 * draw_aapolygon
 * draw_aacircle
 * draw_text
 * draw_text_from_right
 * draw_text_middle
 * draw_spring
 * draw_dashed_line
 * 
 * Example, UI and helper structs:
 * -------------------------
 * Mouse
 * ToggleSwitch
 * Slider
 * ExampleTheme
 * Example
 * 
 * Main loop functions:
 * --------------------
 * draw_ui
 * draw_constraints
 * draw_bodies
 * draw_cloth
 * draw_SHG
 * draw_BVH
 * UI elements update & draw
 */


struct _Example;


/******************************************************************************

                               Utility functions

******************************************************************************/


#define MAX(x, y) (((x) > (y)) ? (x) : (y))

/**
 * @brief Return random integer in given range.
 * 
 * @param lower Min range
 * @param higher Max range
 * @return int 
 */
int irand(int lower, int higher) {
    return (rand() % (higher - lower + 1)) + lower;
}

/**
 * @brief Return random nv_float in given range.
 * 
 * @param lower Min range
 * @param higher Max range
 * @return nv_float 
 */
nv_float frand(nv_float lower, nv_float higher) {
    nv_float normal = rand() / (nv_float)RAND_MAX;
    return lower + normal * (higher - lower);
}

/**
 * @brief Return random boolean.
 * 
 * @return bool
 */
bool brand() {
    return irand(0, 1);
}

/**
 * @brief Convert color from HSV space to RGB space.
 * 
 * @param hsv HSV color
 * @return SDL_Color 
 */
SDL_Color hsv_to_rgb(SDL_Color hsv) {
    SDL_Color rgb;
    int8_t region, remainder, p, q, t;
    
    if (hsv.g == 0) {
        rgb.r = hsv.b;
        rgb.g = hsv.b;
        rgb.b = hsv.b;
        return rgb;
    }
    
    region = hsv.r / 43;
    remainder = (hsv.r - (region * 43)) * 6; 
    
    p = (hsv.b * (255 - hsv.g)) >> 8;
    q = (hsv.b * (255 - ((hsv.g * remainder) >> 8))) >> 8;
    t = (hsv.b * (255 - ((hsv.g * (255 - remainder)) >> 8))) >> 8;
    
    switch (region) {
        case 0:
            rgb.r = hsv.b; rgb.g = t; rgb.b = p;
            break;

        case 1:
            rgb.r = q; rgb.g = hsv.b; rgb.b = p;
            break;

        case 2:
            rgb.r = p; rgb.g = hsv.b; rgb.b = t;
            break;

        case 3:
            rgb.r = p; rgb.g = q; rgb.b = hsv.b;
            break;

        case 4:
            rgb.r = t; rgb.g = p; rgb.b = hsv.b;
            break;

        default:
            rgb.r = hsv.b; rgb.g = p; rgb.b = q;
            break;
    }
    
    return rgb;
}

/**
 * @brief Convert color from HSV (float) space to RGB space.
 * 
 * @param hsv HSV color
 * @return SDL_Color 
 */
SDL_Color fhsv_to_rgb(double h, double s, double v) {
    // Copied from https://stackoverflow.com/a/6930407

    double hh, p, q, t, ff;
    long i;
    SDL_Color out;

    if (s <= 0.0) { // < is bogus, just shuts up warnings
        out.r = v * 255;
        out.g = v * 255;
        out.b = v * 255;
        return out;
    }

    hh = h;
    if(hh >= 360.0) hh = 0.0;
    hh /= 60.0;
    i = (long)hh;
    ff = hh - i;
    p = v * (1.0 - s);
    q = v * (1.0 - (s * ff));
    t = v * (1.0 - (s * (1.0 - ff)));

    switch(i) {
        case 0:
            out.r = v * 255;
            out.g = t * 255;
            out.b = p * 255;
            break;

        case 1:
            out.r = q * 255;
            out.g = v * 255;
            out.b = p * 255;
            break;

        case 2:
            out.r = p * 255;
            out.g = v * 255;
            out.b = t * 255;
            break;


        case 3:
            out.r = p * 255;
            out.g = q * 255;
            out.b = v * 255;
            break;

        case 4:
            out.r = t * 255;
            out.g = p * 255;
            out.b = v * 255;
            break;
            
        case 5:
        default:
            out.r = v * 255;
            out.g = p * 255;
            out.b = q * 255;
            break;
    }

    return out; 
}

/**
 * @brief Linear interpolate between two colors.
 * 
 * @param color0 First color
 * @param color1 Second color
 * @param t Interpolation ratio
 * @return SDL_Color 
 */
SDL_Color color_lerp(SDL_Color color0, SDL_Color color1, float t) {
    SDL_Color result;

    result.r = (nv_uint8)(color0.r + t * (color1.r - color0.r));
    result.g = (nv_uint8)(color0.g + t * (color1.g - color0.g));
    result.b = (nv_uint8)(color0.b + t * (color1.b - color0.b));

    return result;
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


#define FNV_PRIME 1099511628211ULL
#define FNV_BASIS 14695981039346656037ULL

nv_uint64 FNV1a(const char *str) {
    nv_uint64 hash = FNV_BASIS;

    for (size_t i = 0; str[i] != '\0'; i++) {
        hash ^= (nv_uint64)str[i];
        hash *= FNV_PRIME;
    }

    return hash;
}

uint32_t FNV1a_u32(uint32_t value) {
    const uint32_t FNV_prime = 16777619;
    uint32_t hash = 2166136261;

    for (int i = 0; i < sizeof(uint32_t); ++i) {
        hash ^= (value & 0xFF);
        hash *= FNV_prime;
        value >>= 8;
    }

    return hash;
}

typedef struct {
    char *string;
    SDL_Texture *texture;
    nv_uint64 last_access;
} CachedText;

nv_uint64 cached_text_hash(void *item) {
    CachedText *c = (CachedText *)item;
    return FNV1a(c->string);
}


/******************************************************************************

                               Drawing functions

******************************************************************************/



/**
 * @brief Draw circle.
 * 
 * Reference: https://discourse.libsdl.org/t/query-how-do-you-draw-a-circle-in-sdl2-sdl2/33379
 * 
 * @param renderer SDL Renderer
 * @param cx Circle center X
 * @param cy Circle center Y
 * @param radius Circle radius
 */
void draw_circle(
    SDL_Renderer *renderer,
    int cx,
    int cy,
    int radius
) {
    int diameter = (radius * 2);

    int x = (radius - 1);
    int y = 0;
    int tx = 1;
    int ty = 1;
    int error = (tx - diameter);

    while (x >= y) {
        // Each of the following renders an octant of the circle
        SDL_RenderDrawPoint(renderer, cx + x, cy - y);
        SDL_RenderDrawPoint(renderer, cx + x, cy + y);
        SDL_RenderDrawPoint(renderer, cx - x, cy - y);
        SDL_RenderDrawPoint(renderer, cx - x, cy + y);
        SDL_RenderDrawPoint(renderer, cx + y, cy - x);
        SDL_RenderDrawPoint(renderer, cx + y, cy + x);
        SDL_RenderDrawPoint(renderer, cx - y, cy - x);
        SDL_RenderDrawPoint(renderer, cx - y, cy + x);

        if (error <= 0) {
            ++y;
            error += ty;
            ty += 2;
        }

        if (error > 0) {
            --x;
            tx += 2;
            error += (tx - diameter);
        }
    }
}

/**
 * @brief Fill circle.
 * 
 * @param renderer SDL Renderer
 * @param x Circle center X
 * @param y Circle center Y
 * @param radius Circle radius
 * @param color 
 */
void fill_circle(SDL_Renderer *renderer, int x, int y, int radius) {
    for (int w = 0; w < radius * 2; w++) {
        for (int h = 0; h < radius * 2; h++) {
            int dx = radius - w;
            int dy = radius - h;
            if ((dx * dx + dy * dy) <= (radius * radius))
            {
                SDL_RenderDrawPoint(renderer, x + dx, y + dy);
            }
        }
    }
}

/**
 * @brief Draw polygon.
 * 
 * @param renderer SDL Renderer
 * @param vertices Vertices
 */
void draw_polygon(SDL_Renderer *renderer, nvArray *vertices) {
    size_t n = vertices->size;

    for (size_t i = 0; i < n; i++) {
        nvVector2 va = NV_TO_VEC2(vertices->data[i]);
        nvVector2 vb = NV_TO_VEC2(vertices->data[(i + 1) % n]);

        SDL_RenderDrawLineF(
            renderer,
            va.x, va.y,
            vb.x, vb.y
        );
    }
}

/**
 * Utility functions for anti-aliased functions
 */

static inline void _aa_swap(nv_float *a, nv_float *b) {
    nv_float temp = *a;
    *a = *b;
    *b = temp;
}

static inline int _aa_ipart(nv_float x) {
    return (int)x;
}

static inline int _aa_fround(nv_float x) {
    return _aa_ipart(x + 0.5);
}

static inline nv_float _aa_fpart(nv_float x) {
    return x - _aa_ipart(x);
}

static inline nv_float _aa_rfpart(nv_float x) {
    return 1.0 - _aa_fpart(x);
}

static inline void _aa_pixel(
    SDL_Renderer *renderer,
    nv_float x,
    nv_float y,
    nv_float a,
    int r,
    int g,
    int b
) {
    SDL_SetRenderDrawColor(renderer, r, g, b, (nv_uint8)(a * 255));
    SDL_RenderDrawPointF(renderer, x, y);
}

static inline void _aa_pixel4(
    SDL_Renderer *renderer,
    nv_float x,
    nv_float y,
    nv_float dx,
    nv_float dy,
    nv_float alpha,
    nv_uint8 r,
    nv_uint8 g,
    nv_uint8 b
) {
    SDL_SetRenderDrawColor(renderer, r, g, b, (nv_uint8)alpha);
    SDL_RenderDrawPointF(renderer, x + dx, y + dy);
    SDL_RenderDrawPointF(renderer, x - dx, y + dy);
    SDL_RenderDrawPointF(renderer, x + dx, y - dy);
    SDL_RenderDrawPointF(renderer, x - dx, y - dy);
}

/**
 * @brief Draw anti-aliased line.
 * 
 * Reference: https://en.wikipedia.org/wiki/Xiaolin_Wu%27s_line_algorithm
 * 
 * @param renderer SDL Renderer
 * @param x0 Starting point X
 * @param y0 Starting point Y
 * @param x1 End point X
 * @param y1 End point Y
 */
void draw_aaline(
    SDL_Renderer *renderer,
    nv_float x0,
    nv_float y0,
    nv_float x1,
    nv_float y1
) {
    bool steep = nv_fabs(y1 - y0) > nv_fabs(x1 - x0);
    
    nv_uint8 r, g, b, a;
    SDL_GetRenderDrawColor(renderer, &r, &g, &b, &a);

    if (steep) {
        _aa_swap(&x0, &y0);
        _aa_swap(&x1, &y1);
    }
    if (x0 > x1) {
        _aa_swap(&x0, &x1);
        _aa_swap(&y0, &y1);
    }

    nv_float dx = x1 - x0;
    nv_float dy = y1 - y0;

    nv_float gradient;
    if (dx == 0.0) gradient = 1.0;
    else gradient = dy / dx;

    // Handle first endpoint
    int xend = _aa_fround(x0);
    nv_float yend = y0 + gradient * (xend - x0);
    nv_float xgap = _aa_rfpart(x0 + 0.5);
    int xpxl1 = xend; // For main loop
    int ypxl1 = _aa_ipart(yend);

    if (steep) {
        _aa_pixel(renderer, ypxl1,     xpxl1, _aa_rfpart(yend) * xgap, r, g, b);
        _aa_pixel(renderer, ypxl1 + 1, xpxl1,  _aa_fpart(yend) * xgap, r, g, b);
    }
    else {
        _aa_pixel(renderer, xpxl1, ypxl1,     _aa_rfpart(yend) * xgap, r, g, b);
        _aa_pixel(renderer, xpxl1, ypxl1 + 1,  _aa_fpart(yend) * xgap, r, g, b);
    }

    nv_float intery = yend + gradient; // First Y intersection

    // Handle second endpoint
    xend = _aa_fround(x1);
    yend = y1 + gradient * (xend - x1);
    xgap = _aa_fpart(x1 + 0.5);
    int xpxl2 = xend; // For main loop
    int ypxl2 = _aa_ipart(yend);

    if (steep) {
        _aa_pixel(renderer, ypxl2,     xpxl2, _aa_rfpart(yend) * xgap, r, g, b);
        _aa_pixel(renderer, ypxl2 + 1, xpxl2,  _aa_fpart(yend) * xgap, r, g, b);
    }
    else {
        _aa_pixel(renderer, xpxl2, ypxl2,     _aa_rfpart(yend) * xgap, r, g, b);
        _aa_pixel(renderer, xpxl2, ypxl2 + 1,  _aa_fpart(yend) * xgap, r, g, b);
    }

    // Main loop
    if (steep) {
        for (int x = xpxl1 + 1; x <= xpxl2 - 1; x++) {
            _aa_pixel(renderer, _aa_ipart(intery),     x, _aa_rfpart(intery), r, g, b);
            _aa_pixel(renderer, _aa_ipart(intery) + 1, x,  _aa_fpart(intery), r, g, b);
            intery += gradient;
        }
    }
    else {
        for (int x = xpxl1 + 1; x <= xpxl2 - 1; x++) {
            _aa_pixel(renderer, x, _aa_ipart(intery),     _aa_rfpart(intery), r, g, b);
            _aa_pixel(renderer, x, _aa_ipart(intery) + 1,  _aa_fpart(intery), r, g, b);
            intery += gradient;
        }
    }
}

/**
 * @brief Draw anti-aliased polygon.
 * 
 * @param renderer SDL Renderer
 * @param vertices Vertices
 */
void draw_aapolygon(SDL_Renderer *renderer, nvArray *vertices) {
    size_t n = vertices->size;

    for (size_t i = 0; i < n; i++) {
        nvVector2 va = NV_TO_VEC2(vertices->data[i]);
        nvVector2 vb = NV_TO_VEC2(vertices->data[(i + 1) % n]);

        draw_aaline(
            renderer,
            va.x, va.y,
            vb.x, vb.y
        );
    }
}

/**
 * @brief Draw anti-aliased circle
 * 
 * Reference: https://create.stephan-brumme.com/antialiased-circle/#antialiased-circle-wu
 * 
 * @param renderer SDL Renderer
 * @param cx Circle center X
 * @param cy Circle center Y
 * @param radius Circle radius
 * @param r Color R
 * @param g Color G
 * @param b Color B
 */
void draw_aacircle(
    SDL_Renderer *renderer,
    nv_float cx,
    nv_float cy,
    nv_float radius,
    nv_uint8 r,
    nv_uint8 g,
    nv_uint8 b
) {
    // + 0.3 is for arbitrary correction
    nv_float rx = radius + 0.3;
    nv_float ry = radius + 0.3;
    nv_float rx2 = rx * rx;
    nv_float ry2 = ry * ry;

    nv_float max_alpha = 255.0;

    nv_float q = _aa_fround(rx2 / nv_sqrt(rx2 + ry2));
    for (nv_float x = 0; x <= q; x++) {
        nv_float y = ry * nv_sqrt(1 - x * x / rx2);
        nv_float error = y - floor(y);

        nv_float alpha = _aa_fround(error * max_alpha);
        
        _aa_pixel4(renderer, cx, cy, x, floor(y),     alpha,             r, g, b);
        _aa_pixel4(renderer, cx, cy, x, floor(y) - 1, max_alpha - alpha, r, g, b);
    }

    q = _aa_fround(ry2 / nv_sqrt(rx2 + ry2));
    for (nv_float y = 0; y <= q; y++) {
        nv_float x = rx * nv_sqrt(1 - y * y / ry2);
        nv_float error = x - floor(x);

        nv_float alpha = _aa_fround(error * max_alpha);

        _aa_pixel4(renderer, cx, cy, floor(x),     y, alpha,             r, g, b);
        _aa_pixel4(renderer, cx, cy, floor(x) - 1, y, max_alpha - alpha, r, g, b);
    }
}

void draw_dashed_line(
    SDL_Renderer *renderer,
    int x0,
    int y0,
    int x1,
    int y1,
    int dash_length,
    int gap_length
) {
    nvVector2 start = {x0, y0};
    nvVector2 end = {x1, y1};
    nvVector2 delta = nvVector2_sub(end, start);
    double dist = nvVector2_len(delta);
    nvVector2 dir = nvVector2_normalize(delta);

    double step = 0.0;
    while (step < dist) {
        nvVector2 step_start = nvVector2_add(start, nvVector2_mul(dir, step));
        nvVector2 step_end = nvVector2_add(start, nvVector2_mul(dir, step + dash_length));
        SDL_RenderDrawLine(renderer, step_start.x, step_start.y, step_end.x, step_end.y);
        step += dash_length + gap_length;
    }
}



/******************************************************************************

                            Example & helper structs

******************************************************************************/



/**
 * @brief Mouse information struct.
 */
typedef struct {
    int x; /**< X coordinate of mouse. */
    int y; /**< Y coordinate of mouse. */

    nv_float px; /**< X coordinate of mouse in physics space. */
    nv_float py; /**< Y coordinate of mouse in physics space. */

    nvVector2 before_zoom;
    nvVector2 after_zoom;

    bool left; /**< Is left button pressed? */
    bool middle; /**< Is wheel pressed? */
    bool right; /**< Is right button pressed? */
} Mouse;


/**
 * @brief Toggle switch UI element.
 */
typedef struct {
    int x; /**< X coordinate. */
    int y; /**< Y coordinate. */
    int size; /**< Size of the toggle switch in height. */
    bool on; /**< Whether the switch is toggled or not. */
    bool changed; /**< Internal flag to track state change. */
} ToggleSwitch;


typedef enum {
    SliderType_INTEGER,
    SliderType_FLOAT
} SliderType;

/**
 * @brief Slider UI element.
 */
typedef struct {
    int x;
    int cx;
    int y;
    int width;
    nv_float value;
    nv_float max;
    nv_float min;
    bool pressed;
    SliderType type;
} Slider;


typedef struct {
    int x;
    int y;
    int width;
    int height;
    char *text;
    bool hovered;
    bool pressed;
    void ( *callback)(void *);
} Button;


/**
 * @brief Example visual theme enum.
 */
typedef enum {
    ExampleTheme_LIGHT, /**< Light theme. */
    ExampleTheme_DARK /**< Dark theme. */
} ExampleTheme;


// Example callback type
typedef void ( *Example_callback)(struct _Example *example);


// ToggleSwitch_update forward declaration
void ToggleSwitch_update(struct _Example *example, ToggleSwitch *tg);

// ToggleSwitch_draw forward declaration
void ToggleSwitch_draw(struct _Example *example, ToggleSwitch *tg);


// Slider_update forward declaration
void Slider_update(struct _Example *example, Slider *s);

// Slider_draw forward declaration
void Slider_draw(struct _Example *example, Slider *s);


// Button_update forward declaration
void Button_update(struct _Example *example, Button *b);

// Button_draw forward declaration
void Button_draw(struct _Example *example, Button *b, TTF_Font *font);


typedef struct {
    double percent;
    size_t index;
} GraphData;


/**
 * @brief Example base struct.
 */
struct _Example {
    int width; /**< Window width. */
    int height; /**< Window height. */
    SDL_Window *window; /**< SDL Window instance. */
    SDL_Renderer *renderer; /**< SDL Renderer instance. */
    SDL_Texture *texture; /**< SDL Texture instance. */
    
    Mouse mouse; /**< Mouse information struct. */
    const nv_uint8 *keys; /**< Array of pressed keys. */

    nv_float max_fps; /**< Targe FPS. */
    nv_float fps; /**< Current FPS. */
    nv_float dt; /**< Delta-time. */

    nvSpace *space; /**< Nova Physics space instance. */
    nv_float hertz; /**< Simulation hertz. */

    bool selected;

    size_t switch_count; /**< Count of toggle switches. */
    ToggleSwitch **switches; /**< Array of toggle switches. */

    size_t slider_count; /**< Count of sliders. */
    Slider **sliders; /**< Array of sliders. */

    size_t button_count;
    Button **buttons;

    // Theme colors
    SDL_Color bg_color;
    SDL_Color text_color;
    SDL_Color alt_text_color;
    SDL_Color body_color;
    SDL_Color static_color;
    SDL_Color sleep_color;
    SDL_Color spring_color;
    SDL_Color distancejoint_color;
    SDL_Color hingejoint_color;
    SDL_Color aabb_color;
    SDL_Color ui_color;
    SDL_Color ui_color2;
    SDL_Color velocity_color;

    SDL_Color *profiler_palette;

    bool draw_ui; /**< Whether to draw the UI or not. */

    // Profiling stats
    nv_float step_time;
    nv_float step_counter;
    nv_float step_avg;
    nv_float render_time;
    nv_float render_counter;
    nv_float render_avg;
    nv_float total_energy;
    nv_float total_le;
    nv_float total_ae;
    int counter;

    int cloth_example_cols;
    int cloth_example_rows;

    nvHashMap *cached_texts;

    nvVector2 camera;
    nv_float camera_speed;
    nv_float zoom;
    nv_float zoom_scale;
    nvVector2 pan_start;

    GraphData *last_graph;
    size_t graph_counter;
    double max_memory_usage;

    double *fps_graph_data;
    double *memory_graph_data;
    size_t fps_graph_size;
    size_t memory_graph_size;
};

typedef struct _Example Example;


typedef void ( *Example_callback)(Example *);


typedef struct {
    char *name;
    nvArray *slider_settings;
    Example_callback init_callback;
    Example_callback setup_callback;
    Example_callback update_callback;
} ExampleEntry;

typedef struct {
    char *name;
    Slider *slider;
} SliderSetting;

void add_slider_setting(
    ExampleEntry *entry,
    char *setting,
    SliderType type,
    nv_float value,
    nv_float min,
    nv_float max
) {
    // TODO: Free setting and slider

    SliderSetting *slider_setting = NV_NEW(SliderSetting);
    if (!slider_setting) NV_ERROR("Memory error at add_slider_setting");

    slider_setting->name = setting;

    Slider *slider = NV_NEW(Slider);
    if (!slider) NV_ERROR("Memory error at add_slider_setting");

    slider->x = 0;
    slider->y = 0;
    slider->width = 80;
    slider->value = value;
    slider->min = min;
    slider->max = max;
    slider->type = type;
    slider->cx = slider->x + ((slider->value-slider->min) / (slider->max - slider->min)) * slider->width;
    slider->pressed = false;

    slider_setting->slider = slider;

    nvArray_add(entry->slider_settings, slider_setting);
}

extern ExampleEntry example_entries[100];
extern size_t example_count;
size_t current_example;

nv_float get_slider_setting(char *name) {
    ExampleEntry entry = example_entries[current_example];

    for (size_t i = 0; i < entry.slider_settings->size; i++) {
        SliderSetting *setting = entry.slider_settings->data[i];
        if (!strcmp(name, setting->name)) {
            return setting->slider->value;
        }
    }

    return 0.0;
}


// Transform position from world space to screen space
nvVector2 world_to_screen(Example *example, nvVector2 world_pos) {
    return nvVector2_mul(nvVector2_sub(world_pos, example->camera), example->zoom);
}

// Transform position from screen space to world space
nvVector2 screen_to_world(Example *example, nvVector2 screen_pos) {
    return nvVector2_add(nvVector2_div(screen_pos, example->zoom), example->camera);
}


/**
 * Contact drawer callback
 */
static void after_callback(nvHashMap *res_arr, void *user_data) {
    Example *example = (Example *)user_data;

    if (!example->switches[2]->on) return;

    size_t iter = 0;
    void *item;
    while (nvHashMap_iter(res_arr, &iter, &item)) {

        nvResolution *res = item;

        nv_float radius = 0.1 * example->zoom;

        nvVector2 cp;
        SDL_Color color;

        if (res->contact_count == 1) {
            nvContact contact = res->contacts[0];
            cp = world_to_screen(example, contact.position);

            if (!nv_collide_aabb_x_point((nvAABB){0.0, 0.0, example->width, example->height}, cp)) {
                continue;
            }

            if (
                nvVector2_dist2(
                    NV_VEC2(example->mouse.x, example->mouse.y),
                    cp
                ) < 5 * 5
            ) {
                color = (SDL_Color){181, 242, 75, 255};

                if (example->mouse.right) {
                    nv_print_Resolution(res);

                    nvVector2 a = world_to_screen(example, res->a->position);
                    nvVector2 b = world_to_screen(example, res->b->position);

                    draw_aacircle(example->renderer, a.x, a.y, 0.3 * example->zoom, color.r, color.g, color.b);
                    draw_aacircle(example->renderer, b.x, b.y, 0.3 * example->zoom, color.r, color.g, color.b);
                }

            } else {
                color = (SDL_Color){242, 75, 81, 255};
            }

            if (res->state == 2) {
                color = (SDL_Color){227, 208, 98, 255};
            }

            draw_aacircle(example->renderer, cp.x, cp.y, radius, color.r, color.g, color.b);
        }

        else if (res->contact_count == 2) {
            nvContact contact1 = res->contacts[0];
            nvContact contact2 = res->contacts[1];

            nvVector2 c1 = world_to_screen(example, contact1.position);
            nvVector2 c2 = world_to_screen(example, contact2.position);

            if (nv_collide_aabb_x_point((nvAABB){0.0, 0.0, example->width, example->height}, c1)) {
                if (
                    nvVector2_dist2(
                        NV_VEC2(example->mouse.x, example->mouse.y),
                        c1
                    ) < 10 * 10
                ) {
                    if (example->mouse.right) {nv_print_Resolution(res);}
                    color = (SDL_Color){181, 242, 75, 255};

                } else {
                    color = (SDL_Color){242, 75, 81, 255};
                }

                if (res->state == 2) {
                    color = (SDL_Color){227, 208, 98, 255};
                }

                draw_aacircle(example->renderer, c1.x, c1.y, radius, color.r, color.g, color.b);
            }

            if (nv_collide_aabb_x_point((nvAABB){0.0, 0.0, example->width, example->height}, c2)) {
                if (
                    nvVector2_dist2(
                        NV_VEC2(example->mouse.x, example->mouse.y),
                        c2
                    ) < 10 * 10
                ) {
                    if (example->mouse.right) {nv_print_Resolution(res);}
                    color = (SDL_Color){181, 242, 75, 255};

                } else {
                    color = (SDL_Color){242, 75, 81, 255};
                }

                if (res->state == 2) {
                    color = (SDL_Color){227, 208, 98, 255};
                }

                draw_aacircle(example->renderer, c2.x, c2.y, radius, color.r, color.g, color.b);
            }
        }
    }
}

/**
 * @brief Create new example instance.
 * 
 * @param width Window width
 * @param height Window height
 * @param title Window title
 * @param max_fps Window max FPS
 * @param hertz Simulation hertz
 * @return Example 
 */
Example *Example_new(
    int width,
    int height,
    char *title,
    nv_float max_fps,
    nv_float hertz,
    ExampleTheme theme
) {
    Example *example = NV_NEW(Example);
    if (!example) NV_ERROR("Couldn't initialize example.\n");

    // Initialize SDL2 and extensions

    if (SDL_Init(SDL_INIT_EVERYTHING) != 0) {
        printf("SDL2 could not be initialized. Error: %s\n", SDL_GetError());
        exit(1);
    }

    if (TTF_Init() != 0) {
        printf("SDL2_ttf could not be initialized. Error: %s\n", TTF_GetError());
        exit(1);
    }

    // Enable linear filtering for textures
    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "1");

    example->width = width;
    example->height = height;

    example->window = SDL_CreateWindow(
        title,
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        width, height,
        SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE
    );

    SDL_SetWindowMinimumSize(example->window, 1280, 720);

    example->renderer = SDL_CreateRenderer(
        example->window, -1, SDL_RENDERER_ACCELERATED);

    example->texture = SDL_CreateTexture(
        example->renderer,
        SDL_PIXELFORMAT_ARGB8888,
        SDL_TEXTUREACCESS_STREAMING,
        width, height
    );

    // For anti-aliased drawing functions
    SDL_SetRenderDrawBlendMode(example->renderer, SDL_BLENDMODE_BLEND);

    SDL_Surface *window_icon = SDL_LoadBMP("assets/novaicon.bmp");
    SDL_SetWindowIcon(example->window, window_icon);
    SDL_FreeSurface(window_icon);

    example->mouse = (Mouse){0, 0, 0.0, 0.0, nvVector2_zero, nvVector2_zero, false, false, false};
    example->keys = SDL_GetKeyboardState(NULL);

    example->max_fps = max_fps;
    example->fps = max_fps;
    example->dt = 1.0 / max_fps;

    example->space = nvSpace_new();
    example->hertz = hertz;

    example->selected = false;

    example->space->callback_user_data = example;
    example->space->after_collision = after_callback;

    // Light theme
    if (theme == ExampleTheme_LIGHT) {
        example->bg_color = (SDL_Color){255, 255, 255, 255};
        example->text_color = (SDL_Color){0, 0, 0, 255};
        example->alt_text_color = (SDL_Color){90, 90, 96, 255};
        example->body_color = (SDL_Color){40, 40, 44, 255};
        example->static_color = (SDL_Color){123, 124, 138, 255};
        example->sleep_color = (SDL_Color){176, 132, 77, 255};
        example->spring_color = (SDL_Color){56, 255, 169, 255};
        example->distancejoint_color = (SDL_Color){74, 201, 255, 255};
        example->hingejoint_color = (SDL_Color){140, 106, 235, 255};
        example->aabb_color = (SDL_Color){252, 127, 73, 255};
        example->ui_color = (SDL_Color){97, 197, 255, 255};
        example->ui_color2 = (SDL_Color){255, 255, 255, 255};
        example->velocity_color = (SDL_Color){169, 237, 43, 255};
    }
    // Dark theme
    else if (theme == ExampleTheme_DARK) {
        example->bg_color = (SDL_Color){32, 32, 36, 255};
        example->text_color = (SDL_Color){255, 255, 255, 255};
        example->alt_text_color = (SDL_Color){153, 167, 191, 255};
        example->body_color = (SDL_Color){237, 244, 255, 255};
        example->static_color = (SDL_Color){116, 126, 143, 255};
        example->sleep_color = (SDL_Color){227, 196, 157, 255};
        example->spring_color = (SDL_Color){56, 255, 169, 255};
        example->distancejoint_color = (SDL_Color){74, 201, 255, 255};
        example->hingejoint_color = (SDL_Color){140, 106, 235, 255};
        example->aabb_color = (SDL_Color){252, 127, 73, 255};
        example->ui_color = (SDL_Color){66, 164, 245, 255};
        example->ui_color2 = (SDL_Color){0, 0, 0, 255};
        example->velocity_color = (SDL_Color){197, 255, 71, 255};
    }

    example->profiler_palette = malloc(sizeof(SDL_Color) * 11);

    example->profiler_palette[0] = (SDL_Color){0, 22, 134};
    example->profiler_palette[1] = (SDL_Color){94, 43, 255};
    example->profiler_palette[2] = (SDL_Color){200, 0, 255};
    example->profiler_palette[3] = (SDL_Color){238, 47, 123};
    example->profiler_palette[4] = (SDL_Color){195, 33, 0};
    example->profiler_palette[5] = (SDL_Color){255, 145, 0};
    example->profiler_palette[6] = (SDL_Color){255, 243, 110};
    example->profiler_palette[7] = (SDL_Color){124, 228, 39};
    example->profiler_palette[8] = (SDL_Color){11, 146, 121};
    example->profiler_palette[9] = (SDL_Color){66, 164, 245};
    example->profiler_palette[10] = (SDL_Color){255, 255, 213};

    // Profiling stats
    example->step_time = 0.0;
    example->render_time = 0.0;
    example->step_avg = 0.0;
    example->render_avg = 0.0;
    example->step_counter = 0.0;
    example->render_counter = 0.0;
    example->total_ae = 0.0;
    example->total_energy = 0.0;
    example->total_le = 0.0;
    example->counter = 0;

    example->draw_ui = true;

    example->cloth_example_cols = 0;
    example->cloth_example_rows = 0;

    example->cached_texts = nvHashMap_new(sizeof(CachedText), 0, cached_text_hash);

    example->camera = nvVector2_zero;//NV_VEC2(1280.0 / 20.0, 720.0 / 20.0);
    example->camera_speed = 75.0;
    example->zoom = 10.0;
    example->zoom_scale = 0.075;
    example->pan_start = nvVector2_zero;

    example->last_graph = malloc(sizeof(GraphData) * 11);
    example->graph_counter = 0;

    for (size_t j = 0; j < 11; j++) {
        example->last_graph[j] = (GraphData){0.0, 0};
    }

    example->max_memory_usage = 0.0;

    example->fps_graph_data = calloc(240, sizeof(double) * 240);
    example->memory_graph_data = calloc(240, sizeof(double) * 240);
    example->fps_graph_size = 0;
    example->memory_graph_size = 0;

    return example;
}

/**
 * @brief Free space allocated by example.
 * 
 * @param example Example to free
 */
void Example_free(Example *example) {
    SDL_DestroyRenderer(example->renderer);
    SDL_DestroyWindow(example->window);
    nvSpace_free(example->space);

    free(example->sliders);
    free(example->switches);

    for (size_t i = 0; i < example_count; i++) {
        ExampleEntry entry = example_entries[i];
        
        for (size_t j = 0; j < entry.slider_settings->size; j++) {
            SliderSetting *setting = entry.slider_settings->data[j];
            Slider *slider = setting->slider;

            free(setting);
            free(slider);
        }

        nvArray_free(entry.slider_settings);
    }

    size_t l = 0;
    void *map_val;
    while (nvHashMap_iter(example->cached_texts, &l, &map_val)) {
        CachedText *cached_text = map_val;
        SDL_DestroyTexture(cached_text->texture);
        free(cached_text->string);
    }
    nvHashMap_free(example->cached_texts);

    free(example->profiler_palette);
    free(example->last_graph);

    free(example->fps_graph_data);
    free(example->memory_graph_data);

    free(example);
}



/******************************************************************************

                             Main loop functions

******************************************************************************/


/* We have to move some drawing functions here because they have Example as arg. */

/**
 * @brief Draw spring.
 * 
 * @param example Example
 * @param renderer SDL Renderer
 * @param cons Constraint
 * @param aa Anit-aliasing
 * @param color Color
 */
void draw_spring(
    struct _Example *example,
    SDL_Renderer *renderer,
    nvConstraint *cons,
    bool aa,
    SDL_Color color
) {
    nvSpring *spring = (nvSpring *)cons->def;

    nvVector2 ap;
    nvVector2 bp;

    // Transform anchor and body positions
    if (cons->a == NULL) {
        ap = world_to_screen(example, spring->anchor_a);
    } else {
        nvVector2 ra = nvVector2_rotate(spring->anchor_a, cons->a->angle);
        ap = nvVector2_add(cons->a->position, ra);
        ap = world_to_screen(example, ap);
    }
    if (cons->b == NULL) {
        bp = world_to_screen(example, spring->anchor_b);
    } else {
        nvVector2 rb = nvVector2_rotate(spring->anchor_b, cons->b->angle);
        bp = nvVector2_add(cons->b->position, rb);
        bp = world_to_screen(example, bp);
    }

    nvVector2 delta = nvVector2_sub(bp, ap);
    nvVector2 dir = nvVector2_normalize(delta);
    nv_float dist = nvVector2_len(delta);
    nv_float offset = (dist - spring->length * example->zoom) / (spring->length * example->zoom);
    nv_float steps = NV_PI / 3.0;
    nv_float stretch = 1.0 + offset;

    if (aa) {
        draw_aacircle(
            renderer,
            ap.x, ap.y,
            0.2 * example->zoom,
            color.r, color.g, color.b
        );

        draw_aacircle(
            renderer,
            bp.x, bp.y,
            0.2 * example->zoom,
            color.r, color.g, color.b
        );
    }
    else {
        draw_circle(
            renderer,
            ap.x, ap.y,
            0.2 * example->zoom
        );

        draw_circle(
            renderer,
            bp.x, bp.y,
            0.2 * example->zoom
        );
    }

    nvVector2 s = nvVector2_zero;
    nvVector2 e = nvVector2_zero;

    for (nv_float step = 0.0; step < dist; step += steps) {
        nv_float next_step = step + steps;

        nv_float w = ((spring->length / 1.25) - offset);
        if (w < 0.0) w = 0.0;

        s = nvVector2_mul(dir, step);
        s = nvVector2_add(s, nvVector2_mul(nvVector2_perp(dir), sin(step / stretch) * w));
        e = nvVector2_mul(dir, next_step);
        e = nvVector2_add(e, nvVector2_mul(nvVector2_perp(dir), sin(next_step / stretch) * w));

        if (aa)
            draw_aaline(renderer, ap.x + s.x, ap.y + s.y, ap.x + e.x, ap.y + e.y);
        else
            SDL_RenderDrawLine(renderer, ap.x + s.x, ap.y + s.y, ap.x + e.x, ap.y + e.y);
    }
}

/**
 * @brief Draw text.
 * 
 * @param font TTF Font
 * @param renderer SDL Renderer
 * @param text Text
 * @param x X
 * @param y Y
 * @param color SDL Color
 */
void draw_text(
    struct _Example *example,
    TTF_Font *font,
    SDL_Renderer *renderer,
    char *text,
    int x,
    int y,
    SDL_Color color
) {
    CachedText *cached_text = nvHashMap_get(example->cached_texts, &(CachedText){.string=text});
    SDL_Texture *text_tex;

    if (cached_text) {
        text_tex = cached_text->texture;
        nvHashMap_set(example->cached_texts, &(CachedText){.string=cached_text->string, .texture=text_tex, .last_access=time(NULL)});
    }

    else {
        SDL_Surface *text_surf = TTF_RenderText_Blended(font, text, color);
        text_tex = SDL_CreateTextureFromSurface(renderer, text_surf);
        SDL_FreeSurface(text_surf);

        char *text_h = strdup(text);
        nvHashMap_set(example->cached_texts, &(CachedText){.string=text_h, .texture=text_tex, .last_access=time(NULL)});
    }

    int width, height;
    SDL_QueryTexture(text_tex, NULL, NULL, &width, &height);

    SDL_Rect text_rect = {x, y, width, height};

    SDL_RenderCopy(renderer, text_tex, NULL, &text_rect);
}

/**
 * @brief Draw text aligned to right.
 * 
 * @param font TTF Font
 * @param renderer SDL Renderer
 * @param text Text
 * @param x X
 * @param y Y
 * @param color SDL Color
 */
void draw_text_from_right(
    struct _Example *example,
    TTF_Font *font,
    SDL_Renderer *renderer,
    char *text,
    int x,
    int y,
    SDL_Color color
) {
    CachedText *cached_text = nvHashMap_get(example->cached_texts, &(CachedText){.string=text});
    SDL_Texture *text_tex;

    if (cached_text) {
        text_tex = cached_text->texture;
        nvHashMap_set(example->cached_texts, &(CachedText){.string=cached_text->string, .texture=text_tex, .last_access=time(NULL)});
    }

    else {
        SDL_Surface *text_surf = TTF_RenderText_Blended(font, text, color);
        text_tex = SDL_CreateTextureFromSurface(renderer, text_surf);
        SDL_FreeSurface(text_surf);

        char *text_h = strdup(text);
        nvHashMap_set(example->cached_texts, &(CachedText){.string=text_h, .texture=text_tex, .last_access=time(NULL)});
    }

    int width, height;
    SDL_QueryTexture(text_tex, NULL, NULL, &width, &height);

    SDL_Rect text_rect = {example->width - width - x, y, width, height};

    SDL_RenderCopy(renderer, text_tex, NULL, &text_rect);
}

void draw_text_middle(
    struct _Example *example,
    TTF_Font *font,
    SDL_Renderer *renderer,
    char *text,
    int x,
    int y,
    int width,
    int height,
    SDL_Color color
) {
    CachedText *cached_text = nvHashMap_get(example->cached_texts, &(CachedText){.string=text});
    SDL_Texture *text_tex;

    if (cached_text) {
        text_tex = cached_text->texture;
        nvHashMap_set(example->cached_texts, &(CachedText){.string=cached_text->string, .texture=text_tex, .last_access=time(NULL)});
    }

    else {
        SDL_Surface *text_surf = TTF_RenderText_Blended(font, text, color);
        text_tex = SDL_CreateTextureFromSurface(renderer, text_surf);
        SDL_FreeSurface(text_surf);

        char *text_h = strdup(text);
        nvHashMap_set(example->cached_texts, &(CachedText){.string=text_h, .texture=text_tex, .last_access=time(NULL)});
    }

    int twidth, theight;
    SDL_QueryTexture(text_tex, NULL, NULL, &twidth, &theight);

    SDL_Rect text_rect = {x + width/2.0-twidth/2.0, y + height/2.0-theight/2.0, twidth, theight};

    SDL_RenderCopy(renderer, text_tex, NULL, &text_rect);
}


static int graph_cmp(const void *a, const void *b) {
    //return (*(GraphData *)b).percent - (*(GraphData *)a).percent;

    double value_a = (*(GraphData *)a).percent;
    double value_b = (*(GraphData *)b).percent;

    if (value_a < value_b) return 1;
    if (value_a > value_b) return -1;
    return 0;
}


/**
 * @brief Render UI.
 */
void draw_ui(Example *example, TTF_Font *font) {
    int example_ui_y = 200;
    int example_ui_x = example->width - 250;

    if (example->draw_ui) {
        SDL_SetRenderDrawColor(example->renderer, example->ui_color2.r, example->ui_color2.g, example->ui_color2.b, 175);
        SDL_RenderFillRect(example->renderer, &(SDL_Rect){0, 0, 250, example->height});
        SDL_RenderFillRect(example->renderer, &(SDL_Rect){example_ui_x, example_ui_y, 250, 250});
    }
    else {
        SDL_SetRenderDrawColor(example->renderer, example->ui_color2.r, example->ui_color2.g, example->ui_color2.b, 175);
        SDL_RenderFillRect(example->renderer, &(SDL_Rect){0, 0, 220, 56});
    }

    // font size + 4 px for leading
    int y_gap = 12 + 4;

    //char *text_fps = malloc(sizeof(char) * 32);
    char text_fps[32];
    sprintf(text_fps, "FPS: %.1f", example->fps);

    char text_steptime[32];
    sprintf(text_steptime, "Physics: %.2fms", example->step_time);

    char text_rendertime[32];
    sprintf(text_rendertime, "Render: %.2fms", example->render_time);

    draw_text(example, font, example->renderer, text_fps, 5, 5 + (y_gap*0), example->text_color);
    draw_text(example, font, example->renderer, text_steptime, 5, 5 + (y_gap*1), example->text_color);
    draw_text(example, font, example->renderer, text_rendertime, 5, 5 + (y_gap*2), example->text_color);

    if (!example->draw_ui) {
        char text_savg[24];
        sprintf(text_savg, "Avg: %.2fms", example->step_avg);

        char text_ravg[24];
        sprintf(text_ravg, "Avg: %.2fms", example->render_avg);

        draw_text(example, font, example->renderer, text_savg, 120, 5 + (y_gap*1), example->text_color);
        draw_text(example, font, example->renderer, text_ravg, 120, 5 + (y_gap*2), example->text_color);

        return;
    }

    char text_memoryload[32];
    size_t memory_used = get_current_memory_usage();
    double memory_used_mb = (double)memory_used / 1048576.0;
    if (memory_used_mb > example->max_memory_usage) example->max_memory_usage = memory_used_mb;
    sprintf(text_memoryload, "Memory: %.1fMB", memory_used_mb);

    char text_threads[32];
    sprintf(text_threads, "Threads: %llu", (unsigned long long)example->space->thread_count);

    draw_text(example, font, example->renderer, text_memoryload, 5, 5 + (y_gap*3), example->text_color);
    draw_text(example, font, example->renderer, text_threads, 5, 5 + (y_gap*4), example->text_color);


    char example_title[48];
    sprintf(example_title, "%s example settings", example_entries[current_example].name);
    draw_text(example, font, example->renderer, example_title, example_ui_x + 5, example_ui_y + 5, example->text_color);

    SDL_SetRenderDrawColor(
        example->renderer,
        example->alt_text_color.r,
        example->alt_text_color.g,
        example->alt_text_color.b,
        255
    );
    SDL_RenderDrawLine(
        example->renderer,
        example_ui_x + 5, example_ui_y + 5 + 16 + 2,
        example->width - 5, example_ui_y + 5 + 16 + 2
    );

    ExampleEntry entry = example_entries[current_example];

    for (size_t i = 0; i < entry.slider_settings->size; i++) {
        SliderSetting *setting = entry.slider_settings->data[i];
        Slider *s = setting->slider;

        int slider_x = example_ui_x + 100;

        s->x = slider_x;
        s->y = example_ui_y + 67 + 16 * i + 5;
        s->cx = s->x + ((s->value-s->min) / (s->max - s->min)) * s->width;

        Slider_update(example, s);
        Slider_draw(example, s);

        draw_text(example, 
            font,
            example->renderer,
            setting->name,
            example_ui_x + 5,
            example_ui_y + 67 + 16 * i,
            example->text_color
        );

        char slider_val[8];
        if (s->type == SliderType_FLOAT)
            sprintf(slider_val, "%.3f", s->value);
        else
            sprintf(slider_val, "%d", (int)s->value);

        draw_text(example, 
            font,
            example->renderer,
            slider_val,
            slider_x + s->width + 5,
            example_ui_y + 67 + 16 * i,
            example->text_color
        );
    }


    struct SDL_version sdl_ver;
    SDL_GetVersion(&sdl_ver);
    char text_sdlver[32];
    sprintf(text_sdlver, "SDL %d.%d.%d", sdl_ver.major, sdl_ver.minor, sdl_ver.patch);

    char text_novaver[32];
    sprintf(text_novaver, "Nova Physics %s", NV_VERSTR);

    char *text_instr = "Click & drag bodies";
    char *text_instr1 = "Reset scene with [R]";
    char *text_instr2 = "Create explosion with [Q]";
    char *text_instr3 = "Toggle UI with [U]";
    char *text_instr4 = "Toggle pause with [PERIOD]";
    char *text_instr5 = "Step by step with [SLASH]";

    char text_bodies[32];
    sprintf(text_bodies, "Bodies: %llu", (unsigned long long)example->space->bodies->size);

    char text_consts[32];
    sprintf(text_consts, "Constraints: %llu", (unsigned long long)example->space->constraints->size);

    char text_attrs[32];
    sprintf(text_attrs, "Attractors: %llu", (unsigned long long)example->space->attractors->size);

    char text_ress[32];
    sprintf(text_ress, "Resolutions: %llu", (unsigned long long)example->space->res->count);

    char *text_iters =   "Velocity iters";
    char *text_citers =  "Position iters";
    char *text_cciters = "Constrt. iters";
    char *text_subs =    "Substeps";
    char *text_hertz =   "Hertz";

    char text_iters_f[16];
    sprintf(text_iters_f, "%d", (int)example->sliders[0]->value);

    char text_citers_f[16];
    sprintf(text_citers_f, "%d", (int)example->sliders[1]->value);

    char text_cciters_f[16];
    sprintf(text_cciters_f, "%d", (int)example->sliders[2]->value);

    char text_subs_f[16];
    sprintf(text_subs_f, "%d", (int)example->sliders[3]->value);

    char text_hertz_f[32];
    sprintf(text_hertz_f, "%d/sec", (int)example->sliders[4]->value);

    double unit_multipler = 1000.0;
    char unit_char = 'm';
    if (!example->switches[11]->on) {
        unit_multipler = 1000000.0;
        unit_char = 'u';
    }

    char *text_aa = "Anti-aliasing";
    char *text_fs = "Fill shapes";
    char *text_da = "Draw AABBs";
    char *text_dc = "Draw contacts";
    char *text_dd = "Draw directions";
    char *text_dj = "Draw constraints";
    char *text_dv = "Draw velocities";
    char *text_dg = "Draw broad-phase";
    char *text_s  = "Sleeping?";
    char *text_ws = "Warm-starting?";

    // Update and render UI widgets

    for (size_t i = 0; i < example->switch_count; i++) {
        ToggleSwitch *tg = example->switches[i];

        ToggleSwitch_update(example, tg);
        ToggleSwitch_draw(example, tg);
    }

    for (size_t i = 0; i < example->slider_count; i++) {
        Slider *s = example->sliders[i];
        Slider_update(example, s);
        Slider_draw(example, s);
    }

    for (size_t i = 0; i < example->button_count; i++) {
        Button *b = example->buttons[i];
        
        if (!strcmp(b->text, "Reset scene")) {
            b->x = example->width - 250 + 5;
        }

        Button_update(example, b);
        Button_draw(example, b, font);
    }

    draw_text_from_right(example, font, example->renderer, text_sdlver, 5, 5 + (y_gap*0), example->text_color);
    draw_text_from_right(example, font, example->renderer, text_novaver, 5, 5 + (y_gap*1), example->text_color);
    draw_text_from_right(example, font, example->renderer, text_instr, 5, 56 + (y_gap*0), example->alt_text_color);
    draw_text_from_right(example, font, example->renderer, text_instr1, 5, 56 + (y_gap*1), example->alt_text_color);
    draw_text_from_right(example, font, example->renderer, text_instr2, 5, 56 + (y_gap*2), example->alt_text_color);
    draw_text_from_right(example, font, example->renderer, text_instr3, 5, 56 + (y_gap*3), example->alt_text_color);
    draw_text_from_right(example, font, example->renderer, text_instr4, 5, 56 + (y_gap*4), example->alt_text_color);
    draw_text_from_right(example, font, example->renderer, text_instr5, 5, 56 + (y_gap*5), example->alt_text_color);
    draw_text_from_right(example, font, example->renderer, "Move the camera with [MOUSE WHL BUTTON]", 5, 56 + (y_gap*6), example->alt_text_color);
    draw_text_from_right(example, font, example->renderer, "Zoom in/out with [MOUSE WHL SCROLL]", 5, 56 + (y_gap*7), example->alt_text_color);

    draw_text(example, font, example->renderer, text_bodies, 123, 5 + (y_gap*0), example->text_color);
    draw_text(example, font, example->renderer, text_consts, 123, 5 + (y_gap*1), example->text_color);
    draw_text(example, font, example->renderer, text_attrs, 123, 5 + (y_gap*2), example->text_color);
    draw_text(example, font, example->renderer, text_ress, 123, 5 + (y_gap*3), example->text_color);

    draw_text(example, font, example->renderer, text_iters, 5, 10+15 + (y_gap*16), example->text_color);
    draw_text(example, font, example->renderer, text_citers, 5, 15+15 + (y_gap*17), example->text_color);
    draw_text(example, font, example->renderer, text_cciters, 5, 20+15 + (y_gap*18), example->text_color);
    draw_text(example, font, example->renderer, text_subs, 5, 25+15 + (y_gap*19), example->text_color);
    draw_text(example, font, example->renderer, text_hertz, 5, 30+15 + (y_gap*20), example->text_color);
    draw_text(example, font, example->renderer, text_iters_f, 196, 10+15 + (y_gap*16), example->text_color);
    draw_text(example, font, example->renderer, text_citers_f, 196, 15+15 + (y_gap*17), example->text_color);
    draw_text(example, font, example->renderer, text_cciters_f, 196, 20+15 + (y_gap*18), example->text_color);
    draw_text(example, font, example->renderer, text_subs_f, 196, 25+15 + (y_gap*19), example->text_color);
    draw_text(example, font, example->renderer, text_hertz_f, 196, 30+15 + (y_gap*20), example->text_color);

    draw_text(example, font, example->renderer, text_aa, 5, 10 + (y_gap*5), example->text_color);
    draw_text(example, font, example->renderer, text_fs, 5, 10 + (y_gap*6), example->text_color);
    draw_text(example, font, example->renderer, text_da, 5, 10 + (y_gap*7), example->text_color);
    draw_text(example, font, example->renderer, text_dc, 5, 10 + (y_gap*8), example->text_color);
    draw_text(example, font, example->renderer, text_dd, 5, 10 + (y_gap*9), example->text_color);
    draw_text(example, font, example->renderer, text_dj, 5, 10 + (y_gap*10), example->text_color);
    draw_text(example, font, example->renderer, text_dv, 5, 10 + (y_gap*11), example->text_color);
    draw_text(example, font, example->renderer, text_dg, 5, 10 + (y_gap*12), example->text_color);
    draw_text(example, font, example->renderer, "Draw positions", 5, 10 + (y_gap*13), example->text_color);
    draw_text(example, font, example->renderer, text_s,  5, 10 + (y_gap*14), example->text_color);
    draw_text(example, font, example->renderer, text_ws, 5, 10 + (y_gap*15), example->text_color);

    draw_text(example, font, example->renderer, "Parallel", 144, 10 + (y_gap*5), example->text_color);

    char text_threadslider[8];
    sprintf(text_threadslider, "%u", (nv_uint32)example->sliders[5]->value);

    draw_text(example, font, example->renderer, text_threadslider, 234, 110, example->text_color);

    draw_text(example, font, example->renderer, "Show profiler", 5, 140+15 + (y_gap*15), example->text_color);
    draw_text(example, font, example->renderer, "Show in milliseconds", 5, 140+15 + (y_gap*16), example->text_color);

    if (example->memory_graph_size == 240) {
        memmove(example->memory_graph_data, &example->memory_graph_data[1], (240 - 1) * sizeof(double));
        example->memory_graph_data[240 - 1] = memory_used_mb;
    }
    else {
        example->memory_graph_data[example->memory_graph_size] = memory_used_mb;
        example->memory_graph_size++;
    }

    if (example->fps_graph_size == 240) {
        memmove(example->fps_graph_data, &example->fps_graph_data[1], (240 - 1) * sizeof(double));
        example->fps_graph_data[240 - 1] = example->fps;
    }
    else {
        example->fps_graph_data[example->fps_graph_size] = example->fps;
        example->fps_graph_size++;
    }

    int profiler_y = 5;

    if (example->switches[10]->on) {
        SDL_SetRenderDrawColor(example->renderer, example->ui_color2.r, example->ui_color2.g, example->ui_color2.b, 175);
        SDL_RenderFillRect(example->renderer, &(SDL_Rect){250, 0, 540, 201});

        double percents[11] = {
            example->space->profiler.integrate_accelerations / example->space->profiler.step * 100.0,
            example->space->profiler.broadphase / example->space->profiler.step * 100.0,
            example->space->profiler.update_resolutions / example->space->profiler.step * 100.0,
            example->space->profiler.narrowphase / example->space->profiler.step * 100.0,
            example->space->profiler.presolve_collisions / example->space->profiler.step * 100.0,
            example->space->profiler.solve_positions / example->space->profiler.step * 100.0,
            example->space->profiler.solve_velocities / example->space->profiler.step * 100.0,
            example->space->profiler.presolve_constraints / example->space->profiler.step * 100.0,
            example->space->profiler.solve_constraints / example->space->profiler.step * 100.0,
            example->space->profiler.integrate_velocities / example->space->profiler.step * 100.0,
            example->space->profiler.remove_bodies / example->space->profiler.step * 100.0
        };

        GraphData graph_data[11];
        for (size_t j = 0; j < 11; j++) {
            graph_data[j] = (GraphData){percents[j], j};
        }

        char text_profiler0[48];
        sprintf(text_profiler0, "Step:             %.2f%cs 100.0%%", example->space->profiler.step * unit_multipler, unit_char);

        char text_profiler1[48];
        sprintf(text_profiler1, "Integrate accel.: %.2f%cs %.1f%%", example->space->profiler.integrate_accelerations * unit_multipler, unit_char, percents[0]);

        char text_profiler2[48];
        sprintf(text_profiler2, "Broad-phase:      %.2f%cs %.1f%%", example->space->profiler.broadphase * unit_multipler, unit_char, percents[1]);

        char text_profiler3[48];
        sprintf(text_profiler3, "Update res.:      %.2f%cs %.1f%%", example->space->profiler.update_resolutions * unit_multipler, unit_char, percents[2]);

        char text_profiler4[48];
        sprintf(text_profiler4, "Narrow-phase:     %.2f%cs %.1f%%", example->space->profiler.narrowphase * unit_multipler, unit_char, percents[3]);

        char text_profiler5[48];
        sprintf(text_profiler5, "Presolve colls.:  %.2f%cs %.1f%%", example->space->profiler.presolve_collisions * unit_multipler, unit_char, percents[4]);

        char text_profiler6[48];
        sprintf(text_profiler6, "Solve positions:  %.2f%cs %.1f%%", example->space->profiler.solve_positions * unit_multipler, unit_char, percents[5]);

        char text_profiler7[48];
        sprintf(text_profiler7, "Solve velocities: %.2f%cs %.1f%%", example->space->profiler.solve_velocities * unit_multipler, unit_char, percents[6]);

        char text_profiler8[48];
        sprintf(text_profiler8, "Presolve consts.: %.2f%cs %.1f%%", example->space->profiler.presolve_constraints * unit_multipler, unit_char, percents[7]);

        char text_profiler9[48];
        sprintf(text_profiler9, "Solve consts.:    %.2f%cs %.1f%%", example->space->profiler.solve_constraints * unit_multipler, unit_char, percents[8]);

        char text_profiler10[48];
        sprintf(text_profiler10, "Integrate vels.:  %.2f%cs %.1f%%", example->space->profiler.integrate_velocities * unit_multipler, unit_char, percents[9]);

        char text_profiler11[48];
        sprintf(text_profiler11, "Remove bodies:    %.2f%cs %.1f%%", example->space->profiler.remove_bodies * unit_multipler, unit_char, percents[10]);

        int boxsize = 10;
        for (size_t p = 0; p < 11; p++) {
            SDL_SetRenderDrawColor(example->renderer, example->profiler_palette[p].r, example->profiler_palette[p].g, example->profiler_palette[p].b, 255);
            SDL_RenderFillRect(example->renderer, &(SDL_Rect){255, profiler_y + (y_gap*(p+1)) + 3, boxsize, boxsize});
        }

        int profiler_text_x = 270;
        draw_text(example, font, example->renderer, text_profiler0, profiler_text_x, profiler_y + (y_gap*0), example->text_color);
        draw_text(example, font, example->renderer, text_profiler1, profiler_text_x, profiler_y + (y_gap*1), example->text_color);
        draw_text(example, font, example->renderer, text_profiler2, profiler_text_x, profiler_y + (y_gap*2), example->text_color);
        draw_text(example, font, example->renderer, text_profiler3, profiler_text_x, profiler_y + (y_gap*3), example->text_color);
        draw_text(example, font, example->renderer, text_profiler4, profiler_text_x, profiler_y + (y_gap*4), example->text_color);
        draw_text(example, font, example->renderer, text_profiler5, profiler_text_x, profiler_y + (y_gap*5), example->text_color);
        draw_text(example, font, example->renderer, text_profiler6, profiler_text_x, profiler_y + (y_gap*6), example->text_color);
        draw_text(example, font, example->renderer, text_profiler7, profiler_text_x, profiler_y + (y_gap*7), example->text_color);
        draw_text(example, font, example->renderer, text_profiler8, profiler_text_x, profiler_y + (y_gap*8), example->text_color);
        draw_text(example, font, example->renderer, text_profiler9, profiler_text_x, profiler_y + (y_gap*9), example->text_color);
        draw_text(example, font, example->renderer, text_profiler10, profiler_text_x, profiler_y + (y_gap*10), example->text_color);
        draw_text(example, font, example->renderer, text_profiler11, profiler_text_x, profiler_y + (y_gap*11), example->text_color);

        if (example->space->broadphase_algorithm == nvBroadPhaseAlg_BOUNDING_VOLUME_HIERARCHY) {
            char text_bvh0[48];
            sprintf(text_bvh0, "BVH build:        %.2f %cs", example->space->profiler.bvh_build * unit_multipler, unit_char);

            char text_bvh1[48];
            sprintf(text_bvh1, "BVH traverse:     %.2f %cs", example->space->profiler.bvh_traverse * unit_multipler, unit_char);

            char text_bvh2[48];
            sprintf(text_bvh2, "BVH destroy:      %.2f %cs", example->space->profiler.bvh_destroy * unit_multipler, unit_char);

            draw_text(example, font, example->renderer, text_bvh0, 255, profiler_y + (y_gap*12), example->text_color);
            draw_text(example, font, example->renderer, text_bvh1, 255, profiler_y + (y_gap*13), example->text_color);
            draw_text(example, font, example->renderer, text_bvh2, 255, profiler_y + (y_gap*14), example->text_color);
        }

        /* Physics step graph */

        draw_text(example, font, example->renderer, "0%", 501, 5, example->text_color);
        draw_text(example, font, example->renderer, "100%", 502+275-29, 5, example->text_color);

        SDL_SetRenderDrawColor(example->renderer, example->text_color.r, example->text_color.g, example->text_color.b, 120);
        SDL_RenderDrawLine(example->renderer, 502, 19, 502+275, 19);
        SDL_RenderDrawLine(example->renderer, 255, 19, 486, 19);

        float graph_width = 275.0;
        float graph_height = 29.0;
        float graph_x = 502.0;
        float graph_y = 32.0;

        qsort(graph_data, 11, sizeof(GraphData), graph_cmp);

        example->graph_counter++;
        if (example->graph_counter == 10) {
            example->graph_counter = 0;

            for (size_t j = 0; j < 11; j++) {
                example->last_graph[j] = graph_data[j];
            }
        }

        for (size_t j = 0; j < 11; j++) {
            double percent = example->last_graph[j].percent / 100.0;
            SDL_Color color = example->profiler_palette[example->last_graph[j].index];

            float width = graph_width * percent;

            SDL_SetRenderDrawColor(example->renderer, color.r, color.g, color.b, 255);
            SDL_RenderFillRectF(example->renderer, &(SDL_FRect){graph_x, graph_y, width, graph_height});

            graph_x += width;
        }

        graph_width = 240;
        float res = 1.0;

        /* FPS graph */

        SDL_SetRenderDrawColor(example->renderer, example->text_color.r, example->text_color.g, example->text_color.b, 255);
        SDL_RenderDrawLine(example->renderer, 534, 80, 534, 80+45);
        SDL_RenderDrawLine(example->renderer, 534, 80+45, 534+graph_width, 80+45);

        draw_text(example, font, example->renderer, "FPS", 534-10, 64, example->text_color);

        char fps_graph_max[8];
        sprintf(fps_graph_max, "%d", (int)example->max_fps);
        char fps_graph_half[8];
        sprintf(fps_graph_half, "%d", (int)(example->max_fps / 2.0));
        draw_text(example, font, example->renderer, fps_graph_max, 501, 80-3, example->text_color);
        draw_text(example, font, example->renderer, fps_graph_half, 501, 98-3, example->text_color);
        draw_text(example, font, example->renderer, "0", 501, 117-3, example->text_color);

        for (size_t x = 1; x < 240 * res; x += res) {
            float p0 = example->fps_graph_data[(int)((float)x/res)] / example->max_fps;
            p0 = nv_fclamp(p0, 0.0, 1.0);
            float v0 = p0 * 45.0;
            float x0 = x + 535;
            float p1 = example->fps_graph_data[(int)((float)x/res) - 1] / example->max_fps;
            p1 = nv_fclamp(p1, 0.0, 1.0);
            float v1 = p0 * 45.0;
            float x1 = (x - res) + 535;
            SDL_Color bar_color = color_lerp((SDL_Color){255, 0, 0}, (SDL_Color){0, 255, 0}, (p0 + p1)/2.0);
            SDL_SetRenderDrawColor(example->renderer, bar_color.r, bar_color.g, bar_color.b, 255);
            SDL_RenderDrawLine(example->renderer, x1, 80+45-v1, x0, 80+45-v0);
        }

        /* Memory usage graph */

        SDL_SetRenderDrawColor(example->renderer, example->text_color.r, example->text_color.g, example->text_color.b, 255);
        SDL_RenderDrawLine(example->renderer, 534, 147, 534, 147+45);
        SDL_RenderDrawLine(example->renderer, 534, 147+45, 534+graph_width, 147+45);

        draw_text(example, font, example->renderer, "Memory", 534-19, 131, example->text_color);
        char memory_graph_max[8];
        sprintf(memory_graph_max, "%d", (int)example->max_memory_usage);
        char memory_graph_half[8];
        sprintf(memory_graph_half, "%d", (int)(example->max_memory_usage / 2.0));
        draw_text(example, font, example->renderer, memory_graph_max, 501, 147-3, example->text_color);
        draw_text(example, font, example->renderer, memory_graph_half, 501, 165-3, example->text_color);
        draw_text(example, font, example->renderer, "0", 501, 184-3, example->text_color);

        for (size_t x = 1; x < 240 * res; x += res) {
            float p0 = example->memory_graph_data[(int)((float)x/res)] / example->max_memory_usage;
            float p = p0 * 45;
            SDL_Color bar_color = color_lerp((SDL_Color){255, 66, 66}, (SDL_Color){96, 56, 255}, p0);
            SDL_SetRenderDrawColor(example->renderer, bar_color.r, bar_color.g, bar_color.b, 255);
            SDL_RenderDrawLine(example->renderer, x+535, 147+45-p, x+535, 147+45);
        }
    }
}

/**
 * @brief Render constraints
 */
void draw_constraints(Example *example) {
    if (example->switches[4]->on) {
        for (size_t i = 0; i < example->space->constraints->size; i++) {
            nvConstraint *cons = (nvConstraint *)example->space->constraints->data[i];

            // Skip cursor body
            if (cons->a == (nvBody *)example->space->bodies->data[0] ||
                cons->b == (nvBody *)example->space->bodies->data[0])
                continue;

            // ? Forward declare to avoid errors on GCC < 10
            nvDistanceJoint *dist_joint;
            nvHingeJoint *hinge_joint;
            nvVector2 a, b, ra, rb;

            switch (cons->type) {

                case nvConstraintType_SPRING:
                    SDL_SetRenderDrawColor(
                        example->renderer,
                        example->spring_color.r,
                        example->spring_color.g,
                        example->spring_color.b,
                        example->spring_color.a
                    );
                    draw_spring(
                        example,
                        example->renderer,
                        cons,
                        example->switches[0]->on,
                        example->spring_color
                    );
                    break;

                case nvConstraintType_DISTANCEJOINT:
                    dist_joint = (nvDistanceJoint *)cons->def;

                    SDL_SetRenderDrawColor(
                        example->renderer,
                        example->distancejoint_color.r,
                        example->distancejoint_color.g,
                        example->distancejoint_color.b,
                        example->distancejoint_color.a
                    );

                    // Transform anchor points
                    if (cons->a == NULL) {
                        a = world_to_screen(example, dist_joint->anchor_a);
                    } else {
                        ra = nvVector2_rotate(dist_joint->anchor_a, cons->a->angle);
                        a = nvVector2_add(cons->a->position, ra);
                        a = world_to_screen(example, a);
                    }
                    if (cons->b == NULL) {
                        b = world_to_screen(example, dist_joint->anchor_b);
                    } else {
                        rb = nvVector2_rotate(dist_joint->anchor_b, cons->b->angle);
                        b = nvVector2_add(cons->b->position, rb);
                        b = world_to_screen(example, b);
                    }

                    if (example->switches[0]->on) {
                        draw_aaline(
                            example->renderer,
                            a.x, a.y,
                            b.x, b.y
                        );

                        draw_aacircle(
                            example->renderer,
                            a.x, a.y,
                            0.2 * example->zoom,
                            example->distancejoint_color.r,
                            example->distancejoint_color.g,
                            example->distancejoint_color.b
                        );

                        draw_aacircle(
                            example->renderer,
                            b.x, b.y,
                            0.2 * example->zoom,
                            example->distancejoint_color.r,
                            example->distancejoint_color.g,
                            example->distancejoint_color.b
                        );
                    }
                    else {
                        SDL_RenderDrawLineF(
                            example->renderer,
                            a.x, a.y,
                            b.x, b.y
                        );

                        draw_circle(
                            example->renderer,
                            a.x, a.y,
                            0.2 * example->zoom
                        );

                        draw_circle(
                            example->renderer,
                            b.x, b.y,
                            0.2 * example->zoom
                        );
                    }

                    break;

                case nvConstraintType_HINGEJOINT:
                    hinge_joint = (nvHingeJoint *)cons->def;

                    if (cons->a)
                        a = world_to_screen(example,
                            nvVector2_add(
                                nvVector2_rotate(hinge_joint->anchor_a, cons->a->angle), cons->a->position));
                    else
                        a = world_to_screen(example, hinge_joint->anchor);
                    if (cons->b)
                        b = world_to_screen(example,
                            nvVector2_add(
                                nvVector2_rotate(hinge_joint->anchor_b, cons->b->angle), cons->b->position));
                    else
                        b = world_to_screen(example, hinge_joint->anchor);
                    ra = nvVector2_mul(nvVector2_add(a, b), 0.5);

                    SDL_SetRenderDrawColor(
                        example->renderer,
                        example->hingejoint_color.r,
                        example->hingejoint_color.g,
                        example->hingejoint_color.b,
                        example->hingejoint_color.a
                    );

                    if (example->switches[0]->on) {
                        draw_aacircle(
                            example->renderer,
                            ra.x, ra.y,
                            0.5 * example->zoom,
                            example->hingejoint_color.r,
                            example->hingejoint_color.g,
                            example->hingejoint_color.b
                        );

                        draw_aacircle(
                            example->renderer,
                            a.x, a.y,
                            0.25 * example->zoom,
                            example->hingejoint_color.r,
                            example->hingejoint_color.g,
                            example->hingejoint_color.b
                        );

                        draw_aacircle(
                            example->renderer,
                            b.x, b.y,
                            0.25 * example->zoom,
                            example->hingejoint_color.r,
                            example->hingejoint_color.g,
                            example->hingejoint_color.b
                        );
                    }
                    else {
                        draw_circle(
                            example->renderer,
                            ra.x, ra.y,
                            0.5 * example->zoom
                        );

                        draw_circle(
                            example->renderer,
                            a.x, a.y,
                            0.25 * example->zoom
                        );

                        draw_circle(
                            example->renderer,
                            b.x, b.y,
                            0.25 * example->zoom
                        );
                    }

                    break;
            }
        }
    }
}

/**
 * @brief Render bodies.
 */
void draw_bodies(Example *example, TTF_Font *font) {
    // Start from 1 because 0 is cursor body
    for (size_t i = 1; i < example->space->bodies->size; i++) {
        nvBody *body = (nvBody *)example->space->bodies->data[i];

        nvAABB aabb = nvBody_get_aabb(body);
        nvVector2 aabb_min = world_to_screen(example, NV_VEC2(aabb.min_x, aabb.min_y));
        nvVector2 aabb_max = world_to_screen(example, NV_VEC2(aabb.max_x, aabb.max_y));

        // Shape outside of window viewport, don't draw
        if (!nv_collide_aabb_x_aabb((nvAABB){aabb_min.x, aabb_min.y, aabb_max.x, aabb_max.y}, (nvAABB){0.0, 0.0, example->width, example->height})) {
            continue;
        }

        SDL_FRect aabb_rect = (SDL_FRect){
            aabb_min.x,
            aabb_min.y,
            aabb_max.x - aabb_min.x,
            aabb_max.y - aabb_min.y
        };
            
        if (example->switches[1]->on) {
            SDL_SetRenderDrawColor(
                example->renderer, 
                example->aabb_color.r,
                example->aabb_color.g,
                example->aabb_color.b,
                example->aabb_color.a
                );
            SDL_RenderDrawRectF(example->renderer, &aabb_rect);
        }

        SDL_Color aacolor;

        if (body->type == nvBodyType_STATIC) {
            SDL_SetRenderDrawColor(
                example->renderer,
                example->static_color.r,
                example->static_color.g,
                example->static_color.b,
                example->static_color.a
            );

            aacolor = example->static_color;
        }
        else {
            SDL_SetRenderDrawColor(
                example->renderer,
                example->body_color.r,
                example->body_color.g,
                example->body_color.b,
                example->body_color.a
            );

            aacolor = example->body_color;
        }

        if (body->is_sleeping) {
            SDL_SetRenderDrawColor(
                example->renderer,
                example->sleep_color.r,
                example->sleep_color.g,
                example->sleep_color.b,
                example->sleep_color.a
            );

            aacolor = example->sleep_color;
        }

        // Rainbow
        //nv_uint16 r = (nv_uint16)(((nv_float)body->id) / ((nv_float)example->space->bodies->size) * 256.0);

        // Incremental
        //nv_uint16 r = body->id % 5;

        // Deterministic random
        nv_uint16 r = FNV1a_u32(body->id) % 5;

        SDL_Color color;

        // Nova palette
        if (r == 0) color = (SDL_Color){255, 212, 0, 255};
        if (r == 1) color = (SDL_Color){70, 51, 163, 255};
        if (r == 2) color = (SDL_Color){234, 222, 218, 255};
        if (r == 3) color = (SDL_Color){217, 3, 104, 255};
        if (r == 4) color = (SDL_Color){130, 2, 99, 255};

        // Rainbow
        //color = fhsv_to_rgb(r, 0.38, 1.0);
        //color.a = 255;

        // Draw circle bodies
        if (body->shape->type == nvShapeType_CIRCLE) {
            nvVector2 pos = world_to_screen(example, body->position);
            nv_float x = pos.x;
            nv_float y = pos.y;

            if (example->switches[0]->on) {
                draw_aacircle(
                    example->renderer,
                    x, y,
                    body->shape->radius * example->zoom,
                    aacolor.r,
                    aacolor.g,
                    aacolor.b
                );

                if (example->switches[3]->on) {
                    nvVector2 a = (nvVector2){body->shape->radius*example->zoom, 0.0};
                    a = nvVector2_rotate(a, body->angle);

                    draw_aaline(example->renderer, x, y, x+a.x, y+a.y);
                }
            }
            else if (example->switches[9]->on) {
                size_t n = 12;
                SDL_Vertex *vertices = malloc(sizeof(SDL_Vertex) * n);

                nvVector2 arm = NV_VEC2(body->shape->radius, 0.0);
                nvVector2 trans;

                for (size_t i = 0; i < n; i++) {
                    arm = nvVector2_rotate(arm, 2.0 * NV_PI / (nv_float)n);
                    trans = world_to_screen(example, nvVector2_add(body->position, arm));

                    vertices[i] = (SDL_Vertex){
                        .color = color,
                        .position = (SDL_FPoint){trans.x, trans.y},
                        .tex_coord = (SDL_FPoint){0.0, 0.0}
                    };
                }

                int indices[] = {0, 2, 1, 0, 3, 2, 0, 4, 3, 0, 5, 4, 0, 6, 5, 0, 7, 6, 0, 8, 7, 0, 9, 8, 0, 10, 9, 0, 11, 10};

                SDL_RenderGeometry(example->renderer, NULL, vertices, n, indices, 30);
                free(vertices);

                if (example->switches[3]->on) {
                    nvVector2 a = (nvVector2){body->shape->radius*example->zoom, 0.0};
                    a = nvVector2_rotate(a, body->angle);

                    SDL_SetRenderDrawColor(example->renderer, example->body_color.r, example->body_color.g, example->body_color.b, 255);

                    SDL_RenderDrawLineF(example->renderer, x, y, x+a.x, y+a.y);
                }
            }
            else {
                int32_t draw_radius = (int32_t)(body->shape->radius * example->zoom);
                draw_circle(
                    example->renderer,
                    (int32_t)x,
                    (int32_t)y,
                    draw_radius
                );

                if (example->switches[3]->on) {
                    nvVector2 a = (nvVector2){body->shape->radius*example->zoom, 0.0};
                    a = nvVector2_rotate(a, body->angle);

                    SDL_RenderDrawLineF(example->renderer, x, y, x+a.x, y+a.y);
                }
            }
        }

        // Draw polygon bodies
        else {
            nvBody_local_to_world(body);

            nvArray *verts = nvArray_new();
            for (size_t k = 0; k < body->shape->trans_vertices->size; k++) {
                nvVector2 p = world_to_screen(example, NV_TO_VEC2(body->shape->trans_vertices->data[k]));
                nvArray_add(verts, NV_VEC2_NEW(p.x, p.y));
            }

            if (example->switches[0]->on)
                draw_aapolygon(example->renderer, verts);

            else if (example->switches[9]->on) {
                size_t n = verts->size;

                if (n == 3) {

                    SDL_Vertex *vertices = malloc(sizeof(SDL_Vertex) * n);

                    for (size_t j = 0; j < n; j++) {
                        nvVector2 v = NV_TO_VEC2(verts->data[j]);

                        vertices[j] = (SDL_Vertex){
                            .color = color,
                            .position = (SDL_FPoint){v.x, v.y},
                            .tex_coord = (SDL_FPoint){0.0, 0.0}
                        };
                    }

                    SDL_RenderGeometry(example->renderer, NULL, vertices, n, NULL, 0);
                    free(vertices);
                }

                else if (n == 4) {

                    SDL_Vertex *vertices = malloc(sizeof(SDL_Vertex) * n);

                    for (size_t j = 0; j < n; j++) {
                        nvVector2 v = NV_TO_VEC2(verts->data[j]);

                        vertices[j] = (SDL_Vertex){
                            .color = color,
                            .position = (SDL_FPoint){v.x , v.y},
                            .tex_coord = (SDL_FPoint){0.0, 0.0}
                        };
                    }

                    int indices[6] = {0, 2, 1, 0, 3, 2};

                    SDL_RenderGeometry(example->renderer, NULL, vertices, n, indices, 6);
                    free(vertices);

                }

                else if (n == 5) {

                    SDL_Vertex *vertices = malloc(sizeof(SDL_Vertex) * n);

                    for (size_t j = 0; j < n; j++) {
                        nvVector2 v = NV_TO_VEC2(verts->data[j]);

                        vertices[j] = (SDL_Vertex){
                            .color = color,
                            .position = (SDL_FPoint){v.x, v.y},
                            .tex_coord = (SDL_FPoint){0.0, 0.0}
                        };
                    }

                    int indices[9] = {0, 2, 1, 0, 3, 2, 0, 4, 3};

                    SDL_RenderGeometry(example->renderer, NULL, vertices, n, indices, 9);
                    free(vertices);

                }

                else if (n == 6) {

                    SDL_Vertex *vertices = malloc(sizeof(SDL_Vertex) * n);

                    for (size_t j = 0; j < n; j++) {
                        nvVector2 v = NV_TO_VEC2(verts->data[j]);

                        vertices[j] = (SDL_Vertex){
                            .color = color,
                            .position = (SDL_FPoint){v.x, v.y},
                            .tex_coord = (SDL_FPoint){0.0, 0.0}
                        };
                    }

                    int indices[12] = {0, 2, 1, 0, 3, 2, 0, 4, 3, 0, 5, 4};

                    SDL_RenderGeometry(example->renderer, NULL, vertices, n, indices, 12);
                    free(vertices);
                }
            }
            else
                draw_polygon(example->renderer, verts);

            if (example->switches[3]->on) {
                nvVector2 center = nv_polygon_centroid(verts);
                nvVector2 diredge = nvVector2_div(
                    nvVector2_add(
                        NV_TO_VEC2(verts->data[0]),
                        NV_TO_VEC2(verts->data[1])),
                    2.0
                );

                if (example->switches[0]->on)
                    draw_aaline(
                        example->renderer,
                        center.x, center.y,
                        diredge.x, diredge.y
                    );
                else
                    SDL_RenderDrawLineF(
                        example->renderer,
                        center.x, center.y,
                        diredge.x, diredge.y
                    );
            }

            nvArray_free_each(verts, free);
            nvArray_free(verts);
        }

        // Draw velocity vectors
        if (example->switches[5]->on && body->type != nvBodyType_STATIC) {
            SDL_SetRenderDrawColor(
                example->renderer,
                example->velocity_color.r,
                example->velocity_color.g,
                example->velocity_color.b,
                example->velocity_color.a
            );

            nvVector2 vel = nvVector2_mul(body->linear_velocity, 1.0 / 60.0);

            nvVector2 v = world_to_screen(example, nvVector2_add(body->position, vel));

            nv_float threshold = 0.25 / 10.0;

            if (nvVector2_len2(vel) >= threshold) {
                nvVector2 p = nvVector2_mul(body->position, 10.0);
                nvVector2 arrow = nvVector2_mul(nvVector2_normalize(vel), 5.0);
                nvVector2 arrow1 = nvVector2_rotate(arrow, NV_PI / 6.0);
                nvVector2 arrow2 = nvVector2_rotate(arrow, NV_PI * 2.0 -  NV_PI / 6.0);

                if (example->switches[0]->on) {
                    draw_aaline(
                        example->renderer,
                        p.x, p.y,
                        v.x, v.y
                    );

                    draw_aaline(
                        example->renderer,
                        v.x, v.y,
                        v.x - arrow1.x, v.y - arrow1.y
                    );

                    draw_aaline(
                        example->renderer,
                        v.x, v.y,
                        v.x - arrow2.x, v.y - arrow2.y
                    );
                }
                else {
                    SDL_RenderDrawLineF(
                        example->renderer,
                        p.x, p.y,
                        v.x, v.y
                    );

                    SDL_RenderDrawLineF(
                        example->renderer,
                        v.x, v.y,
                        v.x - arrow1.x, v.y - arrow1.y
                    );

                    SDL_RenderDrawLineF(
                        example->renderer,
                        v.x, v.y,
                        v.x - arrow2.x, v.y - arrow2.y
                    );
                }
            }
        }

        // Draw center of masses
        if (example->switches[13]->on) {
            nvVector2 com = world_to_screen(example, body->position);
            nvVector2 dir = nvVector2_rotate(NV_VEC2(0.25, 0.0), body->angle);
            nvVector2 axis1 = nvVector2_add(com, nvVector2_mul(dir, example->zoom));
            nvVector2 axis2 = nvVector2_add(com, nvVector2_mul(nvVector2_perpr(dir), example->zoom));

            SDL_SetRenderDrawColor(example->renderer, 255, 0, 0, 255);

            if (example->switches[0]->on) {
                draw_aaline(example->renderer, com.x, com.y, axis1.x, axis1.y);
            }
            else {
                SDL_RenderDrawLineF(example->renderer, com.x, com.y, axis1.x, axis1.y);
            }

            SDL_SetRenderDrawColor(example->renderer, 0, 255, 0, 255);

            if (example->switches[0]->on) {
                draw_aaline(example->renderer, com.x, com.y, axis2.x, axis2.y);
            }
            else {
                SDL_RenderDrawLineF(example->renderer, com.x, com.y, axis2.x, axis2.y);
            }
        }
    }
}

void draw_cloth(Example *example) {
    int cols = example->cloth_example_cols;
    int rows = example->cloth_example_rows;

    for (size_t y = 0; y < rows; y++) {
        for (size_t x = 0; x < cols; x++) {
            if (x > 0 && y > 0) {
                nvBody *body0 = example->space->bodies->data[y * cols + x + 1];
                nvBody *body1 = example->space->bodies->data[y * cols + (x - 1) + 1];
                nvBody *body2 = example->space->bodies->data[(y - 1) * cols + x + 1];
                nvBody *body3 = example->space->bodies->data[(y - 1) * cols + (x - 1) + 1];
                nvVector2 pos0 = world_to_screen(example, body0->position);
                nvVector2 pos1 = world_to_screen(example, body1->position);
                nvVector2 pos2 = world_to_screen(example, body2->position);
                nvVector2 pos3 = world_to_screen(example, body3->position);

                SDL_Color color;

                // Signed area
                nv_float a = ((pos1.x - pos0.x) * (pos2.y - pos0.y) - (pos2.x - pos0.x) * (pos1.y - pos0.y));
                nv_float b = -((pos1.x - pos3.x) * (pos2.y - pos3.y) - (pos2.x - pos3.x) * (pos1.y - pos3.y));

                if (a < 0) {
                    color = fhsv_to_rgb((x*5 + y*12) % 360, 1.0, 0.5);
                    color.a = 255;
                }
                else {
                    color = fhsv_to_rgb((x*5 + y*12) % 360, 1.0, 1.0);
                    color.a = 255;
                }

                #ifdef NV_COMPILER_MSVC

                    SDL_Vertex *vertices = malloc(sizeof(SDL_Vertex) * 3);

                #else

                    SDL_Vertex vertices[3];

                #endif

                vertices[0] = (SDL_Vertex){
                    .color = color,
                    .position = (SDL_FPoint){pos0.x, pos0.y},
                    .tex_coord = (SDL_FPoint){0.0, 0.0}
                };

                vertices[1] = (SDL_Vertex){
                    .color = color,
                    .position = (SDL_FPoint){pos1.x, pos1.y},
                    .tex_coord = (SDL_FPoint){0.0, 0.0}
                };

                vertices[2] = (SDL_Vertex){
                    .color = color,
                    .position = (SDL_FPoint){pos2.x, pos2.y},
                    .tex_coord = (SDL_FPoint){0.0, 0.0}
                };

                SDL_RenderGeometry(example->renderer, NULL, vertices, 3, NULL, 0);

                SDL_Color colorb;
                if (b < 0) {
                    colorb = fhsv_to_rgb((x*5 + y*12) % 360, 1.0, 0.5);
                    colorb.a = 255;
                }
                else {
                    colorb = fhsv_to_rgb((x*5 + y*12) % 360, 1.0, 1.0);
                    colorb.a = 255;
                }

                vertices[0] = (SDL_Vertex){
                    .color = colorb,
                    .position = (SDL_FPoint){pos3.x, pos3.y},
                    .tex_coord = (SDL_FPoint){0.0, 0.0}
                };

                vertices[1] = (SDL_Vertex){
                    .color = colorb,
                    .position = (SDL_FPoint){pos1.x, pos1.y},
                    .tex_coord = (SDL_FPoint){0.0, 0.0}
                };

                vertices[2] = (SDL_Vertex){
                    .color = colorb,
                    .position = (SDL_FPoint){pos2.x, pos2.y},
                    .tex_coord = (SDL_FPoint){0.0, 0.0}
                };

                SDL_RenderGeometry(example->renderer, NULL, vertices, 3, NULL, 0);

                #ifdef NV_COMPILER_MSVC

                    free(vertices);

                #endif
            }
        }
    }
}

void draw_SHG(Example *example, TTF_Font *font) {
    SDL_SetRenderDrawColor(
        example->renderer,
        70,
        70,
        70,
        255
    );

    nvSHG *shg = example->space->shg;
    nvVector2 shg_min = world_to_screen(example, NV_VEC2(shg->bounds.min_x, shg->bounds.min_y));
    nvVector2 shg_max = world_to_screen(example, NV_VEC2(shg->bounds.max_x, shg->bounds.max_y));

    // Bounds
    SDL_FRect shg_rect = (SDL_FRect){
        shg_min.x,
        shg_min.y,
        shg_max.x - shg_min.x,
        shg_max.y - shg_min.y
    };

    SDL_RenderDrawRectF(example->renderer, &shg_rect);

    // Horizontal lines
    for (size_t y = 0; y < shg->rows; y++) {
        nvVector2 start = NV_VEC2(shg->bounds.min_x, shg->bounds.min_y + y * shg->cell_height);
        nvVector2 end = NV_VEC2(shg->bounds.min_x + shg->cols * shg->cell_width, shg->bounds.min_y + y * shg->cell_height);
        start = world_to_screen(example, start);
        end = world_to_screen(example, end);

        SDL_RenderDrawLine(
            example->renderer,
            start.x, start.y,
            end.x, end.y
        );
    }

    // Vertical lines
    for (size_t x = 0; x < shg->cols; x++) {
        nvVector2 start = NV_VEC2(shg->bounds.min_x + x * shg->cell_width, shg->bounds.min_y);
        nvVector2 end = NV_VEC2(shg->bounds.min_x + x * shg->cell_width, shg->bounds.min_y + shg->rows * shg->cell_height);
        start = world_to_screen(example, start);
        end = world_to_screen(example, end);

        SDL_RenderDrawLine(
            example->renderer,
            start.x, start.y,
            end.x, end.y
        );
    }

    if (example->space->multithreading) {
        nvAABB dyn_aabb = {NV_INF, NV_INF, -NV_INF, -NV_INF};
        for (size_t i = 0; i < example->space->bodies->size; i++) {
            nvBody *body = example->space->bodies->data[i];
            if (body->type == nvBodyType_STATIC) continue;
            nvAABB aabb = nvBody_get_aabb(body);

            dyn_aabb.min_x = nv_fmin(dyn_aabb.min_x, aabb.min_x);
            dyn_aabb.min_y = nv_fmin(dyn_aabb.min_y, aabb.min_y);
            dyn_aabb.max_x = nv_fmax(dyn_aabb.max_x, aabb.max_x);
            dyn_aabb.max_y = nv_fmax(dyn_aabb.max_y, aabb.max_y);
        }

        nv_float q = (dyn_aabb.max_x - dyn_aabb.min_x) / (nv_float)example->space->thread_count;
        for (size_t i = 0; i < example->space->bodies->size; i++) {
            nvBody *body = example->space->bodies->data[i];
            if (body->type == nvBodyType_STATIC) continue;
            nvAABB aabb = nvBody_get_aabb(body);
            nvVector2 p = world_to_screen(example, body->position);

            for (size_t j = 0; j < example->space->thread_count; j++) {

                nv_float s = (nv_float)j / (nv_float)example->space->thread_count * 256.0;
                SDL_Color color = hsv_to_rgb((SDL_Color){(nv_uint8)s, 255, 255});
                SDL_SetRenderDrawColor(example->renderer, color.r, color.g, color.b, 255);

                if (j == 0) {
                    if (
                        aabb.max_x >= dyn_aabb.min_x &&
                        body->position.x <= q + dyn_aabb.min_x
                    ) {
                        draw_circle(example->renderer, p.x, p.y, 0.2 * example->zoom);
                        break;
                    }
                }

                else if (j == (example->space->thread_count - 1)) {
                    if (
                        aabb.min_x <= dyn_aabb.max_x &&
                        body->position.x > q * (nv_float)(example->space->thread_count - 1) + dyn_aabb.min_x
                    ) {
                        draw_circle(example->renderer, p.x, p.y, 0.2 * example->zoom);
                        break;
                    }
                }

                else {
                    if (
                        body->position.x > q * (nv_float)(j) + dyn_aabb.min_x &&
                        body->position.x <= q * (nv_float)(j + 1) + dyn_aabb.min_x
                    ) {
                        draw_circle(example->renderer, p.x, p.y, 0.2 * example->zoom);
                        break;
                    }
                }
            }
        }

        for (size_t j = 0; j < example->space->thread_count; j++) {
            SDL_SetRenderDrawColor(
                example->renderer,
                99,
                66,
                66,
                255
            );

            nvVector2 dyn_min = world_to_screen(example, NV_VEC2(dyn_aabb.min_x, dyn_aabb.min_y));
            nvVector2 dyn_max = world_to_screen(example, NV_VEC2(dyn_aabb.max_x, dyn_aabb.max_y));

            SDL_RenderDrawRect(
                example->renderer,
                &(SDL_Rect){
                    dyn_min.x,
                    dyn_min.y,
                    dyn_max.x - dyn_min.x,
                    dyn_max.y - dyn_min.y
                }
            );

            draw_dashed_line(
                example->renderer,
                round((((nv_float)j * q) * example->zoom + dyn_min.x)),
                dyn_min.y,
                round((((nv_float)j * q) * example->zoom + dyn_min.x)),
                dyn_min.y,
                0.3 * example->zoom,
                0.5 * example->zoom
            );
        }
    }
}

void draw_BVH(Example *example, nvBVHNode *node) {
    SDL_SetRenderDrawColor(
        example->renderer,
        70,
        70,
        70,
        255
    );

    SDL_FRect aabb_rect = (SDL_FRect){
        node->aabb.min_x*10.0,
        node->aabb.min_y*10.0,
        (node->aabb.max_x - node->aabb.min_x)*10.0,
        (node->aabb.max_y - node->aabb.min_y)*10.0
    };

    SDL_RenderDrawRectF(example->renderer, &aabb_rect);

    SDL_SetRenderDrawColor(
        example->renderer,
        99,
        66,
        66,
        255
    );

    if (!node->is_leaf) {
        nv_float width = node->aabb.max_x - node->aabb.min_x;
        nv_float height = node->aabb.max_y - node->aabb.min_y;

        if (width > height) {
            nv_float split = 0.0;
            for (size_t i = 0; i < node->bodies->size; i++) {
                nvBody *body = node->bodies->data[i];
                split += body->position.x;
            }
            split /= (nv_float)node->bodies->size;

            draw_dashed_line(
                example->renderer,
                split * 10.0,
                node->aabb.min_y * 10.0,
                split * 10.0,
                node->aabb.max_y * 10.0,
                3,
                5
            );
        }
        else {
            nv_float split = 0.0;
            for (size_t i = 0; i < node->bodies->size; i++) {
                nvBody *body = node->bodies->data[i];
                split += body->position.y;
            }
            split /= (nv_float)node->bodies->size;

            draw_dashed_line(
                example->renderer,
                node->aabb.min_x * 10.0,
                split * 10.0,
                node->aabb.max_x * 10.0,
                split * 10.0,
                3,
                5
            );
        }

        if (node->left != NULL) draw_BVH(example, node->left);
        if (node->right != NULL) draw_BVH(example, node->right);
    }
}


/**
 * @brief Update ToggleSwitch object.
 */
void ToggleSwitch_update(struct _Example *example, ToggleSwitch *tg) {
    if (example->mouse.x < tg->x + tg->size && example->mouse.x > tg->x &&
        example->mouse.y < tg->y + tg->size && example->mouse.y > tg->y) {
        
        if (!example->selected && example->mouse.left && !tg->changed) {
            tg->on = !tg->on;
            tg->changed = true;

            if (tg == example->switches[7]) {
                if (tg->on)
                    nvSpace_enable_sleeping(example->space);
                else
                    nvSpace_disable_sleeping(example->space);
            }

            if (tg == example->switches[8])
                example->space->warmstarting = tg->on;

            if (tg == example->switches[12]) {
                if (tg->on)
                    nvSpace_enable_multithreading(example->space, example->sliders[5]->value);
                else
                    nvSpace_disable_multithreading(example->space);
            }
        }
    }
}

/**
 * @brief Draw ToggleSwitch object.
 */
void ToggleSwitch_draw(struct _Example *example, ToggleSwitch *tg) {
    if (tg->on) {
        SDL_SetRenderDrawColor(
            example->renderer,
            example->ui_color.r,
            example->ui_color.g,
            example->ui_color.b,
            example->ui_color.a
        );
        SDL_RenderFillRect(example->renderer, &(SDL_Rect){tg->x, tg->y, tg->size, tg->size});
    }

    SDL_SetRenderDrawColor(
        example->renderer,
        example->text_color.r,
        example->text_color.g,
        example->text_color.b,
        example->text_color.a
    );

    SDL_RenderDrawRect(example->renderer, &(SDL_Rect){tg->x, tg->y, tg->size, tg->size});
}

/**
 * @brief Update Slider object.
 */
void Slider_update(struct _Example *example, Slider *s) {
    if (s->pressed) {
        int cx;
        if (example->mouse.x < s->x) cx = s->x;
        else if (example->mouse.x > s->x + s->width) cx = s->x + s->width;
        else cx = example->mouse.x;
        s->cx = cx;
        s->value = s->min + (((nv_float)cx - (nv_float)s->x) / (nv_float)s->width) * (s->max - s->min);
    }
}

/**
 * @brief Draw Slider object.
 */
void Slider_draw(struct _Example *example, Slider *s) {
    SDL_SetRenderDrawColor(
        example->renderer,
        example->ui_color.r,
        example->ui_color.g,
        example->ui_color.b,
        example->ui_color.a
    );

    SDL_RenderFillRectF(
        example->renderer,
        &(SDL_FRect){
            s->x, s->y,
            s->width, 4.0
        }
    );

    SDL_SetRenderDrawColor(
        example->renderer,
        example->text_color.r,
        example->text_color.g,
        example->text_color.b,
        example->text_color.a
    );

    SDL_RenderDrawRectF(
        example->renderer,
        &(SDL_FRect){
            s->cx, s->y - 2.0,
            3.0, 8.0
        }
    );
}


void Button_update(struct _Example *example, Button *b) {
    if (!example->selected) {
        if (example->mouse.x < b->x + b->width && example->mouse.x > b->x &&
            example->mouse.y < b->y + b->height && example->mouse.y > b->y) {

            b->hovered = true;
            
            if (example->mouse.left) {
                b->pressed = true;
            }
        }
        else
            b->hovered = false;
    }
}

void Button_draw(struct _Example *example, Button *b, TTF_Font *font) {
    if (b->pressed) {
        SDL_SetRenderDrawColor(
            example->renderer,
            example->ui_color.r,
            example->ui_color.g,
            example->ui_color.b,
            example->ui_color.a
        );

        SDL_RenderFillRect(example->renderer, &(SDL_Rect){b->x, b->y, b->width, b->height});

        SDL_SetRenderDrawColor(
            example->renderer,
            example->text_color.r,
            example->text_color.g,
            example->text_color.b,
            example->text_color.a
        );
    }
    else if (b->hovered) {
        SDL_SetRenderDrawColor(
            example->renderer,
            example->ui_color.r,
            example->ui_color.g,
            example->ui_color.b,
            example->ui_color.a
        );
    }
    else {
        SDL_SetRenderDrawColor(
            example->renderer,
            example->text_color.r,
            example->text_color.g,
            example->text_color.b,
            example->text_color.a
        );
    }

    SDL_RenderDrawRect(example->renderer, &(SDL_Rect){b->x, b->y, b->width, b->height});

    draw_text_middle(example, font, example->renderer, b->text, b->x, b->y, b->width, b->height, example->text_color);
}


void button_callback(Button *button) {
    if (!strcmp(button->text, "Reset scene")) return;

    for (size_t i = 0; i < example_count; i++) {
        if (!strcmp(button->text, example_entries[i].name)) {
            current_example = i;
            return;
        }
    }
}



/******************************************************************************

                                  Main loop

******************************************************************************/



/**
 * @brief Entry point of the example.
 * 
 * @param example Example to run
 */
void Example_run(Example *example) {
    bool is_running = true;
    Uint64 start_perf;
    Uint64 start_perf_hi = SDL_GetPerformanceCounter();;
    Uint64 end_perf;
    Uint64 end_perf_hi;
    Uint64 step_time_start;
    Uint64 step_time_end;
    nv_float step_time_f;
    nv_float step_final = 0.0;
    size_t step_count = 0;
    Uint64 render_time_start;
    Uint64 render_time;
    nv_float render_time_f = 0.0;
    nv_float frequency = (nv_float)SDL_GetPerformanceFrequency();
    int frames = 0;
    int fps_every_f = 10;

    // Set random seed
    srand(time(NULL));

    SDL_Event event;

    nvBody *mouse_body = nvBody_new(
        nvBodyType_STATIC,
        nvCircleShape_new(0.1),
        nvVector2_zero,
        0.0,
        nvMaterial_BASIC
    );
    mouse_body->enable_collision = false;
    nvSpace_add(example->space, mouse_body);

    nvBody *selected = NULL;
    nvConstraint *selected_const = NULL;
    nvVector2 selected_posf = nvVector2_zero;
    nvVector2 selected_pos = nvVector2_zero;

    TTF_Font *font;

    font = TTF_OpenFont("assets/FiraCode-Regular.ttf", 11);
    if (font == NULL) {
        printf("Couldn't load assets/FiraCode-Regular.ttf\n");
        exit(1);
    }
    TTF_SetFontStyle(font, TTF_STYLE_NORMAL);
    TTF_SetFontOutline(font, 0);
    TTF_SetFontKerning(font, 1);
    TTF_SetFontHinting(font, TTF_HINTING_NORMAL);

    // MSVC doesn't allow variable length arrays
    size_t switches_n = 14;
    ToggleSwitch **switches = malloc(sizeof(ToggleSwitch) * switches_n);

    switches[0] = &(ToggleSwitch){
        .x = 118+6, .y = 63+4+32-5,
        .size = 9, .on = false
    };

    switches[1] = &(ToggleSwitch){
        .x = 118+6, .y = 95+4+32-5,
        .size = 9, .on = false
    };

    switches[2] = &(ToggleSwitch){
        .x = 118+6, .y = 111+4+32-5,
        .size = 9, .on = false
    };

    switches[3] = &(ToggleSwitch){
        .x = 118+6, .y = 127+4+32-5,
        .size = 9, .on = false
    };

    switches[4] = &(ToggleSwitch){
        .x = 118+6, .y = 143+4+32-5,
        .size = 9, .on = true
    };

    switches[5] = &(ToggleSwitch){
        .x = 118+6, .y = 159+4+32-5,
        .size = 9, .on = false
    };

    switches[6] = &(ToggleSwitch){
        .x = 118+6, .y = 175+4+32-5,
        .size = 9, .on = false
    };

    switches[7] = &(ToggleSwitch){
        .x = 118+6, .y = 207+4+32-5,
        .size = 9, .on = false
    };

    switches[8] = &(ToggleSwitch){
        .x = 118+6, .y = 223+4+32-5,
        .size = 9, .on = true
    };
    
    switches[9] = &(ToggleSwitch){
        .x = 118+6, .y = 79+4+32-5,
        .size = 9, .on = false
    };

    switches[10] = &(ToggleSwitch){
        .x = 118+34, .y = 383+15,
        .size = 9, .on = false
    };

    switches[11] = &(ToggleSwitch){
        .x = 118+34, .y = 383+16+15,
        .size = 9, .on = true
    };

    switches[12] = &(ToggleSwitch){
        .x = 210, .y = 94,
        .size = 9, .on = false
    };

    // draw coms
    switches[13] = &(ToggleSwitch){
        .x = 118+6, .y = 192+4+32-5,
        .size = 9, .on = false
    };

    example->switches = switches;
    example->switch_count = switches_n;

    size_t sliders_n = 6;
    Slider **sliders = malloc(sizeof(Slider) * sliders_n);

    int slider_offset = 25;

    sliders[0] = &(Slider){
        .x = 135-slider_offset, .y = 271+15,
        .width = 80,
        .min = 1, .max = 50, .value = 10,
        .type=SliderType_INTEGER
    };
    sliders[0]->cx = sliders[0]->x + ((sliders[0]->value-sliders[0]->min) / (sliders[0]->max - sliders[0]->min)) * sliders[0]->width;

    sliders[1] = &(Slider){
        .x = 135-slider_offset, .y = 271+15 + (21*1),
        .width = 80,
        .min = 1, .max = 50, .value = 10,
        .type=SliderType_INTEGER
    };
    sliders[1]->cx = sliders[1]->x + ((sliders[1]->value-sliders[1]->min) / (sliders[1]->max - sliders[1]->min)) * sliders[1]->width;

    sliders[2] = &(Slider){
        .x = 135-slider_offset, .y = 271+15 + (21*2),
        .width = 80,
        .min = 1, .max = 50, .value = 5,
        .type=SliderType_INTEGER
    };
    sliders[2]->cx = sliders[2]->x + ((sliders[2]->value-sliders[2]->min) / (sliders[2]->max - sliders[2]->min)) * sliders[2]->width;

    sliders[3] = &(Slider){
        .x = 135-slider_offset, .y = 271+15 + (21*3),
        .width = 80,
        .min = 1, .max = 10, .value = 1,
        .type=SliderType_INTEGER
    };
    sliders[3]->cx = sliders[3]->x + ((sliders[3]->value-sliders[3]->min) / (sliders[3]->max - sliders[3]->min)) * sliders[3]->width;

    sliders[4] = &(Slider){
        .x = 135-slider_offset, .y = 271+15 + (21*4),
        .width = 80,
        .min = 12.0, .max = 240.0, .value = 60.0,
        .type=SliderType_INTEGER
    };
    sliders[4]->cx = sliders[4]->x + ((sliders[4]->value-sliders[4]->min) / (sliders[4]->max - sliders[4]->min)) * sliders[4]->width;

    nv_uint32 max_threads = nv_get_cpu_count();

    sliders[5] = &(Slider){
        .x = 145, .y = 113,
        .width = 80,
        .min = 1, .max = max_threads, .value = max_threads,
        .type=SliderType_INTEGER
    };
    sliders[5]->cx = sliders[5]->x + ((sliders[5]->value-sliders[5]->min) / (sliders[5]->max - sliders[5]->min)) * sliders[5]->width;

    example->sliders = sliders;
    example->slider_count = sliders_n;

    size_t buttons_n = 18;
    Button **buttons = malloc(sizeof(Button) * buttons_n);

    int button_height = 23;

    buttons[0] = &(Button){
        .x=5, .y=445,
        .width=117, .height=button_height,
        .text="Arch",
        .callback=(void (*)(void *))button_callback
    };

    buttons[1] = &(Button){
        .x=5, .y=445+(button_height+5)*1,
        .width=117, .height=button_height,
        .text="Bridge",
        .callback=(void (*)(void *))button_callback
    };

    buttons[2] = &(Button){
        .x=5, .y=445+(button_height+5)*2,
        .width=117, .height=button_height,
        .text="Chains",
        .callback=(void (*)(void *))button_callback
    };

    buttons[3] = &(Button){
        .x=5, .y=445+(button_height+5)*3,
        .width=117, .height=button_height,
        .text="Circle Stack",
        .callback=(void (*)(void *))button_callback
    };

    buttons[4] = &(Button){
        .x=5, .y=445+(button_height+5)*4,
        .width=117, .height=button_height,
        .text="Cloth",
        .callback=(void (*)(void *))button_callback
    };

    buttons[5] = &(Button){
        .x=5, .y=445+(button_height+5)*5,
        .width=117, .height=button_height,
        .text="Constraints",
        .callback=(void (*)(void *))button_callback
    };

    buttons[6] = &(Button){
        .x=5, .y=445+(button_height+5)*6,
        .width=117, .height=button_height,
        .text="Domino",
        .callback=(void (*)(void *))button_callback
    };

    buttons[7] = &(Button){
        .x=5, .y=445+(button_height+5)*7,
        .width=117, .height=button_height,
        .text="Fountain",
        .callback=(void (*)(void *))button_callback
    };

    buttons[8] = &(Button){
        .x=127, .y=445+(button_height+5)*0,
        .width=117, .height=button_height,
        .text="Hull",
        .callback=(void (*)(void *))button_callback
    };

    buttons[9] = &(Button){
        .x=127, .y=445+(button_height+5)*1,
        .width=117, .height=button_height,
        .text="Newton's Cradle",
        .callback=(void (*)(void *))button_callback
    };

    buttons[10] = &(Button){
        .x=127, .y=445+(button_height+5)*2,
        .width=117, .height=button_height,
        .text="Orbit",
        .callback=(void (*)(void *))button_callback
    };

    buttons[11] = &(Button){
        .x=127, .y=445+(button_height+5)*3,
        .width=117, .height=button_height,
        .text="Pool",
        .callback=(void (*)(void *))button_callback
    };

    buttons[12] = &(Button){
        .x=127, .y=445+(button_height+5)*4,
        .width=117, .height=button_height,
        .text="Pyramid",
        .callback=(void (*)(void *))button_callback
    };

    buttons[13] = &(Button){
        .x=127, .y=445+(button_height+5)*5,
        .width=117, .height=button_height,
        .text="Spring Car",
        .callback=(void (*)(void *))button_callback
    };

    buttons[14] = &(Button){
        .x=127, .y=445+(button_height+5)*6,
        .width=117, .height=button_height,
        .text="Stack",
        .callback=(void (*)(void *))button_callback
    };

    buttons[15] = &(Button){
        .x=127, .y=445+(button_height+5)*7,
        .width=117, .height=button_height,
        .text="Varying Bounce",
        .callback=(void (*)(void *))button_callback
    };

    buttons[16] = &(Button){
        .x=127, .y=445+(button_height+5)*8,
        .width=117, .height=button_height,
        .text="Varying Friction",
        .callback=(void (*)(void *))button_callback
    };

    buttons[17] = &(Button){
        .x=example->width - 250 + 5, .y=200+34,
        .width=117, .height=button_height,
        .text="Reset scene",
        .callback=(void (*)(void *))button_callback
    };

    example->buttons = buttons;
    example->button_count = buttons_n;

    if (example_entries[current_example].setup_callback != NULL)
        example_entries[current_example].setup_callback(example);

    nv_uint64 step_counter = 0;
    nv_uint64 render_counter = 0;
    nv_uint64 frame_counter = 0;

    bool frame_by_frame = false;
    bool next_frame = false;

    while (is_running) {
        start_perf = SDL_GetTicks64();
        next_frame = false;

        SDL_GetMouseState(&example->mouse.x, &example->mouse.y);
        example->mouse.before_zoom = screen_to_world(example, NV_VEC2(example->mouse.x, example->mouse.y));

        // Handle events
        while(SDL_PollEvent(&event) != 0) {
            if (event.type == SDL_QUIT)
                is_running = false;

            else if (event.type == SDL_WINDOWEVENT) {
                if (event.window.event == SDL_WINDOWEVENT_RESIZED) {
                    example->width = event.window.data1;
                    example->height = event.window.data2;
                }
            }

            else if (event.type == SDL_MOUSEBUTTONDOWN) {
                if (event.button.button == SDL_BUTTON_LEFT)
                    example->mouse.left = true;

                else if (event.button.button == SDL_BUTTON_MIDDLE) {
                    example->mouse.middle = true;
                    example->pan_start = NV_VEC2(example->mouse.x, example->mouse.y);
                }

                else if (event.button.button == SDL_BUTTON_RIGHT)
                    example->mouse.right = true;

                if (example->mouse.left) {
                    selected = NULL;
                    if (nv_collide_aabb_x_point((nvAABB){250.0, 0.0, example->width, example->height}, NV_VEC2(example->mouse.x, example->mouse.y))) {
                        for (size_t i = 0; i < example->space->bodies->size; i++) {
                            nvBody *body = (nvBody *)example->space->bodies->data[i];
                            if (body->type == nvBodyType_STATIC) continue;

                            bool inside = false;
                            nvShape *shape = body->shape;

                            if (shape->type == nvShapeType_POLYGON) {
                                nvBody_local_to_world(body);
                                inside = nv_collide_polygon_x_point(body, example->mouse.after_zoom);
                            }
                            else if (shape->type == nvShapeType_CIRCLE) {
                                inside = nv_collide_circle_x_point(body, example->mouse.after_zoom);
                            }

                            if (inside) {
                                selected = body;
                                example->selected = true;

                                // Transform mouse coordinatets to body local coordinates
                                selected_posf = example->mouse.after_zoom;
                                selected_posf = nvVector2_sub(selected_posf, selected->position);
                                selected_posf = nvVector2_rotate(selected_posf, -selected->angle);

                                selected_pos = NV_VEC2(selected_posf.x+0.00001, selected_posf.y+0.00001);

                                nv_float strength = 150.0 * selected->mass / 3.0;
                                nv_float damping = 70.0 * selected->mass / 4.0;

                                if (!strcmp(example_entries[current_example].name, "Cloth")) {
                                    strength *= 10.0;
                                    damping *= 2.0;
                                }

                                selected_const = nvSpring_new(
                                    mouse_body, selected,
                                    nvVector2_zero, selected_pos,
                                    0.0, strength, damping
                                );

                                nvSpace_add_constraint(example->space, selected_const);

                                if (selected->is_sleeping) nvBody_awake(selected);

                                break;
                            }
                        }
                    }

                    for (size_t i = 0; i < example->slider_count; i++) {
                        Slider *s = example->sliders[i];

                        if (example->mouse.x < s->x + s->width && example->mouse.x > s->x &&
                            example->mouse.y < s->y + 10.0 && example->mouse.y > s->y - 4.0) {
                            
                            s->pressed = true;
                            break;
                        }
                    }

                    for (size_t i = 0; i < example_entries[current_example].slider_settings->size; i++) {
                        Slider *s = ((SliderSetting *)example_entries[current_example].slider_settings->data[i])->slider;

                        if (example->mouse.x < s->x + s->width && example->mouse.x > s->x &&
                            example->mouse.y < s->y + 10.0 && example->mouse.y > s->y - 4.0) {

                            s->pressed = true;
                            break;
                        }
                    }
                }
            }

            else if (event.type == SDL_MOUSEBUTTONUP) {
                if (event.button.button == SDL_BUTTON_LEFT) {
                    example->mouse.left = false;
                    selected = NULL;
                    example->selected = false;

                    if (selected_const != NULL) {
                        nvArray_remove(example->space->constraints, selected_const);
                        nvConstraint_free(selected_const);
                        selected_const = NULL;
                    }

                    for (size_t i = 0; i < switches_n; i++) {
                        switches[i]->changed = false;
                    }

                    for (size_t i = 0; i < sliders_n; i++) {
                        sliders[i]->pressed = false;

                        if (i == 5 && switches[12]->on) {
                            nvSpace_disable_multithreading(example->space);
                            nvSpace_enable_multithreading(example->space, sliders[i]->value);
                        }
                    }

                    for (size_t i = 0; i < example_entries[current_example].slider_settings->size; i++) {
                        ((SliderSetting *)example_entries[current_example].slider_settings->data[i])->slider->pressed = false;
                    }

                    for (size_t i = 0; i < buttons_n; i++) {
                        if (buttons[i]->pressed) {
                            buttons[i]->pressed = false;
                            if (buttons[i]->callback) {
                                selected = NULL;
                                if (selected_const != NULL) {
                                    nvArray_remove(example->space->constraints, selected_const);
                                    nvConstraint_free(selected_const);
                                    selected_const = NULL;
                                }

                                nvSpace_clear(example->space);
                                example->space->_id_counter = 0;

                                mouse_body = nvBody_new(
                                    nvBodyType_STATIC,
                                    nvCircleShape_new(0.1),
                                    nvVector2_zero,
                                    0.0,
                                    nvMaterial_BASIC
                                );
                                mouse_body->enable_collision = false;
                                mouse_body->position = NV_VEC2(example->mouse.px, example->mouse.py);
                                mouse_body->_cache_aabb = false;
                                nvSpace_add(example->space, mouse_body);

                                example->counter = 0;

                                buttons[i]->callback(buttons[i]);

                                example->space->gravity = NV_VEC2(0.0, NV_GRAV_EARTH);

                                if (example->space->broadphase_algorithm == nvBroadPhaseAlg_SPATIAL_HASH_GRID)
                                    nvSpace_set_SHG(example->space, (nvAABB){0.0, 0.0, 128.0, 72.0}, 3.0, 3.0);

                                if (example_entries[current_example].setup_callback != NULL)
                                    example_entries[current_example].setup_callback(example);

                                step_count = 0;
                                step_final = 0.0;

                                if (!strcmp(example_entries[current_example].name, "Cloth")) {
                                    example->cloth_example_cols = get_slider_setting("Columns");
                                    example->cloth_example_rows = get_slider_setting("Rows");
                                }
                            }
                        }
                    }
                }

                else if (event.button.button == SDL_BUTTON_MIDDLE)
                    example->mouse.middle = false;

                else if (event.button.button == SDL_BUTTON_RIGHT)
                    example->mouse.right = false;
            }

            else if (event.type == SDL_MOUSEWHEEL) {
                if (event.wheel.y > 0) {
                    example->zoom *= 1 + example->zoom_scale;
                }

                else if (event.wheel.y < 0) {
                    example->zoom *= 1 - example->zoom_scale;
                }
            }

            else if (event.type == SDL_KEYDOWN) {
                if (event.key.keysym.scancode == SDL_SCANCODE_Q) {
                    for (size_t i = 0; i < example->space->bodies->size; i++) {
                        nvBody *body = (nvBody *)example->space->bodies->data[i];
                        if (body->type == nvBodyType_STATIC) continue;

                        nvVector2 delta = nvVector2_sub(
                            body->position,
                            screen_to_world(example, NV_VEC2(example->mouse.x, example->mouse.y))
                        );

                        nv_float strength = 10.0 * pow(10.0, 3.0);

                        if (!strcmp(example_entries[current_example].name, "Cloth")) {
                            strength /= 30.0;
                        }

                        nvVector2 force = nvVector2_mul(delta, strength);
                        force = nvVector2_div(force, nvVector2_len(delta));

                        nvBody_apply_force(body, force);
                    }
                }

                else if (event.key.keysym.scancode == SDL_SCANCODE_R) {
                    example->camera = NV_VEC2(example->width / 20.0, example->height / 20.0);
                    example->zoom = 10.0;
                    example->pan_start = nvVector2_zero;
                    example->mouse.before_zoom = nvVector2_zero;
                    example->mouse.after_zoom = nvVector2_zero;
                    example->mouse.x = 0;
                    example->mouse.y = 0;

                    selected = NULL;
                    if (selected_const != NULL) {
                        nvArray_remove(example->space->constraints, selected_const);
                        nvConstraint_free(selected_const);
                        selected_const = NULL;
                    }

                    nvSpace_clear(example->space);
                    example->space->_id_counter = 0;

                    mouse_body = nvBody_new(
                        nvBodyType_STATIC,
                        nvCircleShape_new(0.1),
                        nvVector2_zero,
                        0.0,
                        nvMaterial_BASIC
                    );
                    mouse_body->enable_collision = false;
                    mouse_body->position = nvVector2_zero;
                    mouse_body->_cache_aabb = false;
                    nvSpace_add(example->space, mouse_body);

                    example->counter = 0;

                    if (example_entries[current_example].setup_callback != NULL)
                        example_entries[current_example].setup_callback(example);

                    if (!strcmp(example_entries[current_example].name, "Cloth")) {
                        example->cloth_example_cols = get_slider_setting("Columns");
                        example->cloth_example_rows = get_slider_setting("Rows");
                    }

                    step_count = 0;
                    step_final = 0.0;

                }

                else if (event.key.keysym.scancode == SDL_SCANCODE_U) {
                    example->draw_ui = !example->draw_ui;
                }

                else if (event.key.keysym.scancode == SDL_SCANCODE_PERIOD) {
                    frame_by_frame = !frame_by_frame;
                }

                else if (event.key.keysym.scancode == SDL_SCANCODE_SLASH) {
                    next_frame = true;
                }

                else if (event.key.keysym.scancode == SDL_SCANCODE_ESCAPE) {
                    is_running = false;
                }
            }
        }

        example->mouse.after_zoom = screen_to_world(example, NV_VEC2(example->mouse.x, example->mouse.y));

        if (example->mouse.middle) {
            example->camera = nvVector2_sub(example->camera, nvVector2_div(nvVector2_sub(NV_VEC2(example->mouse.x, example->mouse.y), example->pan_start), example->zoom));
            example->pan_start = NV_VEC2(example->mouse.x, example->mouse.y);
        }

        example->camera = nvVector2_add(example->camera, nvVector2_sub(example->mouse.before_zoom, example->mouse.after_zoom));

        mouse_body->position = example->mouse.before_zoom;
        mouse_body->_cache_aabb = false;

        // Call example callback if there is one
        if (!frame_by_frame || (frame_by_frame && next_frame)) {
            if (example_entries[current_example].update_callback != NULL)
                example_entries[current_example].update_callback(example);
        }

        render_time_start = SDL_GetPerformanceCounter();

        // Clear display
        SDL_SetRenderDrawColor(
            example->renderer,
            example->bg_color.r,
            example->bg_color.g,
            example->bg_color.b,
            255
        );
        SDL_RenderClear(example->renderer);

        if (example->switches[6]->on) {
            switch (example->space->broadphase_algorithm) {
                case nvBroadPhaseAlg_BRUTE_FORCE:
                    break;

                case nvBroadPhaseAlg_SPATIAL_HASH_GRID:
                    draw_SHG(example, font);
                    break;

                case nvBroadPhaseAlg_BOUNDING_VOLUME_HIERARCHY:
                    nvBVHNode *bvh_tree = nvBVHTree_new(example->space->bodies);
                    draw_BVH(example, bvh_tree);
                    nvBVHTree_free(bvh_tree);
                    break;
            }
        }

        if (!strcmp(example_entries[current_example].name, "Cloth")) {
            draw_cloth(example);
        }
        else {
            draw_bodies(example, font);
        }

        draw_constraints(example);

        // Draw the constraint between selected object and mouse
        if (selected) {
            SDL_SetRenderDrawColor(
                example->renderer,
                example->alt_text_color.r,
                example->alt_text_color.g,
                example->alt_text_color.b,
                example->alt_text_color.a
            );

            // Transform selection anchor point to world space
            nvVector2 anchor = nvVector2_rotate(selected_posf, selected->angle);
            anchor = world_to_screen(example, nvVector2_add(selected->position, anchor));
            nvVector2 p = world_to_screen(example, mouse_body->position);

            if (example->switches[0]->on) {
                draw_aaline(
                    example->renderer,
                    p.x,
                    p.y,
                    anchor.x,
                    anchor.y
                );
            }
            else {
                SDL_RenderDrawLineF(
                    example->renderer,
                    p.x,
                    p.y,
                    anchor.x,
                    anchor.y
                );
            }
        }

        draw_ui(example, font);

        // Sanity control
        if (example->cached_texts->count > 5000) {
            size_t cached_i = 0;
            void *cached_val;
            while (nvHashMap_iter(example->cached_texts, &cached_i, &cached_val)) {
                CachedText *cached_text = (CachedText *)cached_val;

                SDL_DestroyTexture(cached_text->texture);
                free(cached_text->string);
            }

            nvHashMap_clear(example->cached_texts);
        }
        else {
            nvArray *pending_delete = nvArray_new();

            nv_uint64 current_time = time(NULL);
            size_t cached_i = 0;
            void *cached_val;
            while (nvHashMap_iter(example->cached_texts, &cached_i, &cached_val)) {
                CachedText *cached_text = (CachedText *)cached_val;

                if (current_time - cached_text->last_access > 5) {
                    SDL_DestroyTexture(cached_text->texture);
                    // WHY CAN'T I FREE THE STRING HERE ???
                    // free(cached_text->string);
                    nvArray_add(pending_delete, cached_text->string);
                    nvHashMap_remove(example->cached_texts, cached_text);
                    
                    cached_i = 0;
                }
            }

            nvArray_free_each(pending_delete, free);
            nvArray_free(pending_delete);
        }

        // Calculate elapsed time during rendering
        render_time = SDL_GetPerformanceCounter() - render_time_start;
        render_time_f = (nv_float)render_time / frequency * 1000.0;
        example->render_time = render_time_f;
        example->render_counter += example->render_time;
        if (render_counter == 15) {
            example->render_avg = example->render_counter / (nv_float)render_counter;
            render_counter = 0;
            example->render_counter = 0.0;
        }


        // Advance the simulation
        // The only reason of advancing the simulation after rendering is
        // to render contact points more visible. Ideally the main loop
        // would look like: events -> update -> render -> loop
        if (!frame_by_frame || (frame_by_frame && next_frame)) {
            step_time_start = SDL_GetPerformanceCounter();

            nvSpace_step(
                example->space,
                1.0 / example->sliders[4]->value,
                (int)example->sliders[0]->value,
                (int)example->sliders[1]->value,
                (int)example->sliders[2]->value,
                (int)example->sliders[3]->value
            );

            step_time_end = SDL_GetPerformanceCounter() - step_time_start;
            step_time_f = (nv_float)step_time_end / frequency * 1000.0;
            example->step_time = step_time_f;
            example->step_counter += example->step_time;
            step_final += example->step_time;
            if (step_counter == 15) {
                example->step_avg = example->step_counter / (double)step_counter;
                step_counter = 0;
                example->step_counter = 0.0;
            }
        }

        if (example->space->after_collision != NULL)
            example->space->after_collision(example->space->res, example->space->callback_user_data);

        // Update the display
        SDL_RenderPresent(example->renderer);

        // Sync current fps with max_fps
        end_perf = SDL_GetTicks64();

        Uint64 frame = end_perf - start_perf;

        frames++;
        if (frames == fps_every_f) {
            end_perf_hi = SDL_GetPerformanceCounter();
            nv_float start = (nv_float)start_perf_hi / frequency;
            nv_float end = (nv_float)end_perf_hi / frequency;

            example->fps = (nv_float)fps_every_f / (end - start);

            frames = 0;
            start_perf_hi = SDL_GetPerformanceCounter();
        }

        if (frame < (1000 / example->max_fps)) {
            SDL_Delay((1000 / example->max_fps) - frame);
        }

        if (!frame_by_frame || (frame_by_frame && next_frame)) 
            example->counter++;
        step_counter++;
        step_count++;
        render_counter++;
        frame_counter++;
    }
}


#endif