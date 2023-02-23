/*

  This file is a part of the Nova Physics Engine
  project and distributed under the MIT license.

  Copyright © Kadir Aksoy
  https://github.com/kadir014/nova-physics

*/

#include <stdbool.h>
#include <time.h>
#include <math.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include "novaphysics/novaphysics.h"


/**
 * @file example_base.h
 * 
 * This header defines all things needed to setup & run
 * a basic Nova Phyiscs example with SDL2.
 */


#define MAX(x, y) (((x) > (y)) ? (x) : (y))

/**
 * @brief Return random integer in given range
 * 
 * @param lower Min range
 * @param higher Max range
 * @return int 
 */
int irand(int lower, int higher) {
    return (rand() % (higher - lower + 1)) + lower;
}

/**
 * @brief Return random double in given range
 * 
 * @param lower Min range
 * @param higher Max range
 * @return double 
 */
double frand(double lower, double higher) {
    double normal = rand() / (double)RAND_MAX;
    return lower + normal * (higher - lower);
}


/***********************************************

  Drawing functions
  -----------------
  - draw_circle:    Draw circle
  - draw_polygon:   Draw polygon
  - draw_aaline:    Draw anti-aliased line
  - draw_aapolygon: Draw anti-aliased polygon
  - draw_aacircle:  Draw anti-aliased circle
  - draw_text:      Draw text
  - draw_spring:    Draw spring with sine wave

***********************************************/


/**
 * @brief Draw circle
 *        Reference: https://discourse.libsdl.org/t/query-how-do-you-draw-a-circle-in-sdl2-sdl2/33379
 * 
 * @param renderer SDL Renderer
 * @param cx Circle center X
 * @param cy Circle center Y
 * @param radius Circle radius
 */
void draw_circle(
    SDL_Renderer *renderer,
    int32_t cx,
    int32_t cy,
    int32_t radius
) {
   const int32_t diameter = (radius * 2);

   int32_t x = (radius - 1);
   int32_t y = 0;
   int32_t tx = 1;
   int32_t ty = 1;
   int32_t error = (tx - diameter);

   while (x >= y)
   {
      //  Each of the following renders an octant of the circle
      SDL_RenderDrawPoint(renderer, cx + x, cy - y);
      SDL_RenderDrawPoint(renderer, cx + x, cy + y);
      SDL_RenderDrawPoint(renderer, cx - x, cy - y);
      SDL_RenderDrawPoint(renderer, cx - x, cy + y);
      SDL_RenderDrawPoint(renderer, cx + y, cy - x);
      SDL_RenderDrawPoint(renderer, cx + y, cy + x);
      SDL_RenderDrawPoint(renderer, cx - y, cy - x);
      SDL_RenderDrawPoint(renderer, cx - y, cy + x);

      if (error <= 0)
      {
         ++y;
         error += ty;
         ty += 2;
      }

      if (error > 0)
      {
         --x;
         tx += 2;
         error += (tx - diameter);
      }
   }
}

/**
 * @brief Draw polygon
 * 
 * @param renderer SDL Renderer
 * @param vertices Vertices
 */
void draw_polygon(SDL_Renderer *renderer, nv_Array *vertices) {
    size_t n = vertices->size;

    for (size_t i = 0; i < n; i++) {
        nv_Vector2 va = NV_TO_VEC2(vertices->data[i]);
        nv_Vector2 vb = NV_TO_VEC2(vertices->data[(i + 1) % n]);

        SDL_RenderDrawLineF(
            renderer,
            va.x * 10.0, va.y * 10.0,
            vb.x * 10.0, vb.y * 10.0
            );
    }
}

/**
 * Utility functions for drawing anti-aliased line
 */

void swap(double *a, double *b) {
    double temp = *a;
    *a = *b;
    *b = temp;
}

int ipart(double x) {
    return (int)x;
}

int fround(double x) {
    return ipart(x + 0.5);
}

double fpart(double x) {
    return x - ipart(x);
}

double rfpart(double x) {
    return 1.0 - fpart(x);
}

void pixel(
    SDL_Renderer *renderer,
    double x,
    double y,
    double a,
    int r,
    int g,
    int b
) {
    SDL_SetRenderDrawColor(renderer, r, g, b, a * 255);
    SDL_RenderDrawPointF(renderer, x, y);
}

/**
 * @brief Draw anti-aliased line
 *        Reference: https://en.wikipedia.org/wiki/Xiaolin_Wu%27s_line_algorithm
 * 
 * @param renderer SDL Renderer
 * @param x0 First point X
 * @param y0 First point Y
 * @param x1 Second point X
 * @param y1 Second point Y
 */
