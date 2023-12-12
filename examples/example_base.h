/*

  This file is a part of the Nova Physics Engine
  project and distributed under the MIT license.

  Copyright © Kadir Aksoy
  https://github.com/kadir014/nova-physics

*/

#include <stdbool.h>
#include <stdint.h>
#include <time.h>
#include <math.h>
#include <string.h> // Required on OSX for some reason
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_image.h>
#include "novaphysics/novaphysics.h"


/**
 * @file example_base.h
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
 * load_image
 * 
 * Drawing functions:
 * ------------------
 * hsv_to_rgb
 * draw_circle
 * fill_circle
 * draw_polygon
 * draw_aaline
 * draw_aapolygon
 * draw_aacircle
 * draw_text
 * draw_text_from_right
 * draw_spring
 * load_image
 * draw_image
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
 * draw_SHG
 * draw_BVH
 * UI elements update & draw
 */


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
 * @brief Load image from path.
 * 
 * @param renderer SDL Renderer
 * @param path Filepath to image
 * @return SDL_Texture *
 */
SDL_Texture *load_image(SDL_Renderer *renderer, char *path) {
    SDL_Texture *texture = IMG_LoadTexture(renderer, path);
    if (texture == NULL) {
        printf("Unable to load image at %s. Error: %s\n", path, IMG_GetError());
        exit(1);
    }

    return texture;
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
            va.x * 10.0, va.y * 10.0,
            vb.x * 10.0, vb.y * 10.0
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
    SDL_SetRenderDrawColor(renderer, r, g, b, a * 255);
    SDL_RenderDrawPointF(renderer, x, y);
}

static inline void _aa_pixel4(
    SDL_Renderer *renderer,
    nv_float x,
    nv_float y,
    nv_float dx,
    nv_float dy,
    nv_float alpha,
    uint8_t r,
    uint8_t g,
    uint8_t b
) {
    SDL_SetRenderDrawColor(renderer, r, g, b, alpha);
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
    
    uint8_t r, g, b, a;
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
            va.x * 10.0, va.y * 10.0,
            vb.x * 10.0, vb.y * 10.0
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
    uint8_t r,
    uint8_t g,
    uint8_t b
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
    TTF_Font *font,
    SDL_Renderer *renderer,
    char *text,
    int x,
    int y,
    SDL_Color color
) {
    SDL_Surface *text_surf = TTF_RenderText_Blended(font, text, color);
    SDL_Texture *text_tex = SDL_CreateTextureFromSurface(renderer, text_surf);

    int width, height;
    SDL_QueryTexture(text_tex, NULL, NULL, &width, &height);

    SDL_Rect text_rect = {x, y, width, height};

    SDL_RenderCopy(renderer, text_tex, NULL, &text_rect);

    SDL_FreeSurface(text_surf);
    SDL_DestroyTexture(text_tex);
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
    TTF_Font *font,
    SDL_Renderer *renderer,
    char *text,
    int x,
    int y,
    SDL_Color color
) {
    SDL_Surface *text_surf = TTF_RenderText_Blended(font, text, color);
    SDL_Texture *text_tex = SDL_CreateTextureFromSurface(renderer, text_surf);

    int width, height;
    SDL_QueryTexture(text_tex, NULL, NULL, &width, &height);

    SDL_Rect text_rect = {1280 - width - x, y, width, height};

    SDL_RenderCopy(renderer, text_tex, NULL, &text_rect);

    SDL_FreeSurface(text_surf);
    SDL_DestroyTexture(text_tex);
}

/**
 * @brief Draw spring.
 * 
 * @param renderer SDL Renderer
 * @param cons Constraint
 * @param aa Anit-aliasing
 * @param color Color
 */
