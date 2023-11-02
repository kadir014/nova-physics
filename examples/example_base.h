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
#include <SDL2/SDL_image.h>
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
 * @brief Return random nv_float in given range
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
 * @brief Return random boolean
 * 
 * @return bool
 */
bool brand() {
    return irand(0, 1);
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


SDL_Color hsv_to_rgb(SDL_Color hsv) {
    // hsv.rgb = hsv.hsv

    SDL_Color rgb;
    Uint8 region, remainder, p, q, t;
    
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
 * @brief Fill circle
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

void swap(nv_float *a, nv_float *b) {
    nv_float temp = *a;
    *a = *b;
    *b = temp;
}

int ipart(nv_float x) {
    return (int)x;
}

int fround(nv_float x) {
    return ipart(x + 0.5);
}

nv_float fpart(nv_float x) {
    return x - ipart(x);
}

nv_float rfpart(nv_float x) {
    return 1.0 - fpart(x);
}

void pixel(
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
    nv_float x0,
    nv_float y0,
    nv_float x1,
    nv_float y1
) {
    bool steep = nv_fabs(y1 - y0) > nv_fabs(x1 - x0);
    
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

    nv_float dx = x1 - x0;
    nv_float dy = y1 - y0;

    nv_float gradient;
    if (dx == 0.0) gradient = 1.0;
    else gradient = dy / dx;

    // Handle first endpoint
    int xend = fround(x0);
    nv_float yend = y0 + gradient * (xend - x0);
    nv_float xgap = rfpart(x0 + 0.5);
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

    nv_float intery = yend + gradient; // First Y intersection

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
    nv_float x,
    nv_float y,
    nv_float dx,
    nv_float dy,
    nv_float alpha,
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
    nv_float cx,
    nv_float cy,
    nv_float radius,
    Uint8 r,
    Uint8 g,
    Uint8 b
) {
    // + 0.3 is for correction
    nv_float rx = radius + 0.3;
    nv_float ry = radius + 0.3;
    nv_float rx2 = rx * rx;
    nv_float ry2 = ry * ry;

    nv_float max_alpha = 255.0;

    nv_float q = fround(rx2 / nv_sqrt(rx2 + ry2));
    for (nv_float x = 0; x <= q; x++) {
        nv_float y = ry * nv_sqrt(1 - x * x / rx2);
        nv_float error = y - floor(y);

        nv_float alpha = fround(error * max_alpha);
        
        pixel4(renderer, cx, cy, x, floor(y),     alpha,             r, g, b);
        pixel4(renderer, cx, cy, x, floor(y) - 1, max_alpha - alpha, r, g, b);
    }

    q = fround(ry2 / nv_sqrt(rx2 + ry2));
    for (nv_float y = 0; y <= q; y++) {
        nv_float x = rx * nv_sqrt(1 - y * y / ry2);
        nv_float error = x - floor(x);

        nv_float alpha = fround(error * max_alpha);

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
 * @brief Draw text
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
    nv_Spring *spring = (nv_Spring *)cons->def;

    nv_Vector2 ap;
    nv_Vector2 bp;

    // Transform anchor and body positions
    if (cons->a == NULL) {
        ap = nv_Vector2_mul(spring->anchor_a, 10.0);
    } else {
        nv_Vector2 ra = nv_Vector2_rotate(spring->anchor_a, cons->a->angle);
        ap = nv_Vector2_add(cons->a->position, ra);
        ap = nv_Vector2_mul(ap, 10.0);
    }
    if (cons->b == NULL) {
        bp = nv_Vector2_mul(spring->anchor_b, 10.0);
    } else {
        nv_Vector2 rb = nv_Vector2_rotate(spring->anchor_b, cons->b->angle);
        bp = nv_Vector2_add(cons->b->position, rb);
        bp = nv_Vector2_mul(bp, 10.0);
    }

    nv_Vector2 delta = nv_Vector2_sub(bp, ap);
    nv_Vector2 dir = nv_Vector2_normalize(delta);
    nv_float dist = nv_Vector2_len(delta);
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

    nv_Vector2 s = nv_Vector2_zero;
    nv_Vector2 e = nv_Vector2_zero;

    for (nv_float step = 0.0; step < dist; step += steps) {
        nv_float next_step = step + steps;
        s = nv_Vector2_mul(dir, step);
        s = nv_Vector2_add(s, nv_Vector2_mul(nv_Vector2_perp(dir), sin(step / stretch) * (10.0 - offset)));
        e = nv_Vector2_mul(dir, next_step);
        e = nv_Vector2_add(e, nv_Vector2_mul(nv_Vector2_perp(dir), sin(next_step / stretch) * (10.0 - offset)));

        if (aa)
            draw_aaline(renderer, ap.x + s.x, ap.y + s.y, ap.x + e.x, ap.y + e.y);
        else
            SDL_RenderDrawLine(renderer, ap.x + s.x, ap.y + s.y, ap.x + e.x, ap.y + e.y);
    }
}


SDL_Texture *load_image(SDL_Renderer *renderer, char *path) {
    // Load image at specified path
    SDL_Texture *texture = IMG_LoadTexture(renderer, path);
    if (texture == NULL) {
        printf("Unable to load image at %s. Error: %s\n", path, IMG_GetError());
        exit(1);
    }

    return texture;
}

void draw_image(SDL_Renderer *renderer, SDL_Texture *texture, int x, int y, nv_float angle) {
    //int width, height;
    //SDL_QueryTexture(texture, NULL, NULL, &width, &height);

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

    nv_float px;
    nv_float py;

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
    nv_float value;
    nv_float max;
    nv_float min;
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
    SDL_Texture *texture;
    
    Mouse mouse;
    const Uint8 *keys;

    nv_float max_fps;
    nv_float fps;
    nv_float dt;

    nv_Space *space;
    nv_float hertz;

    bool step;

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
    SDL_Color sleep_color;
    SDL_Color spring_color;
    SDL_Color distancejoint_color;
    SDL_Color aabb_color;
    SDL_Color ui_color;
    SDL_Color velocity_color;

    bool draw_ui;

    bool record;

    nv_Array *sprites;

    // Profile
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
void after_callback(nv_HashMap *res_arr, void *user_data) {
    Example *example = (Example *)user_data;

    if (!example->switches[2]->on) return;

    size_t iter = 0;
    void *item;
    while (nv_HashMap_iter(res_arr, &iter, &item)) {

        nv_Resolution *res = item;

        nv_float radius = 2.5;

        nv_Vector2 cp;
        SDL_Color color;

        if (res->contact_count == 1) {
            nv_Contact contact = res->contacts[0];
            cp = nv_Vector2_mul(contact.position, 10.0);

            if (
                nv_Vector2_dist2(
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
            nv_Contact contact1 = res->contacts[0];
            nv_Contact contact2 = res->contacts[1];

            nv_Vector2 c1 = nv_Vector2_mul(contact1.position, 10.0);
            nv_Vector2 c2 = nv_Vector2_mul(contact2.position, 10.0);

            if (
                nv_Vector2_dist2(
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
                nv_Vector2_dist2(
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
    nv_float max_fps,
    nv_float hertz,
    ExampleTheme theme
) {
    Example *example = (Example *)malloc(sizeof(Example));

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

    // Linear filtering for textures
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

    example->space = nv_Space_new();
    example->hertz = hertz;

    example->step = true;

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
        example->sleep_color = (SDL_Color){176, 132, 77, 255};
        example->spring_color = (SDL_Color){56, 255, 169, 255};
        example->distancejoint_color = (SDL_Color){74, 201, 255, 255};
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
        example->aabb_color = (SDL_Color){252, 127, 73, 255};
        example->ui_color = (SDL_Color){66, 164, 245, 255};
        example->velocity_color = (SDL_Color){197, 255, 71, 255};
    }

    example->sprites = nv_Array_new();

    // Profile stats
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

    nv_Array_free_each(example->sprites, (void (*)(void *))SDL_DestroyTexture);
    nv_Array_free(example->sprites);

    example->update_callback = NULL;

    free(example->sliders);
    free(example->switches);

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
    sprintf(text_profiler3, "Presolve colls.:  %.2f%cs", example->space->profiler.presolve_collisions * unit_multipler, unit_char);

    char text_profiler4[32];
    sprintf(text_profiler4, "Solve positions:  %.2f%cs", example->space->profiler.solve_positions * unit_multipler, unit_char);

    char text_profiler5[32];
    sprintf(text_profiler5, "Solve velocities: %.2f%cs", example->space->profiler.solve_velocities * unit_multipler, unit_char);

    char text_profiler6[32];
    sprintf(text_profiler6, "Presolve consts.: %.2f%cs", example->space->profiler.presolve_constraints * unit_multipler, unit_char);

    char text_profiler7[32];
    sprintf(text_profiler7, "Solve consts.:    %.2f%cs", example->space->profiler.solve_constraints * unit_multipler, unit_char);

    char text_profiler8[32];
    sprintf(text_profiler8, "Integrate vels.:  %.2f%cs", example->space->profiler.integrate_velocities * unit_multipler, unit_char);

    char text_profiler9[32];
    sprintf(text_profiler9, "Remove bodies:    %.2f%cs", example->space->profiler.remove_bodies * unit_multipler, unit_char);

    char *text_aa = "Anti-aliasing";
    char *text_fs = "Fill shapes";
    char *text_da = "Draw AABBs";
    char *text_dc = "Draw contacts";
    char *text_dd = "Draw directions";
    char *text_dj = "Draw constraints";
    char *text_dv = "Draw velocities";
    char *text_dg = "Draw SHG";
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
}

/**
 * Render constraints
*/
void draw_constraints(Example *example) {
    if (example->switches[4]->on) {
        for (size_t i = 0; i < example->space->constraints->size; i++) {
            nv_Constraint *cons = (nv_Constraint *)example->space->constraints->data[i];

            // Skip cursor body
            if (cons->a == (nv_Body *)example->space->bodies->data[0] ||
                cons->b == (nv_Body *)example->space->bodies->data[0])
                continue;

            // ? Forward declare to avoid errors on GCC < 10
            nv_DistanceJoint *dist_joint;
            nv_Vector2 a, b, ra, rb;

            switch (cons->type) {
                // Spring constraint
                case nv_ConstraintType_SPRING:
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

                // Distance joint constraint
                case nv_ConstraintType_DISTANCEJOINT:
                    dist_joint = (nv_DistanceJoint *)cons->def;

                    SDL_SetRenderDrawColor(
                        example->renderer,
                        example->distancejoint_color.r,
                        example->distancejoint_color.g,
                        example->distancejoint_color.b,
                        example->distancejoint_color.a
                    );

                    // Transform anchor points
                    if (cons->a == NULL) {
                        a = nv_Vector2_mul(dist_joint->anchor_a, 10.0);
                    } else {
                        ra = nv_Vector2_rotate(dist_joint->anchor_a, cons->a->angle);
                        a = nv_Vector2_add(cons->a->position, ra);
                        a = nv_Vector2_mul(a, 10.0);
                    }
                    if (cons->b == NULL) {
                        b = nv_Vector2_mul(dist_joint->anchor_b, 10.0);
                    } else {
                        rb = nv_Vector2_rotate(dist_joint->anchor_b, cons->b->angle);
                        b = nv_Vector2_add(cons->b->position, rb);
                        b = nv_Vector2_mul(b, 10.0);
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
            }
        }
    }
}

/**
 * Render bodies
*/
void draw_bodies(Example *example, TTF_Font *font) {
    // Start from 1 because 0 is cursor body
    for (size_t i = 1; i < example->space->bodies->size; i++) {
        nv_Body *body = (nv_Body *)example->space->bodies->data[i];

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
            nv_AABB aabb = nv_Body_get_aabb(body);

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

        SDL_Color aacolor;

        if (body->type == nv_BodyType_STATIC) {
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
            if (body->shape->type == nv_ShapeType_CIRCLE) {
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
                        nv_Vector2 a = (nv_Vector2){body->shape->radius*10.0, 0.0};
                        a = nv_Vector2_rotate(a, body->angle);

                        draw_aaline(example->renderer, x, y, x+a.x, y+a.y);
                    }
                }
                else if (example->switches[9]->on) {
                    SDL_SetRenderDrawColor(example->renderer, color.r, color.g, color.b, 255);

                    fill_circle(example->renderer, x, y, body->shape->radius * 10.0);

                    if (example->switches[3]->on) {
                        nv_Vector2 a = (nv_Vector2){body->shape->radius*10.0, 0.0};
                        a = nv_Vector2_rotate(a, body->angle);

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
                        nv_Vector2 a = (nv_Vector2){body->shape->radius*10.0, 0.0};
                        a = nv_Vector2_rotate(a, body->angle);

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
                            nv_Vector2 v = NV_TO_VEC2(body->shape->trans_vertices->data[j]);

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
                            nv_Vector2 v = NV_TO_VEC2(body->shape->trans_vertices->data[j]);

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
                            nv_Vector2 v = NV_TO_VEC2(body->shape->trans_vertices->data[j]);

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
                            nv_Vector2 v = NV_TO_VEC2(body->shape->trans_vertices->data[j]);

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
                    nv_Vector2 center = nv_Vector2_mul(nv_polygon_centroid(body->shape->trans_vertices), 10.0);
                    nv_Vector2 diredge = nv_Vector2_mul(nv_Vector2_div(
                        nv_Vector2_add(
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

        // Draw body IDs
        // char text_id[8];
        // sprintf(text_id, "%u", body->id);
        // draw_text(
        //     font,
        //     example->renderer,
        //     text_id,
        //     body->position.x * 10.0,
        //     body->position.y * 10.0,
        //     aacolor
        // );

        // char text_vel[16];
        // sprintf(text_vel, "%f", body->linear_velocity.y);
        // draw_text(
        //     font,
        //     example->renderer,
        //     text_vel,
        //     body->position.x * 10.0 + 30,
        //     body->position.y * 10.0,
        //     aacolor
        // );

        // nv_Resolution *resv = hashmap_get(example->space->res, &(nv_Resolution){.a=body->id-1,.b=body});
        // if (resv) {
        //     char text_j[16];
        //     sprintf(text_id, "%f", resv->jn[0]);
        //     draw_text(
        //         font,
        //         example->renderer,
        //         text_j,
        //         body->position.x * 10.0 + 30,
        //         body->position.y * 10.0 + 30,
        //         aacolor
        //     );
        // }

        // Draw velocity vectors
        if (example->switches[5]->on && body->type != nv_BodyType_STATIC) {
            SDL_SetRenderDrawColor(
                example->renderer,
                example->velocity_color.r,
                example->velocity_color.g,
                example->velocity_color.b,
                example->velocity_color.a
            );

            nv_Vector2 vel = nv_Vector2_mul(body->linear_velocity, 1.0 / 60.0);

            nv_Vector2 v = nv_Vector2_mul(nv_Vector2_add(body->position, vel), 10.0);

            nv_float threshold = 0.25 / 10.0;

            if (nv_Vector2_len2(vel) >= threshold) {
                nv_Vector2 p = nv_Vector2_mul(body->position, 10.0);
                nv_Vector2 arrow = nv_Vector2_mul(nv_Vector2_normalize(vel), 5.0);
                nv_Vector2 arrow1 = nv_Vector2_rotate(arrow, NV_PI / 6.0);
                nv_Vector2 arrow2 = nv_Vector2_rotate(arrow, NV_PI * 2.0 -  NV_PI / 6.0);

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
 * Update ToggleSwitch object
 */
void ToggleSwitch_update(struct _Example *example, ToggleSwitch *tg) {
    if (example->mouse.x < tg->x + tg->size && example->mouse.x > tg->x &&
        example->mouse.y < tg->y + tg->size && example->mouse.y > tg->y) {
        
        if (example->mouse.left && !tg->changed) {
            tg->on = !tg->on;
            tg->changed = true;

            if (tg == example->switches[7]) {
                if (tg->on)
                    nv_Space_enable_sleeping(example->space);
                else
                    nv_Space_disable_sleeping(example->space);
            }

            if (tg == example->switches[8])
                example->space->warmstarting = tg->on;
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
        s->value = s->min + (((nv_float)cx - (nv_float)s->x) / (nv_float)s->width) * (s->max - s->min);
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

    nv_Body *mouse_body = nv_Circle_new(
        nv_BodyType_STATIC,
        nv_Vector2_zero,
        0.0,
        nv_Material_WOOD,
        0.3
    );
    mouse_body->collision = false;
    nv_Space_add(example->space, mouse_body);
    nv_Array_add(example->sprites, NULL);

    nv_Body *selected = NULL;
    nv_Constraint *selected_const = NULL;
    nv_Vector2 selected_posf = nv_Vector2_zero;
    nv_Vector2 selected_pos = nv_Vector2_zero;

    int energy_tick = 0;

    TTF_Font *font;

    font = TTF_OpenFont("assets/FiraCode-Regular.ttf", 11);
    if (font == NULL) {
        printf("Couldn't load assets/FiraCode-Regular.ttf");
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
        .size = 9, .on = false
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
                        nv_Body *body = (nv_Body *)example->space->bodies->data[i];
                        if (body->type == nv_BodyType_STATIC) continue;
                        nv_AABB aabb = nv_Body_get_aabb(body);

                        if (nv_collide_aabb_x_point(aabb, (nv_Vector2){example->mouse.px, example->mouse.py})) {
                            selected = body;

                            // Transform mouse coordinatets to body local coordinates
                            selected_posf = (nv_Vector2){example->mouse.px, example->mouse.py};
                            selected_posf = nv_Vector2_sub(selected_posf, selected->position);
                            selected_posf = nv_Vector2_rotate(selected_posf, -selected->angle);

                            selected_pos = (nv_Vector2){selected_posf.x+0.00001, selected_posf.y+0.00001};

                            selected_const = nv_DistanceJoint_new(
                                mouse_body, selected,
                                nv_Vector2_zero, selected_pos,
                                0.1
                            );

                            nv_Space_add_constraint(example->space, selected_const);

                            if (selected->is_sleeping) nv_Body_awake(selected);

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
                        nv_Array_remove(example->space->constraints, selected_const);
                        nv_Constraint_free(selected_const);
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
                        nv_Body *body = (nv_Body *)example->space->bodies->data[i];
                        if (body->type == nv_BodyType_STATIC) continue;

                        nv_Vector2 delta = nv_Vector2_sub(
                            body->position,
                            (nv_Vector2){example->mouse.px, example->mouse.py}
                        );

                        nv_float strength = 10.0 * pow(10.0, 3.0);

                        nv_Vector2 force = nv_Vector2_mul(delta, strength);
                        force = nv_Vector2_div(force, nv_Vector2_len(delta));

                        nv_Body_apply_force(body, force);
                    }
                }

                else if (event.key.keysym.scancode == SDL_SCANCODE_R) {
                    //printf("before: size %d capacity %d\n", example->space->res->size, example->space->res->capacity);
                    selected = NULL;
                    nv_Array_remove(example->space->constraints, selected_const);
                    nv_Constraint_free(selected_const);
                    selected_const = NULL;

                    nv_Space_clear(example->space);

                    mouse_body = nv_Circle_new(
                        nv_BodyType_STATIC,
                        nv_Vector2_zero,
                        0.0,
                        nv_Material_WOOD,
                        0.3
                    );
                    mouse_body->collision = false;
                    nv_Space_add(example->space, mouse_body);

                    example->counter = 0;

                    if (example->setup_callback != NULL)
                        example->setup_callback(example);

                    step_count = 0;
                    step_final = 0.0;
                    //printf("after: size %d capacity %d\n", example->space->res->size, example->space->res->capacity);
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


        // Draw Spatial Hash Grid
        if (example->switches[6]->on && example->space->broadphase_algorithm == nv_BroadPhase_SPATIAL_HASH_GRID) {
            SDL_SetRenderDrawColor(
                example->renderer,
                70,
                70,
                70,
                255
            );

            nv_SHG *shg = example->space->shg;

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
                    nv_Array *cell = nv_SHG_get(shg, nv_pair(x, y));
                    if (cell == NULL) continue;

                    char text_cell[8];
                    sprintf(text_cell, "%llu", cell->size);

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

            int32_t cell_x = (int32_t)(mouse_body->position.x / shg->cell_width);
            int32_t cell_y = (int32_t)(mouse_body->position.y / shg->cell_height);

            uint32_t neighbors[8];
            bool neighbor_flags[8];
            nv_SHG_get_neighbors(shg, cell_x, cell_y, neighbors, neighbor_flags);

            // Draw neighbor cell texts
            for (size_t j = 0; j < 9; j++) {

                nv_Array *cell;

                // Own cell
                if (j == 8) {
                    cell = nv_SHG_get(shg, nv_pair(cell_x, cell_y));
                    if (cell == NULL) continue;
                }
                // Neighbor cells
                else {
                    if (!neighbor_flags[j]) continue;

                    cell = nv_SHG_get(shg, neighbors[j]);
                    if (cell == NULL) continue;
                }

                for (size_t k = 0; k < cell->size; k++) {
                    nv_Body *b = (nv_Body *)cell->data[k];

                    if (b == mouse_body) continue;

                    draw_aacircle(
                        example->renderer,
                        example->mouse.x,
                        example->mouse.y,
                        3.5 * 10.0,
                        0, 255, 0
                    );

                    if (nv_Vector2_dist2(b->position, NV_VEC2(example->mouse.px, example->mouse.py)) <= 3.5 * 3.5) {
                        draw_aacircle(
                            example->renderer,
                            b->position.x * 10.0,
                            b->position.y * 10.0,
                            3.0,
                            0, 255, 0
                        );
                    }
                }
            }
        }

        // if (example->switches[6]->on && example->space->broadphase_algorithm == nv_BroadPhase_QUAD_TREE) {
        //     SDL_SetRenderDrawColor(
        //         example->renderer,
        //         70,
        //         70,
        //         70,
        //         255
        //     );

        //     draw_quadtree
        // }


        draw_bodies(example, font);

        draw_constraints(example);

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

            nv_Space_step(
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
            sprintf(filename, "../examples/recording/frame%llu.png", frame_counter);

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
                nv_Body *body = (nv_Body *)example->space->bodies->data[i];
                le = nv_Body_get_kinetic_energy(body);
                ae = nv_Body_get_rotational_energy(body);
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

    //printf("final: %f\n", step_final / (nv_float)step_count);
}