void draw_aaline(
    SDL_Renderer *renderer,
    double x0,
    double y0,
    double x1,
    double y1
) {
    bool steep = fabs(y1 - y0) > fabs(x1 - x0);
    
    Uint8 r, g, b, a;
    SDL_GetRenderDrawColor(renderer, &r, &g, &b, &a);

    if (steep) {
        swap(&x0, &y0);
        swap(&x1, &y1);
    }
    if (x0 > x1) {
        swap(&x0, &x1);
        swap(&y0, &y1);
    }

    double dx = x1 - x0;
    double dy = y1 - y0;

    double gradient;
    if (dx == 0.0) gradient = 1.0;
    else gradient = dy / dx;

    // Handle first endpoint
    int xend = fround(x0);
    double yend = y0 + gradient * (xend - x0);
    double xgap = rfpart(x0 + 0.5);
    int xpxl1 = xend; // For main loop
    int ypxl1 = ipart(yend);

    if (steep) {
        pixel(renderer, ypxl1,     xpxl1, rfpart(yend) * xgap, r, g, b);
        pixel(renderer, ypxl1 + 1, xpxl1,  fpart(yend) * xgap, r, g, b);
    }
    else {
        pixel(renderer, xpxl1, ypxl1,     rfpart(yend) * xgap, r, g, b);
        pixel(renderer, xpxl1, ypxl1 + 1,  fpart(yend) * xgap, r, g, b);
    }

    double intery = yend + gradient; // First Y intersection

    // Handle second endpoint
    xend = fround(x1);
    yend = y1 + gradient * (xend - x1);
    xgap = fpart(x1 + 0.5);
    int xpxl2 = xend; // For main loop
    int ypxl2 = ipart(yend);

    if (steep) {
        pixel(renderer, ypxl2,     xpxl2, rfpart(yend) * xgap, r, g, b);
        pixel(renderer, ypxl2 + 1, xpxl2,  fpart(yend) * xgap, r, g, b);
    }
    else {
        pixel(renderer, xpxl2, ypxl2,     rfpart(yend) * xgap, r, g, b);
        pixel(renderer, xpxl2, ypxl2 + 1,  fpart(yend) * xgap, r, g, b);
    }

    // Main loop
    if (steep) {
        for (int x = xpxl1 + 1; x <= xpxl2 - 1; x++) {
            pixel(renderer, ipart(intery),     x, rfpart(intery), r, g, b);
            pixel(renderer, ipart(intery) + 1, x,  fpart(intery), r, g, b);
            intery += gradient;
        }
    }
    else {
        for (int x = xpxl1 + 1; x <= xpxl2 - 1; x++) {
            pixel(renderer, x, ipart(intery),     rfpart(intery), r, g, b);
            pixel(renderer, x, ipart(intery) + 1,  fpart(intery), r, g, b);
            intery += gradient;
        }
    }
}

/**
 * @brief Render anti-aliased polygon
 * 
 * @param renderer SDL Renderer
 * @param vertices Vertices
 */
void draw_aapolygon(SDL_Renderer *renderer, nv_Array *vertices) {
    size_t n = vertices->size;

    for (size_t i = 0; i < n; i++) {
        nv_Vector2 va = NV_TO_VEC2(vertices->data[i]);
        nv_Vector2 vb = NV_TO_VEC2(vertices->data[(i + 1) % n]);

        draw_aaline(
            renderer,
            va.x * 10.0, va.y * 10.0,
            vb.x * 10.0, vb.y * 10.0
        );
    }
}

/**
 * Utility function for drawing anti-aliased circle
 */
void pixel4(
    SDL_Renderer *renderer,
    double x,
    double y,
    double dx,
    double dy,
    double alpha,
    Uint8 r,
    Uint8 g,
    Uint8 b
) {
    SDL_SetRenderDrawColor(renderer, r, g, b, alpha);
    SDL_RenderDrawPointF(renderer, x + dx, y + dy);
    SDL_RenderDrawPointF(renderer, x - dx, y + dy);
    SDL_RenderDrawPointF(renderer, x + dx, y - dy);
    SDL_RenderDrawPointF(renderer, x - dx, y - dy);
}

/**
 * @brief Draw anti-aliased circle
 *        Reference: https://create.stephan-brumme.com/antialiased-circle/#antialiased-circle-wu
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
    double cx,
    double cy,
    double radius,
    Uint8 r,
    Uint8 g,
    Uint8 b
) {
    // + 0.3 is for correction
    double rx = radius + 0.3;
    double ry = radius + 0.3;
    double rx2 = rx * rx;
    double ry2 = ry * ry;

    double max_alpha = 255.0;

    double q = fround(rx2 / sqrt(rx2 + ry2));
    for (double x = 0; x <= q; x++) {
        double y = ry * sqrt(1 - x * x / rx2);
        double error = y - floor(y);

        double alpha = fround(error * max_alpha);
        
        pixel4(renderer, cx, cy, x, floor(y),     alpha,             r, g, b);
        pixel4(renderer, cx, cy, x, floor(y) - 1, max_alpha - alpha, r, g, b);
    }

    q = fround(ry2 / sqrt(rx2 + ry2));
    for (double y = 0; y <= q; y++) {
        double x = rx * sqrt(1 - y * y / ry2);
        double error = x - floor(x);

        double alpha = fround(error * max_alpha);

        pixel4(renderer, cx, cy, floor(x),     y, alpha,             r, g, b);
        pixel4(renderer, cx, cy, floor(x) - 1, y, max_alpha - alpha, r, g, b);
    }
}

/**
 * @brief Draw text
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
 * @brief Draw spring
 * 
 * @param renderer SDL Renderer
 * @param cons Constraint
 * @param aa Anit-aliasing
 * @param color Color
 */