void draw_spring(
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
        ap = nvVector2_mul(spring->anchor_a, 10.0);
    } else {
        nvVector2 ra = nvVector2_rotate(spring->anchor_a, cons->a->angle);
        ap = nvVector2_add(cons->a->position, ra);
        ap = nvVector2_mul(ap, 10.0);
    }
    if (cons->b == NULL) {
        bp = nvVector2_mul(spring->anchor_b, 10.0);
    } else {
        nvVector2 rb = nvVector2_rotate(spring->anchor_b, cons->b->angle);
        bp = nvVector2_add(cons->b->position, rb);
        bp = nvVector2_mul(bp, 10.0);
    }

    nvVector2 delta = nvVector2_sub(bp, ap);
    nvVector2 dir = nvVector2_normalize(delta);
    nv_float dist = nvVector2_len(delta);
    nv_float offset = (dist - spring->length * 10.0) / (spring->length * 10.0);
    nv_float steps = NV_PI / 3.0;
    nv_float stretch = 1.0 + offset;

    if (aa) {
        draw_aacircle(
            renderer,
            ap.x, ap.y,
            3.0,
            color.r, color.g, color.b
        );

        draw_aacircle(
            renderer,
            bp.x, bp.y,
            3.0,
            color.r, color.g, color.b
        );
    }
    else {
        draw_circle(
            renderer,
            ap.x, ap.y,
            3.0
        );

        draw_circle(
            renderer,
            bp.x, bp.y,
            3.0
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
 * @brief Draw image at given position.
 * 
 * @param renderer SDL Renderer
 * @param texture SDL Texture
 * @param x Position X
 * @param y Position Y
 * @param angle Angle
 */
void draw_image(
    SDL_Renderer *renderer,
    SDL_Texture *texture,
    int x,
    int y,
    nv_float angle
) {
    int width = 90;
    int height = 90;
    
    SDL_Rect rect = {
        x-width/2, y-height/2,
        width, height
    };
    
    SDL_RenderCopyEx(
        renderer,
        texture,
        NULL,
        &rect,
        angle,
        &(SDL_Point){width/2, width/2},
        SDL_FLIP_NONE
    );
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
} Slider;


/**
 * @brief Example visual theme enum.
 */
typedef enum {
    ExampleTheme_LIGHT, /**< Light theme. */
    ExampleTheme_DARK /**< Dark theme. */
} ExampleTheme;


struct _Example;

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
    const uint8_t *keys; /**< Array of pressed keys. */

    nv_float max_fps; /**< Targe FPS. */
    nv_float fps; /**< Current FPS. */
    nv_float dt; /**< Delta-time. */

    nvSpace *space; /**< Nova Physics space instance. */
    nv_float hertz; /**< Simulation hertz. */

    bool step; /**< Whether to step the simulation or not. */

    size_t switch_count; /**< Count of toggle switches. */
    ToggleSwitch **switches; /**< Array of toggle switches. */

    size_t slider_count; /**< Count of sliders. */
    Slider **sliders; /**< Array of sliders. */

    Example_callback update_callback; /**< Example update callback, called every tick. */
    Example_callback setup_callback; /**< Example setup callback, called once at start. */

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
    SDL_Color velocity_color;

    bool draw_ui; /**< Whether to draw the UI or not. */

    bool record;

    nvArray *sprites;

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
};

typedef struct _Example Example;

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

        nv_float radius = 2.5;

        nvVector2 cp;
        SDL_Color color;

        if (res->contact_count == 1) {
            nvContact contact = res->contacts[0];
            cp = nvVector2_mul(contact.position, 10.0);

            if (
                nvVector2_dist2(
                    NV_VEC2(example->mouse.x, example->mouse.y),
                    cp
                ) < 5 * 5
            ) {
                color = (SDL_Color){181, 242, 75, 255};

                if (example->mouse.right) {
                    nv_print_Resolution(res);

                    nv_float ax = res->a->position.x * 10.0;
                    nv_float ay = res->a->position.y * 10.0;
                    nv_float bx = res->b->position.x * 10.0;
                    nv_float by = res->b->position.y * 10.0;
                    draw_aacircle(example->renderer, ax, ay, 5, color.r, color.g, color.b);
                    draw_aacircle(example->renderer, bx, by, 5, color.r, color.g, color.b);
                }

            } else {
                color = (SDL_Color){242, 75, 81, 255};
            }

            if (res->state == 2) {
                color = (SDL_Color){227, 208, 98};
            }

            draw_aacircle(example->renderer, cp.x, cp.y, radius, color.r, color.g, color.b);
        }

        else if (res->contact_count == 2) {
            nvContact contact1 = res->contacts[0];
            nvContact contact2 = res->contacts[1];

            nvVector2 c1 = nvVector2_mul(contact1.position, 10.0);
            nvVector2 c2 = nvVector2_mul(contact2.position, 10.0);

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
                color = (SDL_Color){227, 208, 98};
            }

            draw_aacircle(example->renderer, c1.x, c1.y, radius, color.r, color.g, color.b);

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
                color = (SDL_Color){227, 208, 98};
            }

            draw_aacircle(example->renderer, c2.x, c2.y, radius, color.r, color.g, color.b);
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

    // Initialize PNG loading
    if (!(IMG_Init(IMG_INIT_PNG) & IMG_INIT_PNG)) {
        printf("SDL2_image could not be initialized. Error: %s\n", IMG_GetError());
        exit(1);
    }

    // Enable linear filtering for textures
    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "1");

    example->width = width;
    example->height = height;

    example->window = SDL_CreateWindow(
        title,
        SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
        width, height,
        SDL_WINDOW_SHOWN
    );

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

    example->mouse = (Mouse){0, 0, 0.0, 0.0, false, false, false};
    example->keys = SDL_GetKeyboardState(NULL);

    example->max_fps = max_fps;
    example->fps = max_fps;
    example->dt = 1.0 / max_fps;

    example->space = nvSpace_new();
    example->hertz = hertz;

    example->step = true;

    example->space->callback_user_data = example;
    example->space->after_collision = after_callback;

    example->update_callback = NULL;
    example->setup_callback = NULL;

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
        example->velocity_color = (SDL_Color){197, 255, 71, 255};
    }

    example->sprites = nvArray_new();

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

    example->record = false;

    return example;
}

/**
 * @brief Free space allocated by example.
 * 
 * @param example Example to free
 */
void Example_free(Example *example) {
    SDL_DestroyWindow(example->window);
    example->window = NULL;

    SDL_DestroyRenderer(example->renderer);
    example->renderer = NULL;

    example->keys = NULL;

    nvSpace_free(example->space);
    example->space = NULL;

    nvArray_free_each(example->sprites, (void (*)(void *))SDL_DestroyTexture);
    nvArray_free(example->sprites);

    example->update_callback = NULL;

    free(example->sliders);
    free(example->switches);

    free(example);
}



/******************************************************************************

                             Main loop functions

******************************************************************************/



/**
 * @brief Render UI.
 */
void draw_ui(Example *example, TTF_Font *font) {
    if (example->draw_ui) {
        SDL_SetRenderDrawColor(example->renderer, 0, 0, 0, 115);
        SDL_RenderFillRect(example->renderer, &(SDL_Rect){0, 0, 250, example->height});
    }

    // font size + 4 px
    int y_gap = 12 + 4;

    char text_fps[32];
    sprintf(text_fps, "FPS: %.1f", example->fps);

    char text_steptime[32];
    sprintf(text_steptime, "Physics: %.2fms", example->step_time);

    char text_rendertime[32];
    sprintf(text_rendertime, "Render: %.2fms", example->render_time);

    draw_text(font, example->renderer, text_fps, 5, 5 + (y_gap*0), example->text_color);
    draw_text(font, example->renderer, text_steptime, 5, 5 + (y_gap*1), example->text_color);
    draw_text(font, example->renderer, text_rendertime, 5, 5 + (y_gap*2), example->text_color);

    if (!example->draw_ui) {
        char text_savg[24];
        sprintf(text_savg, "Avg: %.2fms", example->step_avg);

        char text_ravg[24];
        sprintf(text_ravg, "Avg: %.2fms", example->render_avg);

        draw_text(font, example->renderer, text_savg, 120, 5 + (y_gap*1), example->text_color);
        draw_text(font, example->renderer, text_ravg, 120, 5 + (y_gap*2), example->text_color);

        return;
    }

    struct SDL_version sdl_ver;
    SDL_GetVersion(&sdl_ver);
    char text_sdlver[32];
    sprintf(text_sdlver, "SDL %d.%d.%d", sdl_ver.major, sdl_ver.minor, sdl_ver.patch);

    char text_novaver[32];
    sprintf(text_novaver, "Nova Physics %s", NV_VERSTR);

    char *text_instr0 = "1 meter = 10 pixels";
    char *text_instr = "Click & drag bodies";
    char *text_instr1 = "Restart with [R]";
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

    char text_iters_f[8];
    sprintf(text_iters_f, "%d", (int)example->sliders[0]->value);

    char text_citers_f[8];
    sprintf(text_citers_f, "%d", (int)example->sliders[1]->value);

    char text_cciters_f[8];
    sprintf(text_cciters_f, "%d", (int)example->sliders[2]->value);

    char text_subs_f[8];
    sprintf(text_subs_f, "%d", (int)example->sliders[3]->value);

    char text_hertz_f[32];
    sprintf(text_hertz_f, "%d/sec", (int)example->sliders[4]->value);

    nv_float unit_multipler = 1000.0;
    char unit_char = 'm';
    if (!example->switches[10]->on) {
        unit_multipler = 1000000.0;
        unit_char = 'u';
    }


    char text_profiler0[32];
    sprintf(text_profiler0, "Step:             %.2f%cs", example->space->profiler.step * unit_multipler, unit_char);

    char text_profiler1[32];
    sprintf(text_profiler1, "Integrate accel.: %.2f%cs", example->space->profiler.integrate_accelerations * unit_multipler, unit_char);

    char text_profiler2[32];
    sprintf(text_profiler2, "Broad-phase:      %.2f%cs", example->space->profiler.broadphase * unit_multipler, unit_char);

    char text_profiler3[32];
    sprintf(text_profiler3, "Narrow-phase:     %.2f%cs", example->space->profiler.narrowphase * unit_multipler, unit_char);

    char text_profiler4[32];
    sprintf(text_profiler4, "Presolve colls.:  %.2f%cs", example->space->profiler.presolve_collisions * unit_multipler, unit_char);

    char text_profiler5[32];
    sprintf(text_profiler5, "Solve positions:  %.2f%cs", example->space->profiler.solve_positions * unit_multipler, unit_char);

    char text_profiler6[32];
    sprintf(text_profiler6, "Solve velocities: %.2f%cs", example->space->profiler.solve_velocities * unit_multipler, unit_char);

    char text_profiler7[32];
    sprintf(text_profiler7, "Presolve consts.: %.2f%cs", example->space->profiler.presolve_constraints * unit_multipler, unit_char);

    char text_profiler8[32];
    sprintf(text_profiler8, "Solve consts.:    %.2f%cs", example->space->profiler.solve_constraints * unit_multipler, unit_char);

    char text_profiler9[32];
    sprintf(text_profiler9, "Integrate vels.:  %.2f%cs", example->space->profiler.integrate_velocities * unit_multipler, unit_char);

    char text_profiler10[32];
    sprintf(text_profiler10, "Remove bodies:    %.2f%cs", example->space->profiler.remove_bodies * unit_multipler, unit_char);

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

    // Update and render toggle switches
    for (size_t i = 0; i < example->switch_count; i++) {
        ToggleSwitch *tg = example->switches[i];

        ToggleSwitch_update(example, tg);
        ToggleSwitch_draw(example, tg);
    }

    // Update and render sliders
    for (size_t i = 0; i < example->slider_count; i++) {
        Slider *s = example->sliders[i];
        Slider_update(example, s);
        Slider_draw(example, s);
    }

    draw_text_from_right(font, example->renderer, text_sdlver, 5, 5 + (y_gap*0), example->text_color);
    draw_text_from_right(font, example->renderer, text_novaver, 5, 5 + (y_gap*1), example->text_color);
    draw_text_from_right(font, example->renderer, text_instr0, 5, 56 + (y_gap*0), example->alt_text_color);
    draw_text_from_right(font, example->renderer, text_instr, 5, 56 + (y_gap*1), example->alt_text_color);
    draw_text_from_right(font, example->renderer, text_instr1, 5, 56 + (y_gap*2), example->alt_text_color);
    draw_text_from_right(font, example->renderer, text_instr2, 5, 56 + (y_gap*3), example->alt_text_color);
    draw_text_from_right(font, example->renderer, text_instr3, 5, 56 + (y_gap*4), example->alt_text_color);
    draw_text_from_right(font, example->renderer, text_instr4, 5, 56 + (y_gap*5), example->alt_text_color);
    draw_text_from_right(font, example->renderer, text_instr5, 5, 56 + (y_gap*6), example->alt_text_color);

    draw_text(font, example->renderer, text_bodies, 123, 5 + (y_gap*0), example->text_color);
    draw_text(font, example->renderer, text_consts, 123, 5 + (y_gap*1), example->text_color);
    draw_text(font, example->renderer, text_attrs, 123, 5 + (y_gap*2), example->text_color);
    draw_text(font, example->renderer, text_ress, 123, 5 + (y_gap*3), example->text_color);

    draw_text(font, example->renderer, text_iters, 5, 10 + (y_gap*16), example->text_color);
    draw_text(font, example->renderer, text_citers, 5, 15 + (y_gap*17), example->text_color);
    draw_text(font, example->renderer, text_cciters, 5, 20 + (y_gap*18), example->text_color);
    draw_text(font, example->renderer, text_subs, 5, 25 + (y_gap*19), example->text_color);
    draw_text(font, example->renderer, text_hertz, 5, 30 + (y_gap*20), example->text_color);
    draw_text(font, example->renderer, text_iters_f, 196, 10 + (y_gap*16), example->text_color);
    draw_text(font, example->renderer, text_citers_f, 196, 15 + (y_gap*17), example->text_color);
    draw_text(font, example->renderer, text_cciters_f, 196, 20 + (y_gap*18), example->text_color);
    draw_text(font, example->renderer, text_subs_f, 196, 25 + (y_gap*19), example->text_color);
    draw_text(font, example->renderer, text_hertz_f, 196, 30 + (y_gap*20), example->text_color);

    draw_text(font, example->renderer, text_aa, 5, 10 + (y_gap*5), example->text_color);
    draw_text(font, example->renderer, text_fs, 5, 10 + (y_gap*6), example->text_color);
    draw_text(font, example->renderer, text_da, 5, 10 + (y_gap*7), example->text_color);
    draw_text(font, example->renderer, text_dc, 5, 10 + (y_gap*8), example->text_color);
    draw_text(font, example->renderer, text_dd, 5, 10 + (y_gap*9), example->text_color);
    draw_text(font, example->renderer, text_dj, 5, 10 + (y_gap*10), example->text_color);
    draw_text(font, example->renderer, text_dv, 5, 10 + (y_gap*11), example->text_color);
    draw_text(font, example->renderer, text_dg, 5, 10 + (y_gap*12), example->text_color);
    draw_text(font, example->renderer, text_s,  5, 10 + (y_gap*13), example->text_color);
    draw_text(font, example->renderer, text_ws, 5, 10 + (y_gap*14), example->text_color);

    int profiler_y = 150;

    draw_text(font, example->renderer, "Show in milliseconds", 5, profiler_y + (y_gap*15), example->text_color);
    draw_text(font, example->renderer, text_profiler0, 5, profiler_y + (y_gap*16), example->text_color);
    draw_text(font, example->renderer, text_profiler1, 5, profiler_y + (y_gap*17), example->text_color);
    draw_text(font, example->renderer, text_profiler2, 5, profiler_y + (y_gap*18), example->text_color);
    draw_text(font, example->renderer, text_profiler3, 5, profiler_y + (y_gap*19), example->text_color);
    draw_text(font, example->renderer, text_profiler4, 5, profiler_y + (y_gap*20), example->text_color);
    draw_text(font, example->renderer, text_profiler5, 5, profiler_y + (y_gap*21), example->text_color);
    draw_text(font, example->renderer, text_profiler6, 5, profiler_y + (y_gap*22), example->text_color);
    draw_text(font, example->renderer, text_profiler7, 5, profiler_y + (y_gap*23), example->text_color);
    draw_text(font, example->renderer, text_profiler8, 5, profiler_y + (y_gap*24), example->text_color);
    draw_text(font, example->renderer, text_profiler9, 5, profiler_y + (y_gap*25), example->text_color);
    draw_text(font, example->renderer, text_profiler10, 5, profiler_y + (y_gap*26), example->text_color);
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
                        a = nvVector2_mul(dist_joint->anchor_a, 10.0);
                    } else {
                        ra = nvVector2_rotate(dist_joint->anchor_a, cons->a->angle);
                        a = nvVector2_add(cons->a->position, ra);
                        a = nvVector2_mul(a, 10.0);
                    }
                    if (cons->b == NULL) {
                        b = nvVector2_mul(dist_joint->anchor_b, 10.0);
                    } else {
                        rb = nvVector2_rotate(dist_joint->anchor_b, cons->b->angle);
                        b = nvVector2_add(cons->b->position, rb);
                        b = nvVector2_mul(b, 10.0);
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
                            2.0,
                            example->distancejoint_color.r,
                            example->distancejoint_color.g,
                            example->distancejoint_color.b
                        );

                        draw_aacircle(
                            example->renderer,
                            b.x, b.y,
                            2.0,
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
                            2.0
                        );

                        draw_circle(
                            example->renderer,
                            b.x, b.y,
                            2.0
                        );
                    }

                    break;

                case nvConstraintType_HINGEJOINT:
                    hinge_joint = (nvHingeJoint *)cons->def;

                    if (cons->a)
                        a = nvVector2_mul(
                            nvVector2_add(
                                nvVector2_rotate(hinge_joint->anchor_a, cons->a->angle), cons->a->position), 10.0);
                    else
                        a = nvVector2_mul(hinge_joint->anchor, 10.0);
                    if (cons->b)
                        b = nvVector2_mul(
                            nvVector2_add(
                                nvVector2_rotate(hinge_joint->anchor_b, cons->b->angle), cons->b->position), 10.0);
                    else
                        b = nvVector2_mul(hinge_joint->anchor, 10.0);
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
                            5.0,
                            example->hingejoint_color.r,
                            example->hingejoint_color.g,
                            example->hingejoint_color.b
                        );

                        draw_aacircle(
                            example->renderer,
                            a.x, a.y,
                            2.5,
                            example->hingejoint_color.r,
                            example->hingejoint_color.g,
                            example->hingejoint_color.b
                        );

                        draw_aacircle(
                            example->renderer,
                            b.x, b.y,
                            2.5,
                            example->hingejoint_color.r,
                            example->hingejoint_color.g,
                            example->hingejoint_color.b
                        );
                    }
                    else {
                        draw_circle(
                            example->renderer,
                            ra.x, ra.y,
                            5.0
                        );

                        draw_circle(
                            example->renderer,
                            a.x, a.y,
                            2.5
                        );

                        draw_circle(
                            example->renderer,
                            b.x, b.y,
                            2.5
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

        // Draw sprites
        bool draw_sprite = false;
        if (i < example->sprites->size) {
            if (example->sprites->data[i] != NULL) {
                nv_float x = body->position.x * 10.0;
                nv_float y = body->position.y * 10.0;

                draw_image(
                    example->renderer,
                    (SDL_Texture *)example->sprites->data[i],
                    x, y,
                    body->angle * (180.0 / NV_PI)
                );

                draw_sprite = true;
            }
        }

        // Draw AABB
        if (example->switches[1]->on) {
            nvAABB aabb = nvBody_get_aabb(body);

            SDL_FRect aabb_rect = (SDL_FRect){
                aabb.min_x*10.0,
                aabb.min_y*10.0,
                (aabb.max_x-aabb.min_x)*10.0,
                (aabb.max_y - aabb.min_y)*10.0
            };

            SDL_SetRenderDrawColor(
                example->renderer,
                example->aabb_color.r,
                example->aabb_color.g,
                example->aabb_color.b,
                example->aabb_color.a
                );
            SDL_RenderDrawRectF(example->renderer, &aabb_rect);
        }

        // Draw threading split indicators
        // if (example->space->multithreading) {
        //     nv_float q = example->space->shg->bounds.max_x / 4.0;
        //     nvVector2 pos = nvVector2_mul(body->position, 10.0);
        //     int px = pos.x;
        //     int py = pos.y;

        //     if (
        //         body->position.x > example->space->shg->bounds.min_x &&
        //         body->position.x <= q * 1.0
        //     ) {
        //         SDL_SetRenderDrawColor(example->renderer, 255, 0, 0, 255);
        //     }
        //     else if (
        //         body->position.x > q * 1.0 &&
        //         body->position.x <= q * 2.0
        //     ) {
        //         SDL_SetRenderDrawColor(example->renderer, 255, 255, 0, 255);
        //     }
        //     else if (
        //         body->position.x > q * 2.0 &&
        //         body->position.x <= q * 3.0
        //     ) {
        //         SDL_SetRenderDrawColor(example->renderer, 0, 255, 0, 255);
        //     }
        //     else if (
        //         body->position.x > q * 3.0 &&
        //         body->position.x <= example->space->shg->bounds.max_x
        //     ) {
        //         SDL_SetRenderDrawColor(example->renderer, 0, 180, 255, 255);
        //     }

        //     draw_circle(example->renderer, px, py, 2);
        // }

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

        int r = (body->id) % 5;
        SDL_Color color;

        if (r == 0) color = (SDL_Color){255, 212, 0, 255};
        if (r == 1) color = (SDL_Color){70, 51, 163, 255};
        if (r == 2) color = (SDL_Color){234, 222, 218, 255};
        if (r == 3) color = (SDL_Color){217, 3, 104, 255};
        if (r == 4) color = (SDL_Color){130, 2, 99, 255};

        // Draw circle bodies
        if (!draw_sprite) {
            if (body->shape->type == nvShapeType_CIRCLE) {
                nv_float x = body->position.x * 10.0;
                nv_float y = body->position.y * 10.0;

                if (example->switches[0]->on) {
                    draw_aacircle(
                        example->renderer,
                        x, y,
                        body->shape->radius * 10.0,
                        aacolor.r,
                        aacolor.g,
                        aacolor.b
                    );

                    if (example->switches[3]->on) {
                        nvVector2 a = (nvVector2){body->shape->radius*10.0, 0.0};
                        a = nvVector2_rotate(a, body->angle);

                        draw_aaline(example->renderer, x, y, x+a.x, y+a.y);
                    }
                }
                else if (example->switches[9]->on) {
                    int n = 12;
                    SDL_Vertex *vertices = malloc(sizeof(SDL_Vertex) * n);

                    nvVector2 arm = NV_VEC2(body->shape->radius, 0.0);
                    nvVector2 trans;

                    for (size_t i = 0; i < n; i++) {
                        arm = nvVector2_rotate(arm, 2.0 * NV_PI / (nv_float)n);
                        trans = nvVector2_add(body->position, arm);

                        vertices[i] = (SDL_Vertex){
                            .color = color,
                            .position = (SDL_FPoint){trans.x * 10.0, trans.y * 10.0},
                            .tex_coord = (SDL_FPoint){0.0, 0.0}
                        };
                    }

                    int indices[] = {0, 2, 1, 0, 3, 2, 0, 4, 3, 0, 5, 4, 0, 6, 5, 0, 7, 6, 0, 8, 7, 0, 9, 8, 0, 10, 9, 0, 11, 10};

                    SDL_RenderGeometry(example->renderer, NULL, vertices, n, indices, 30);
                    free(vertices);

                    if (example->switches[3]->on) {
                        nvVector2 a = (nvVector2){body->shape->radius*10.0, 0.0};
                        a = nvVector2_rotate(a, body->angle);

                        SDL_SetRenderDrawColor(example->renderer, example->body_color.r, example->body_color.g, example->body_color.b, 255);

                        SDL_RenderDrawLineF(example->renderer, x, y, x+a.x, y+a.y);
                    }
                }
                else {
                    int32_t draw_radius = (int32_t)(body->shape->radius * 10.0);
                    draw_circle(
                        example->renderer,
                        (int32_t)x,
                        (int32_t)y,
                        draw_radius
                    );

                    if (example->switches[3]->on) {
                        nvVector2 a = (nvVector2){body->shape->radius*10.0, 0.0};
                        a = nvVector2_rotate(a, body->angle);

                        SDL_RenderDrawLineF(example->renderer, x, y, x+a.x, y+a.y);
                    }
                }
            }

            // Draw polygon bodies
            else {
                nv_Polygon_model_to_world(body);

                if (example->switches[0]->on)
                    draw_aapolygon(example->renderer, body->shape->trans_vertices);

                else if (example->switches[9]->on) {
                    size_t n = body->shape->trans_vertices->size;

                    if (n == 3) {

                        SDL_Vertex *vertices = malloc(sizeof(SDL_Vertex) * n);

                        for (size_t j = 0; j < n; j++) {
                            nvVector2 v = NV_TO_VEC2(body->shape->trans_vertices->data[j]);

                            vertices[j] = (SDL_Vertex){
                                .color = color,
                                .position = (SDL_FPoint){v.x * 10.0, v.y * 10.0},
                                .tex_coord = (SDL_FPoint){0.0, 0.0}
                            };
                        }

                        SDL_RenderGeometry(example->renderer, NULL, vertices, n, NULL, 0);
                        free(vertices);
                    }

                    else if (n == 4) {

                        SDL_Vertex *vertices = malloc(sizeof(SDL_Vertex) * n);

                        for (size_t j = 0; j < n; j++) {
                            nvVector2 v = NV_TO_VEC2(body->shape->trans_vertices->data[j]);

                            vertices[j] = (SDL_Vertex){
                                .color = color,
                                .position = (SDL_FPoint){v.x * 10.0, v.y * 10.0},
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
                            nvVector2 v = NV_TO_VEC2(body->shape->trans_vertices->data[j]);

                            vertices[j] = (SDL_Vertex){
                                .color = color,
                                .position = (SDL_FPoint){v.x * 10.0, v.y * 10.0},
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
                            nvVector2 v = NV_TO_VEC2(body->shape->trans_vertices->data[j]);

                            vertices[j] = (SDL_Vertex){
                                .color = color,
                                .position = (SDL_FPoint){v.x * 10.0, v.y * 10.0},
                                .tex_coord = (SDL_FPoint){0.0, 0.0}
                            };
                        }

                        int indices[12] = {0, 2, 1, 0, 3, 2, 0, 4, 3, 0, 5, 4};

                        SDL_RenderGeometry(example->renderer, NULL, vertices, n, indices, 12);
                        free(vertices);
                    }
                }
                else
                    draw_polygon(example->renderer, body->shape->trans_vertices);

                if (example->switches[3]->on) {
                    nvVector2 center = nvVector2_mul(nv_polygon_centroid(body->shape->trans_vertices), 10.0);
                    nvVector2 diredge = nvVector2_mul(nvVector2_div(
                        nvVector2_add(
                            NV_TO_VEC2(body->shape->trans_vertices->data[0]),
                            NV_TO_VEC2(body->shape->trans_vertices->data[1])),
                        2.0), 10.0);

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
            }
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

            nvVector2 v = nvVector2_mul(nvVector2_add(body->position, vel), 10.0);

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
    }
}

/**
 * @brief Update ToggleSwitch object.
 */
void ToggleSwitch_update(struct _Example *example, ToggleSwitch *tg) {
    if (example->mouse.x < tg->x + tg->size && example->mouse.x > tg->x &&
        example->mouse.y < tg->y + tg->size && example->mouse.y > tg->y) {
        
        if (example->mouse.left && !tg->changed) {
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

    // Horizontal lines
    for (size_t y = 0; y < shg->rows; y++) {
        SDL_RenderDrawLine(
            example->renderer,
            0, y * shg->cell_height * 10.0,
            shg->cols * shg->cell_width * 10.0, y * shg->cell_height * 10.0
        );
    }

    // Vertical lines
    for (size_t x = 0; x < shg->cols; x++) {
        SDL_RenderDrawLine(
            example->renderer,
            x * shg->cell_width * 10.0, 0,
            x * shg->cell_width * 10.0, shg->rows * shg->cell_height * 10.0
        );
    }

    // Cell content texts
    for (size_t y = 0; y < shg->rows; y++) {
        for (size_t x = 0; x < shg->cols; x++) {
            nvArray *cell = nvSHG_get(shg, nv_pair(x, y));
            if (cell == NULL) continue;

            char text_cell[8];
            sprintf(text_cell, "%llu", (unsigned long long)cell->size);

            draw_text(
                font,
                example->renderer,
                text_cell,
                x * shg->cell_width * 10.0 + 3.0,
                y * shg->cell_height * 10.0 + 3.0,
                (SDL_Color){89, 89, 89, 255}
            );
        }
    }
}

void draw_BVH(Example *example, nvBVHNode *node) {
    SDL_SetRenderDrawColor(
            example->renderer,
            example->ui_color.r,
            example->ui_color.g,
            example->ui_color.b,
            example->ui_color.a
        );

    SDL_FRect aabb_rect = (SDL_FRect){
        node->aabb.min_x*10.0,
        node->aabb.min_y*10.0,
        (node->aabb.max_x - node->aabb.min_x)*10.0,
        (node->aabb.max_y - node->aabb.min_y)*10.0
    };

    SDL_RenderDrawRectF(example->renderer, &aabb_rect);

    if (!node->is_leaf) {
        if (node->left != NULL) draw_BVH(example, node->left);
        if (node->right != NULL) draw_BVH(example, node->right);
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

    nvBody *mouse_body = nv_Circle_new(
        nvBodyType_STATIC,
        nvVector2_zero,
        0.0,
        nvMaterial_WOOD,
        0.3
    );
    mouse_body->enable_collision = false;
    nvSpace_add(example->space, mouse_body);
    nvArray_add(example->sprites, NULL);

    nvBody *selected = NULL;
    nvConstraint *selected_const = NULL;
    nvVector2 selected_posf = nvVector2_zero;
    nvVector2 selected_pos = nvVector2_zero;

    int energy_tick = 0;

    TTF_Font *font;

    font = TTF_OpenFont("assets/FiraCode-Regular.ttf", 11);
    if (font == NULL) {
        printf("Couldn't load assets/FiraCode-Regular.ttf\n");
        return;
    }
    TTF_SetFontStyle(font, TTF_STYLE_NORMAL);
    TTF_SetFontOutline(font, 0);
    TTF_SetFontKerning(font, 1);
    TTF_SetFontHinting(font, TTF_HINTING_NORMAL);

    // MSVC doesn't allow variable length arrays
    size_t switches_n = 11;
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
        .x = 118+6, .y = 191+4+32-5,
        .size = 9, .on = false
    };

    switches[8] = &(ToggleSwitch){
        .x = 118+6, .y = 207+4+32-5,
        .size = 9, .on = true
    };
    
    switches[9] = &(ToggleSwitch){
        .x = 118+6, .y = 79+4+32-5,
        .size = 9, .on = false
    };

    switches[10] = &(ToggleSwitch){
        .x = 118+34, .y = 393,
        .size = 9, .on = true
    };

    example->switches = switches;
    example->switch_count = switches_n;

    size_t sliders_n = 5;
    Slider **sliders = malloc(sizeof(Slider) * sliders_n);

    int slider_offset = 25;

    sliders[0] = &(Slider){
        .x = 135-slider_offset, .y = 271,
        .width = 80,
        .min = 1, .max = 50, .value = 10,
    };
    sliders[0]->cx = sliders[0]->x + ((sliders[0]->value-sliders[0]->min) / (sliders[0]->max - sliders[0]->min)) * sliders[0]->width;

    sliders[1] = &(Slider){
        .x = 135-slider_offset, .y = 271 + (21*1),
        .width = 80,
        .min = 1, .max = 50, .value = 10,
    };
    sliders[1]->cx = sliders[1]->x + ((sliders[1]->value-sliders[1]->min) / (sliders[1]->max - sliders[1]->min)) * sliders[1]->width;

    sliders[2] = &(Slider){
        .x = 135-slider_offset, .y = 271 + (21*2),
        .width = 80,
        .min = 1, .max = 50, .value = 5,
    };
    sliders[2]->cx = sliders[2]->x + ((sliders[2]->value-sliders[2]->min) / (sliders[2]->max - sliders[2]->min)) * sliders[2]->width;

    sliders[3] = &(Slider){
        .x = 135-slider_offset, .y = 271 + (21*3),
        .width = 80,
        .min = 1, .max = 10, .value = 1,
    };
    sliders[3]->cx = sliders[3]->x + ((sliders[3]->value-sliders[3]->min) / (sliders[3]->max - sliders[3]->min)) * sliders[3]->width;

    sliders[4] = &(Slider){
        .x = 135-slider_offset, .y = 271 + (21*4),
        .width = 80,
        .min = 12.0, .max = 240.0, .value = 60.0,
    };
    sliders[4]->cx = sliders[4]->x + ((sliders[4]->value-sliders[4]->min) / (sliders[4]->max - sliders[4]->min)) * sliders[4]->width;

    example->sliders = sliders;
    example->slider_count = sliders_n;

    if (example->setup_callback != NULL)
        example->setup_callback(example);

    size_t step_counter = 0;
    size_t render_counter = 0;
    size_t frame_counter = 0;

    bool frame_by_frame = false;
    bool next_frame = false;

    while (is_running) {
        start_perf = SDL_GetTicks64();

        example->step = true;
        next_frame = false;

        // Handle events
        while(SDL_PollEvent(&event) != 0) {
            if (event.type == SDL_QUIT)
                is_running = false;

            else if (event.type == SDL_MOUSEMOTION) {
                SDL_GetMouseState(&example->mouse.x, &example->mouse.y);
                example->mouse.px = example->mouse.x / 10.0;
                example->mouse.py = example->mouse.y / 10.0;
                mouse_body->position = NV_VEC2(example->mouse.px, example->mouse.py);
            }

            else if (event.type == SDL_MOUSEBUTTONDOWN) {
                if (event.button.button == SDL_BUTTON_LEFT)
                    example->mouse.left = true;
                else if (event.button.button == SDL_BUTTON_MIDDLE)
                    example->mouse.middle = true;
                else if (event.button.button == SDL_BUTTON_RIGHT)
                    example->mouse.right = true;

                if (example->mouse.left) {
                    selected = NULL;
                    for (size_t i = 0; i < example->space->bodies->size; i++) {
                        nvBody *body = (nvBody *)example->space->bodies->data[i];
                        if (body->type == nvBodyType_STATIC) continue;

                        bool inside = false;
                        nvShape *shape = body->shape;

                        if (shape->type == nvShapeType_POLYGON) {
                            nv_Polygon_model_to_world(body);
                            inside = nv_collide_polygon_x_point(body, NV_VEC2(example->mouse.px, example->mouse.py));
                        }
                        else if (shape->type == nvShapeType_CIRCLE) {
                            inside = nv_collide_circle_x_point(body, NV_VEC2(example->mouse.px, example->mouse.py));
                        }

                        if (inside) {
                            selected = body;

                            // Transform mouse coordinatets to body local coordinates
                            selected_posf = (nvVector2){example->mouse.px, example->mouse.py};
                            selected_posf = nvVector2_sub(selected_posf, selected->position);
                            selected_posf = nvVector2_rotate(selected_posf, -selected->angle);

                            selected_pos = (nvVector2){selected_posf.x+0.00001, selected_posf.y+0.00001};

                            selected_const = nvSpring_new(
                                mouse_body, selected,
                                nvVector2_zero, selected_pos,
                                0.0, 150.0 * selected->mass / 3.0, 70.0 * selected->mass / 4.0
                            );

                            nvSpace_add_constraint(example->space, selected_const);

                            if (selected->is_sleeping) nvBody_awake(selected);

                            break;
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
                }
            }

            else if (event.type == SDL_MOUSEBUTTONUP) {
                if (event.button.button == SDL_BUTTON_LEFT) {
                    example->mouse.left = false;
                    selected = NULL;

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
                    }
                }
                else if (event.button.button == SDL_BUTTON_MIDDLE)
                    example->mouse.middle = false;
                else if (event.button.button == SDL_BUTTON_RIGHT)
                    example->mouse.right = false;
            }

            else if (event.type == SDL_KEYDOWN) {
                if (event.key.keysym.scancode == SDL_SCANCODE_Q) {
                    for (size_t i = 0; i < example->space->bodies->size; i++) {
                        nvBody *body = (nvBody *)example->space->bodies->data[i];
                        if (body->type == nvBodyType_STATIC) continue;

                        nvVector2 delta = nvVector2_sub(
                            body->position,
                            (nvVector2){example->mouse.px, example->mouse.py}
                        );

                        nv_float strength = 10.0 * pow(10.0, 3.0);

                        nvVector2 force = nvVector2_mul(delta, strength);
                        force = nvVector2_div(force, nvVector2_len(delta));

                        nvBody_apply_force(body, force);
                    }
                }

                else if (event.key.keysym.scancode == SDL_SCANCODE_R) {
                    selected = NULL;
                    nvArray_remove(example->space->constraints, selected_const);
                    nvConstraint_free(selected_const);
                    selected_const = NULL;

                    nvSpace_clear(example->space);
                    example->space->_id_counter = 0;

                    mouse_body = nv_Circle_new(
                        nvBodyType_STATIC,
                        nvVector2_zero,
                        0.0,
                        nvMaterial_WOOD,
                        0.3
                    );
                    mouse_body->enable_collision = false;
                    nvSpace_add(example->space, mouse_body);

                    example->counter = 0;

                    if (example->setup_callback != NULL)
                        example->setup_callback(example);

                    step_count = 0;
                    step_final = 0.0;

                }

                else if (event.key.keysym.scancode == SDL_SCANCODE_U) {
                    example->draw_ui = !example->draw_ui;
                }

                else if (event.key.keysym.scancode == SDL_SCANCODE_S) {
                    example->step = true;
                }

                else if (event.key.keysym.scancode == SDL_SCANCODE_P) {
                    frame_counter = 0;
                    example->record = true;
                    step_final = 0.0;
                    step_count = 0;
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

        // Call example callback if there is one
        if (example->update_callback != NULL)
            example->update_callback(example);

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

        draw_bodies(example, font);

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
            anchor = nvVector2_add(selected->position, anchor);

            if (example->switches[0]->on) {
                draw_aaline(
                    example->renderer,
                    mouse_body->position.x * 10.0,
                    mouse_body->position.y * 10.0,
                    anchor.x * 10.0,
                    anchor.y * 10.0
                );
            }
            else {
                SDL_RenderDrawLineF(
                    example->renderer,
                    mouse_body->position.x * 10.0,
                    mouse_body->position.y * 10.0,
                    anchor.x * 10.0,
                    anchor.y * 10.0
                );
            }
        }

        draw_ui(example, font);


        // Calculate elapsed time during rendering
        render_time = SDL_GetPerformanceCounter() - render_time_start;
        render_time_f = (nv_float)render_time / frequency * 1000.0;
        example->render_time = render_time_f;
        example->render_counter += example->render_time;
        if (render_counter == 15) {
            example->render_avg = example->render_counter / (double)render_counter;
            render_counter = 0;
            example->render_counter = 0.0;
        }


        // Advance the simulation
        // The only reason of advancing the simulation after rendering is
        // to render contact points more visible. Ideally the main loop
        // would look like: events -> update -> render -> loop

        if (!frame_by_frame || (frame_by_frame && next_frame)){
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

        // Save current frame
        if (example->record) {
            SDL_Surface *frame = SDL_CreateRGBSurface(
                0,
                example->width, example->height,
                32, 0x00ff0000, 0x0000ff00, 0x000000ff, 0xff000000
            );

            SDL_RenderReadPixels(
                example->renderer,
                NULL,
                SDL_PIXELFORMAT_ARGB8888,
                frame->pixels,
                frame->pitch
            );

            char filename[48];
            sprintf(filename, "../examples/recording/frame%llu.png", (unsigned long long)frame_counter);

            IMG_SavePNG(frame, filename);

            SDL_FreeSurface(frame);
        }


        // Calculate total energy
        energy_tick++;
        if (energy_tick == 10) {
            nv_float le;
            nv_float ae;
            example->total_ae = 0.0;
            example->total_le = 0.0;
            example->total_energy = 0.0;
            for (size_t i = 0; i < example->space->bodies->size; i++) {
                nvBody *body = (nvBody *)example->space->bodies->data[i];
                le = nvBody_get_kinetic_energy(body);
                ae = nvBody_get_rotational_energy(body);
                example->total_le += le;
                example->total_ae += ae;
                example->total_energy += le + ae;
            }

            energy_tick = 0;
        }


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

        example->counter++;
        step_counter++;
        step_count++;
        render_counter++;
        frame_counter++;
    }
}