void draw_spring(
    SDL_Renderer *renderer,
    nv_Constraint *cons,
    bool aa,
    SDL_Color color
) {
    nv_Spring *spring = (nv_Spring *)cons->head;

    // Transform anchor and body positions
    nv_Vector2 ra = nv_Vector2_rotate(spring->anchor_a, cons->a->angle);
    nv_Vector2 rb = nv_Vector2_rotate(spring->anchor_b, cons->b->angle);
    nv_Vector2 ap = nv_Vector2_add(cons->a->position, ra);
    nv_Vector2 bp = nv_Vector2_add(cons->b->position, rb);
    ap = nv_Vector2_muls(ap, 10.0);
    bp = nv_Vector2_muls(bp, 10.0);

    nv_Vector2 delta = nv_Vector2_sub(bp, ap);
    nv_Vector2 dir = nv_Vector2_normalize(delta);
    double dist = nv_Vector2_len(delta);
    double offset = (dist - spring->length * 10.0) / (spring->length * 10.0);
    double steps = NV_PI / 3.0;
    double stretch = 1.0 + offset;

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

    nv_Vector2 s = nv_Vector2_zero;
    nv_Vector2 e = nv_Vector2_zero;

    for (double step = 0.0; step < dist; step += steps) {
        double next_step = step + steps;
        s = nv_Vector2_muls(dir, step);
        s = nv_Vector2_add(s, nv_Vector2_muls(nv_Vector2_perp(dir), sin(step / stretch) * (10.0 - offset)));
        e = nv_Vector2_muls(dir, next_step);
        e = nv_Vector2_add(e, nv_Vector2_muls(nv_Vector2_perp(dir), sin(next_step / stretch) * (10.0 - offset)));

        if (aa)
            draw_aaline(renderer, ap.x + s.x, ap.y + s.y, ap.x + e.x, ap.y + e.y);
        else
            SDL_RenderDrawLine(renderer, ap.x + s.x, ap.y + s.y, ap.x + e.x, ap.y + e.y);
    }
}


/******************************************

  Example & helper structs
  ------------------------
  - Example:      Stores & handles example
  - ExampleTheme: Color theme of the example
  - Mouse:        Stores mouse information
  - ToggleSwitch: Toggleable UI element
  - Slider:       Slider UI element

******************************************/


/**
 * @brief Mouse struct
 * 
 * @param x X coordinate of mouse
 * @param y Y coordinate of mouse
 * @param px X coordinate of mouse in physics space
 * @param py Y coordinate of mouse in physics space
 * @param left Is left button pressed
 * @param middle Is wheel pressed
 * @param right Is right button pressed
 */
typedef struct {
    int x;
    int y;

    double px;
    double py;

    bool left;
    bool middle;
    bool right;
} Mouse;


/**
 * @brief Toggle switch UI element
 * 
 * @param x X coordinate of toggle switch
 * @param y Y coordinate of toggle switch
 * @param size Size of toggle switch in height
 * @param on Whether the toggle switch is toggled or not
 */
typedef struct {
    int x;
    int y;
    int size;
    bool on;
    bool changed;
} ToggleSwitch;


typedef struct {
    int x;
    int cx;
    int y;
    int width;
    double value;
    double max;
    double min;
    bool pressed;
} Slider;


typedef enum {
    ExampleTheme_LIGHT,
    ExampleTheme_DARK
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
 * @brief Example base
 * 
 * @param window SDL_Window
 * @param renderer SDL_Renderer
 * @param mouse Mouse struct
 * @param keys Array of pressed keys
 * @param max_fps Max FPS
 * @param fps Current FPS
 * @param dt Delta-time
 * @param hertz Simulation hertz
 * @param space Nova physics space instance
 * @param update_callback Function called when example is updating
 * @param event_callback Function called for every new event
 * @param draw_callback Function called when example is rendering
 */
struct _Example {
    int width;
    int height;
    SDL_Window *window;
    SDL_Renderer *renderer;
    
    Mouse mouse;
    const Uint8 *keys;

    double max_fps;
    double fps;
    double dt;

    nv_Space *space;
    int iters;
    int substeps;
    double hertz;

    size_t switch_count;
    ToggleSwitch **switches;

    size_t slider_count;
    Slider **sliders;

    // Calbacks
    Example_callback update_callback;
    Example_callback setup_callback;

    // Theme colors
    SDL_Color bg_color;
    SDL_Color text_color;
    SDL_Color alt_text_color;
    SDL_Color body_color;
    SDL_Color static_color;
    SDL_Color constraint_color;
    SDL_Color aabb_color;
    SDL_Color ui_color;
    SDL_Color velocity_color;

    // Profile
    double step_time;
    double render_time;
};

typedef struct _Example Example;

/**
 * Contact drawer callback
 */
void after_callback(nv_Array *res_arr, void *user_data) {
    Example *example = (Example *)user_data;
    
    if (example->switches[2]->on) {
        for (size_t i = 0; i < res_arr->size; i++) {
            nv_Resolution *res = (nv_Resolution *)res_arr->data[i];
            nv_Vector2 cp;

            SDL_SetRenderDrawColor(example->renderer, 255, 170, 0, 255);

            if (res->contact_count == 1) {
                cp = nv_Vector2_muls(res->contacts[0], 10.0);

                draw_aacircle(example->renderer, cp.x, cp.y, 3.0, 255, 0, 0);
            }

            else if (res->contact_count == 2) {
                cp = nv_Vector2_divs(
                    nv_Vector2_add(res->contacts[0], res->contacts[1]), 2.0 * 0.1);

                nv_Vector2 c1 = nv_Vector2_muls(res->contacts[0], 10.0);
                nv_Vector2 c2 = nv_Vector2_muls(res->contacts[1], 10.0);

                draw_aacircle(example->renderer, c1.x, c1.y, 3.0, 255, 0, 0);
                draw_aacircle(example->renderer, c2.x, c2.y, 3.0, 255, 0, 0);
            }
            SDL_SetRenderDrawColor(example->renderer, 255, 0, 0, 255);
        }
    }
}

/**
 * @brief Create new example instance
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
    double max_fps,
    double hertz,
    ExampleTheme theme
) {
    Example *example = (Example *)malloc(sizeof(Example));

    SDL_Init(SDL_INIT_EVERYTHING);
    TTF_Init();

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

    // For anti-aliased drawing functions
    SDL_SetRenderDrawBlendMode(example->renderer, SDL_BLENDMODE_BLEND);

    example->mouse = (Mouse){0, 0, 0.0, 0.0, false, false, false};
    example->keys = SDL_GetKeyboardState(NULL);

    example->max_fps = max_fps;
    example->fps = max_fps;
    example->dt = 1.0 / max_fps;

    example->space = nv_Space_new();
    example->hertz = hertz;
    example->iters = 10;
    example->substeps = 1;

    example->space->callback_user_data = example;
    example->space->after_collision = after_callback;

    example->update_callback = NULL;
    example->setup_callback = NULL;

    //Light theme
    if (theme == ExampleTheme_LIGHT) {
        example->bg_color = (SDL_Color){255, 255, 255, 255};
        example->text_color = (SDL_Color){0, 0, 0, 255};
        example->alt_text_color = (SDL_Color){90, 90, 96, 255};
        example->body_color = (SDL_Color){40, 40, 44, 255};
        example->static_color = (SDL_Color){123, 124, 138, 255};
        example->constraint_color = (SDL_Color){56, 255, 169, 255};
        example->aabb_color = (SDL_Color){252, 127, 73, 255};
        example->ui_color = (SDL_Color){97, 197, 255, 255};
        example->velocity_color = (SDL_Color){169, 237, 43};
    }
    // Dark theme
    else if (theme == ExampleTheme_DARK) {
        example->bg_color = (SDL_Color){32, 32, 36, 255};
        example->text_color = (SDL_Color){255, 255, 255, 255};
        example->alt_text_color = (SDL_Color){153, 167, 191, 255};
        example->body_color = (SDL_Color){237, 244, 255, 255};
        example->static_color = (SDL_Color){116, 126, 143, 255};
        example->constraint_color = (SDL_Color){56, 255, 169, 255};
        example->aabb_color = (SDL_Color){252, 127, 73, 255};
        example->ui_color = (SDL_Color){66, 164, 245, 255};
        example->velocity_color = (SDL_Color){197, 255, 71};
    }

    // Profile stats
    example->step_time = 0.0;
    example->render_time = 0.0;

    return example;
}

/**
 * @brief Free space allocated by example
 * 
 * @param example Example to free
 */
void Example_free(Example *example) {
    SDL_DestroyWindow(example->window);
    example->window = NULL;

    SDL_DestroyRenderer(example->renderer);
    example->renderer = NULL;

    example->keys = NULL;

    nv_Space_free(example->space);
    example->space = NULL;

    example->update_callback = NULL;

    free(example);
}


/*********************************************************

  Main loop functions
  -------------------
  - draw_ui:          Draw the user interface
  - draw_constraints: Draw constraints
  - draw_bodies:      Draw bodies
  - *_update, *_draw: Update & draw UI elements

*********************************************************/


/**
 * Render UI
 */
void draw_ui(Example *example, TTF_Font *font) {
    // Calculate total kinetic energy
    double le;
    double ae;
    double total_le = 0.0;
    double total_ae = 0.0;
    double total_energy = 0.0;
    for (size_t i = 0; i < example->space->bodies->size; i++) {
        nv_Body *body = (nv_Body *)example->space->bodies->data[i];
        le = nv_Body_get_kinetic_energy(body);
        ae = nv_Body_get_rotational_energy(body);
        total_le += le;
        total_ae += ae;
        total_energy += le + ae;
    }

    struct SDL_version sdl_ver;
    SDL_GetVersion(&sdl_ver);
    char text_sdlver[32];
    sprintf(text_sdlver, "SDL %d.%d.%d", sdl_ver.major, sdl_ver.minor, sdl_ver.patch);

    char text_novaver[32];
    sprintf(text_novaver, "Nova Physics %s", NV_VERSTR);

    char *text_instr0 = "1 meter = 10 pixels";
    char *text_instr = "Click & drag bodies";
    char *text_instr1 = "R to restart";
    char *text_instr2 = "Q to explosion";

    char text_fps[32];
    sprintf(text_fps, "FPS: %.1f", example->fps);

    char text_steptime[32];
    sprintf(text_steptime, "Step: %.2fms", example->step_time);

    char text_rendertime[32];
    sprintf(text_rendertime, "Render: %.2fms", example->render_time);

    char text_bodies[32];
    sprintf(text_bodies, "Bodies: %lu", (unsigned long)example->space->bodies->size);

    char text_consts[32];
    sprintf(text_consts, "Constraints: %lu", (unsigned long)example->space->constraints->size);

    char text_attrs[32];
    sprintf(text_attrs, "Attractors: %lu", (unsigned long)example->space->attractors->size);

    char text_res[32];
    sprintf(text_res, "Resolutions: %lu", (unsigned long)example->space->res->size);

    char text_subs[32];
    sprintf(text_subs, "Substeps: %d", (int)example->sliders[2]->value);

    char text_iters[32];
    sprintf(text_iters, "Collision iters: %d", (int)example->sliders[0]->value);

    char text_citers[32];
    sprintf(text_citers, "Constraint iters: %d", (int)example->sliders[1]->value);

    char text_cbias[32];
    sprintf(text_cbias, "Corr. bias: %.3f", example->sliders[3]->value);

    char text_le[64];
    sprintf(text_le, "Total linear energy: %.2fJ", total_le);

    char text_ae[64];
    sprintf(text_ae, "Total angular energy: %.2fJ", total_ae);

    char text_energy[64];
    sprintf(text_energy, "Total energy: %.2fJ", total_energy);

    char *text_aa = "Anti-aliasing";
    char *text_da = "Draw AABBs";
    char *text_dc = "Draw contacts";
    char *text_dd = "Draw directions";
    char *text_dj = "Draw constraints";
    char *text_dv = "Draw velocities";

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
    
    // font size + 4 px
    int y_gap = 12 + 4;

    draw_text(font, example->renderer, text_sdlver, example->width-80+15, 5 + (y_gap*0), example->text_color);
    draw_text(font, example->renderer, text_novaver, example->width-138+24, 5 + (y_gap*1), example->text_color);
    draw_text(font, example->renderer, text_instr0, example->width-137+28, 56 + (y_gap*0), example->alt_text_color);
    draw_text(font, example->renderer, text_instr, example->width-145+27, 56 + (y_gap*1), example->alt_text_color);
    draw_text(font, example->renderer, text_instr1, example->width-118+46, 56 + (y_gap*2), example->alt_text_color);
    draw_text(font, example->renderer, text_instr2, example->width-118+28, 56 + (y_gap*3), example->alt_text_color);


    draw_text(font, example->renderer, text_fps, 5, 5 + (y_gap*0), example->text_color);
    draw_text(font, example->renderer, text_steptime, 5, 5 + (y_gap*1), example->text_color);
    draw_text(font, example->renderer, text_rendertime, 5, 5 + (y_gap*2), example->text_color);

    draw_text(font, example->renderer, text_bodies, 123, 5 + (y_gap*0), example->text_color);
    draw_text(font, example->renderer, text_consts, 123, 5 + (y_gap*1), example->text_color);
    draw_text(font, example->renderer, text_attrs, 123, 5 + (y_gap*2), example->text_color);
    draw_text(font, example->renderer, text_res, 123, 5 + (y_gap*3), example->text_color);

    draw_text(font, example->renderer, text_le, 233, 5 + (y_gap*0), example->text_color);
    draw_text(font, example->renderer, text_ae, 233, 5 + (y_gap*1), example->text_color);
    draw_text(font, example->renderer, text_energy, 233, 5 + (y_gap*2), example->text_color);

    draw_text(font, example->renderer, text_iters, 145, 10 + (y_gap*5), example->text_color);
    draw_text(font, example->renderer, text_citers, 145, 45 + (y_gap*6), example->text_color);
    draw_text(font, example->renderer, text_subs, 272, 10 + (y_gap*5), example->text_color);
    draw_text(font, example->renderer, text_cbias, 272, 45 + (y_gap*6), example->text_color);

    draw_text(font, example->renderer, text_aa, 5, 10 + (y_gap*5), example->text_color);
    draw_text(font, example->renderer, text_da, 5, 10 + (y_gap*6), example->text_color);
    draw_text(font, example->renderer, text_dc, 5, 10 + (y_gap*7), example->text_color);
    draw_text(font, example->renderer, text_dd, 5, 10 + (y_gap*8), example->text_color);
    draw_text(font, example->renderer, text_dj, 5, 10 + (y_gap*9), example->text_color);
    draw_text(font, example->renderer, text_dv, 5, 10 + (y_gap*10), example->text_color);
}

/**
 * Render constraints
*/
void draw_constraints(Example *example) {
    if (example->switches[4]->on) {
        for (size_t i = 0; i < example->space->constraints->size; i++) {
            nv_Constraint *cons = (nv_Constraint *)example->space->constraints->data[i];

            SDL_SetRenderDrawColor(
                example->renderer,
                example->constraint_color.r,
                example->constraint_color.g,
                example->constraint_color.b,
                example->constraint_color.a
            );

            switch (cons->type) {
                case nv_ConstraintType_SPRING:
                    draw_spring(
                        example->renderer,
                        cons,
                        example->switches[0]->on,
                        example->constraint_color
                    );
                    break;
            }
        }
    }
}

/**
 * Render bodies
*/
void draw_bodies(Example *example) {
for (size_t i = 0; i < example->space->bodies->size; i++) {
        nv_Body *body = (nv_Body *)example->space->bodies->data[i];
        nv_AABB aabb = nv_Body_get_aabb(body);

        // Draw AABB
        if (example->switches[1]->on){
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

        if (body->type == nv_BodyType_STATIC)
            SDL_SetRenderDrawColor(
                example->renderer,
                example->static_color.r,
                example->static_color.g,
                example->static_color.b,
                example->static_color.a
            );
        else
            SDL_SetRenderDrawColor(
                example->renderer,
                example->body_color.r,
                example->body_color.g,
                example->body_color.b,
                example->body_color.a
            );

        if (body->is_sleeping)
            SDL_SetRenderDrawColor(example->renderer, 255, 100, 100, 255);

        // Draw circle bodies
        if (body->shape == nv_BodyShape_CIRCLE) {
            double x = body->position.x * 10.0;
            double y = body->position.y * 10.0;

            if (example->switches[0]->on) {
                draw_aacircle(
                    example->renderer,
                    x, y,
                    body->radius * 10.0,
                    example->body_color.r,
                    example->body_color.g,
                    example->body_color.b
                );

                if (example->switches[3]->on) {
                    nv_Vector2 a = (nv_Vector2){body->radius*10.0, 0.0};
                    a = nv_Vector2_rotate(a, body->angle);

                    draw_aaline(example->renderer, x, y, x+a.x, y+a.y);
                }
            }
            else {
                draw_circle(
                    example->renderer,
                    (int32_t)x,
                    (int32_t)y,
                    (int32_t)(body->radius * 10.0)
                );

                if (example->switches[3]->on) {
                    nv_Vector2 a = (nv_Vector2){body->radius*10.0, 0.0};
                    a = nv_Vector2_rotate(a, body->angle);

                    SDL_RenderDrawLineF(example->renderer, x, y, x+a.x, y+a.y);
                }
            }
        }

        // Draw polygon bodies
        else {
            nv_Polygon_model_to_world(body);

            if (example->switches[0]->on)
                draw_aapolygon(example->renderer, body->trans_vertices);
            else
                draw_polygon(example->renderer, body->trans_vertices);

            if (example->switches[3]->on) {
                nv_Vector2 center = nv_Vector2_muls(nv_polygon_centroid(body->trans_vertices), 10.0);
                nv_Vector2 diredge = nv_Vector2_muls(nv_Vector2_divs(
                    nv_Vector2_add(
                        NV_TO_VEC2(body->trans_vertices->data[0]),
                        NV_TO_VEC2(body->trans_vertices->data[1])),
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

        // Draw velocity vectors
        if (example->switches[5]->on && body->type != nv_BodyType_STATIC) {
            SDL_SetRenderDrawColor(
                example->renderer,
                example->velocity_color.r,
                example->velocity_color.g,
                example->velocity_color.b,
                example->velocity_color.a
            );

            nv_Vector2 v = nv_Vector2_muls(nv_Vector2_add(body->position, body->linear_velocity), 10.0);

            if (nv_Vector2_len2(body->linear_velocity) >= 0.26 / 10.0) {
                nv_Vector2 p = nv_Vector2_muls(body->position, 10.0);
                nv_Vector2 arrow = nv_Vector2_muls(nv_Vector2_normalize(body->linear_velocity), 5.0);
                nv_Vector2 arrow1 = nv_Vector2_rotate(arrow, NV_PI / 6.0);
                nv_Vector2 arrow2 = nv_Vector2_rotate(arrow, NV_PI * 2.0 -  NV_PI / 6.0);

                if (example->switches[0]) {
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
 * Update ToggleSwitch object
 */
void ToggleSwitch_update(struct _Example *example, ToggleSwitch *tg) {
    if (example->mouse.x < tg->x + tg->size && example->mouse.x > tg->x &&
        example->mouse.y < tg->y + tg->size && example->mouse.y > tg->y) {
        
        if (example->mouse.left && !tg->changed) {
            tg->on = !tg->on;
            tg->changed = true;
        }
    }
}

/**
 * Draw ToggleSwitch object
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
 * Update Slider object
 */
void Slider_update(struct _Example *example, Slider *s) {
    if (s->pressed) {
        int cx;
        if (example->mouse.x < s->x) cx = s->x;
        else if (example->mouse.x > s->x + s->width) cx = s->x + s->width;
        else cx = example->mouse.x;
        s->cx = cx;
        s->value = s->min + (((double)cx - (double)s->x) / (double)s->width) * (s->max - s->min);
    }
}

/**
 * Draw Slider object
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


/*****************************

  Main loop
  ---------
  - Initialize
    - Handle events
    - Advance the simulation
    - Render
    - Loop

*****************************/


/**
 * @brief Entry point of example
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
    double step_time_f;
    Uint64 render_time_start;
    Uint64 render_time;
    double render_time_f = 0.0;
    double frequency = (double)SDL_GetPerformanceFrequency();
    int frames = 0;
    int fps_every_f = 10;

    // Set random seed
    srand(time(NULL));

    SDL_Event event;

    nv_Body *selected = NULL;
    nv_Vector2 selected_posf = nv_Vector2_zero;
    nv_Vector2 selected_pos = nv_Vector2_zero;

    TTF_Font *font;

    font = TTF_OpenFont("assets/Montserrat-Regular.ttf", 12);
    if (font == NULL) {
        printf("Couldn't load assets/Montserrat-Regular.ttf");
        return;
    }
    TTF_SetFontStyle(font, TTF_STYLE_NORMAL);
    TTF_SetFontOutline(font, 0);
    TTF_SetFontKerning(font, 1);
    TTF_SetFontHinting(font, TTF_HINTING_NORMAL);

    size_t switches_n = 6;
    ToggleSwitch *switches[switches_n];

    switches[0] = &(ToggleSwitch){
        .x = 118, .y = 63+4+32-5,
        .size = 9, .on = true
    };

    switches[1] = &(ToggleSwitch){
        .x = 118, .y = 79+4+32-5,
        .size = 9, .on = false
    };

    switches[2] = &(ToggleSwitch){
        .x = 118, .y = 95+4+32-5,
        .size = 9, .on = true
    };

    switches[3] = &(ToggleSwitch){
        .x = 118, .y = 111+4+32-5,
        .size = 9, .on = false
    };

    switches[4] = &(ToggleSwitch){
        .x = 118, .y = 127+4+32-5,
        .size = 9, .on = true
    };

    switches[5] = &(ToggleSwitch){
        .x = 118, .y = 143+4+32-5,
        .size = 9, .on = false
    };

    example->switches = switches;
    example->switch_count = switches_n;

    size_t sliders_n = 4;
    Slider *sliders[sliders_n];

    sliders[0] = &(Slider){
        .x = 145, .y = 113,
        .width = 80,
        .min = 1, .max = 50, .value = 10,
    };
    sliders[0]->cx = sliders[0]->x + ((sliders[0]->value-sliders[0]->min) / (sliders[0]->max - sliders[0]->min)) * sliders[0]->width;

    sliders[1] = &(Slider){
        .x = 145, .y = 164,
        .width = 80,
        .min = 1, .max = 50, .value = 5,
    };
    sliders[1]->cx = sliders[1]->x + ((sliders[1]->value-sliders[1]->min) / (sliders[1]->max - sliders[1]->min)) * sliders[1]->width;

    sliders[2] = &(Slider){
        .x = 272, .y = 113,
        .width = 80,
        .min = 1, .max = 10, .value = 1,
    };
    sliders[2]->cx = sliders[2]->x + ((sliders[2]->value-sliders[2]->min) / (sliders[2]->max - sliders[2]->min)) * sliders[2]->width;

    sliders[3] = &(Slider){
        .x = 272, .y = 164,
        .width = 80,
        .min = 0.001, .max = 1.00, .value = 0.20,
    };
    sliders[3]->cx = sliders[3]->x + ((sliders[3]->value-sliders[3]->min) / (sliders[3]->max - sliders[3]->min)) * sliders[3]->width;

    example->sliders = sliders;
    example->slider_count = sliders_n;

    if (example->setup_callback != NULL)
        example->setup_callback(example);

    while (is_running) {
        start_perf = SDL_GetTicks64();

        // Handle events
        while(SDL_PollEvent(&event) != 0) {
            if (event.type == SDL_QUIT)
                is_running = false;

            else if (event.type == SDL_MOUSEMOTION) {
                SDL_GetMouseState(&example->mouse.x, &example->mouse.y);
                example->mouse.px = example->mouse.x / 10.0;
                example->mouse.py = example->mouse.y / 10.0;
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
                        nv_Body *body = (nv_Body *)example->space->bodies->data[i];
                        if (body->type == nv_BodyType_STATIC) continue;
                        nv_AABB aabb = nv_Body_get_aabb(body);

                        if (nv_collide_aabb_x_point(aabb, (nv_Vector2){example->mouse.px, example->mouse.py})) {
                            selected = body;

                            // transform mouse coordinatets to body local coordinates
                            selected_posf = (nv_Vector2){example->mouse.px, example->mouse.py};
                            selected_posf = nv_Vector2_sub(selected_posf, selected->position);
                            selected_posf = nv_Vector2_rotate(selected_posf, -selected->angle);

                            selected_pos = (nv_Vector2){selected_posf.x, selected_posf.y};

                            if (selected->is_sleeping) nv_Body_awake(selected);

                            break;
                        }
                    }

                    for (size_t i = 0; i < example->slider_count; i++) {
                        Slider *s = example->sliders[i];

                        if (example->mouse.x < s->x + s->width && example->mouse.x > s->x &&
                            example->mouse.y < s->y + 6.0 && example->mouse.y > s->y) {
                            
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
                    for (size_t i = 0; i < switches_n; i++) {
                       switches[i]->changed = false;
                    }

                    for (size_t i = 0; i < example->slider_count; i++) {
                        example->sliders[i]->pressed = false;
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
                        nv_Body *body = (nv_Body *)example->space->bodies->data[i];
                        if (body->type == nv_BodyType_STATIC) continue;

                        nv_Vector2 delta = nv_Vector2_sub(
                            body->position,
                            (nv_Vector2){example->mouse.px, example->mouse.py}
                        );

                        double strength = 10.0 * pow(10.0, 4.0);

                        nv_Vector2 force = nv_Vector2_muls(delta, strength);
                        force = nv_Vector2_divs(force, nv_Vector2_len(delta));

                        nv_Body_apply_force(body, force);
                    }
                }
                else if (event.key.keysym.scancode == SDL_SCANCODE_R) {
                    nv_Space_clear(example->space);

                    if (example->setup_callback != NULL)
                        example->setup_callback(example);
                }
            }
        }

        // Drag selected object towards mouse
        if (selected) {
            selected_pos = nv_Vector2_rotate(selected_posf, selected->angle);

            nv_Vector2 force = nv_Vector2_sub(
                NV_VEC2(example->mouse.px, example->mouse.py),
                nv_Vector2_add(selected->position, selected_pos)
            );

            force = nv_Vector2_muls(force, 1.0 * pow(10.0, 3.0));

            nv_Body_apply_force_at(selected, force, selected_pos);
        }

        // Call example callback if there is one
        if (example->update_callback != NULL)
            example->update_callback(example);


        // We clear display before stepping because collision callback
        // functions draws things
        SDL_SetRenderDrawColor(
            example->renderer,
            example->bg_color.r,
            example->bg_color.g,
            example->bg_color.b,
            255
        );
        SDL_RenderClear(example->renderer);


        // Advance the simulation

        step_time_start = SDL_GetPerformanceCounter();

         nv_Space_step(
            example->space,
            example->hertz,
            (int)example->sliders[0]->value,
            (int)example->sliders[2]->value
        );

        step_time_end = SDL_GetPerformanceCounter() - step_time_start;
        step_time_f = (double)step_time_end / frequency * 1000.0;
        example->step_time = step_time_f;


        render_time_start = SDL_GetPerformanceCounter();

        draw_bodies(example);

        draw_constraints(example);

        // Draw line between mouse and selected object
        if (selected) {
            SDL_SetRenderDrawColor(example->renderer, 0, 255, 50, 255);
            
            if (switches[0]->on)
                draw_aaline(
                    example->renderer,
                    (selected->position.x + selected_pos.x) * 10.0,
                    (selected->position.y + selected_pos.y) * 10.0,
                    example->mouse.x,
                    example->mouse.y
                    );
            else {
                SDL_RenderDrawLineF(
                    example->renderer,
                    (selected->position.x + selected_pos.x) * 10.0,
                    (selected->position.y + selected_pos.y) * 10.0,
                    example->mouse.x,
                    example->mouse.y
                    );
            }
        };

        draw_ui(example, font);

        // Update the display
        SDL_RenderPresent(example->renderer);

        // Calculate elapsed time during rendering
        render_time = SDL_GetPerformanceCounter() - render_time_start;
        render_time_f = (double)render_time / frequency * 1000.0;
        example->render_time = render_time_f;

        // Sync current fps with max_fps
        end_perf = SDL_GetTicks64();

        Uint64 frame = end_perf - start_perf;

        frames++;
        if (frames == fps_every_f) {
            end_perf_hi = SDL_GetPerformanceCounter();
            double start = (double)start_perf_hi / frequency;
            double end = (double)end_perf_hi / frequency;

            example->fps = (double)fps_every_f / (end - start);

            frames = 0;
            start_perf_hi = SDL_GetPerformanceCounter();
        }

        if (frame < (1000 / example->max_fps)) {
            SDL_Delay((1000 / example->max_fps) - frame);
        }
    }
}