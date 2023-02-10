/*

  This file is a part of the Nova Physics Engine
  project and distributed under the MIT license.

  Copyright © Kadir Aksoy
  https://github.com/kadir014/Nova-Physics

*/

#include <stdbool.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include "novaphysics/novaphysics.h"


/**
 * @brief Draw circle
 * 
 * @param renderer 
 * @param cx 
 * @param cy 
 * @param radius 
 */
void draw_circle(SDL_Renderer *renderer, int32_t cx, int32_t cy, int32_t radius) {
   // https://discourse.libsdl.org/t/query-how-do-you-draw-a-circle-in-sdl2-sdl2/33379
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
 * @param renderer 
 * @param vertices 
 */
void draw_polygon(SDL_Renderer *renderer, nv_Vector2Array *vertices) {
    size_t n = vertices->size;

    for (size_t i = 0; i < n; i++) {
        nv_Vector2 va = vertices->data[i];
        nv_Vector2 vb = vertices->data[(i + 1) % n];

        SDL_RenderDrawLineF(
            renderer,
            va.x * 10.0, va.y * 10.0,
            vb.x * 10.0, vb.y * 10.0
            );
    }
}

/**
 * @brief Draw text
 * 
 * @param font 
 * @param renderer 
 * @param text 
 * @param x 
 * @param y 
 * @param color 
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


struct _Example;

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
 * @param draw_aabb Draw AABBs?
 * @param update_callback Function called when example is updating
 * @param event_callback Function called for every new event
 * @param draw_callback Function called when example is rendering
 */
struct _Example {
    SDL_Window* window;
    SDL_Renderer* renderer;
    
    Mouse mouse;
    const Uint8 *keys;

    double max_fps;
    double fps;
    double dt;

    nv_Space *space;
    int iters;
    int substeps;
    double hertz;

    bool draw_aabb;
    bool draw_contacts;
};

typedef struct _Example Example;

/**
 * Contact drawer callback
 */
void after_callback(nv_ResolutionArray *res_arr, void *user_data) {
    Example *example = (Example *)user_data;
    
    if (example->draw_contacts) {
        for (size_t i = 0; i < res_arr->size; i++) {
            nv_Resolution res = res_arr->data[i];
            nv_Vector2 cp;

            SDL_SetRenderDrawColor(example->renderer, 255, 170, 0, 255);

            if (res.contact_count == 1) {
                cp = nv_Vector2_muls(res.contacts[0], 10.0);
                draw_circle(example->renderer, cp.x, cp.y, 1);
                draw_circle(example->renderer, cp.x, cp.y, 2);
            }

            else if (res.contact_count == 2) {
                cp = nv_Vector2_divs(
                    nv_Vector2_add(res.contacts[0], res.contacts[1]), 2.0 * 0.1);

                nv_Vector2 c1 = nv_Vector2_muls(res.contacts[0], 10.0);
                nv_Vector2 c2 = nv_Vector2_muls(res.contacts[1], 10.0);

                draw_circle(example->renderer, c1.x, c1.y, 1);
                draw_circle(example->renderer, c2.x, c2.y, 1);
                draw_circle(example->renderer, c1.x, c1.y, 2);
                draw_circle(example->renderer, c2.x, c2.y, 2);
            }
            SDL_SetRenderDrawColor(example->renderer, 255, 0, 0, 255);

            nv_Vector2 contact_line = nv_Vector2_add(cp, nv_Vector2_muls(res.normal, 7.0));

            SDL_RenderDrawLineF(example->renderer, cp.x, cp.y, contact_line.x, contact_line.y);
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
    double hertz
) {
    Example *example = (Example *)malloc(sizeof(Example));

    SDL_Init(SDL_INIT_EVERYTHING);
    TTF_Init();

    example->window = SDL_CreateWindow(
        title,
        SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
        width, height,
        SDL_WINDOW_SHOWN
    );

    example->renderer = SDL_CreateRenderer(
        example->window, -1, SDL_RENDERER_ACCELERATED);;

    example->keys = SDL_GetKeyboardState(NULL);

    example->max_fps = max_fps;
    example->fps = max_fps;
    example->dt = 1.0 / max_fps;

    example->space = nv_Space_new();
    example->hertz = hertz;
    example->iters = 7;
    example->substeps = 3;

    example->draw_aabb = false;
    example->draw_contacts = true;

    example->space->callback_user_data = example;
    example->space->after_collision = after_callback;

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
    example->max_fps = 0.0;
    example->fps = 0.0;
    example->dt = 0.0;

    nv_Space_free(example->space);
    example->space = NULL;
    example->iters = 0;
    example->substeps = 0;

    free(example);
}

/**
 * @brief Entry point of example
 * 
 * @param example Example to run
 */
void Example_run(Example *example) {
    bool is_running = true;
    Uint32 start_time;
    Uint32 end_time;
    Uint32 step_time;
    Uint32 render_time_start;
    Uint32 render_time;
    SDL_Event event;

    nv_Body *selected = NULL;

    TTF_Font *font;

    font = TTF_OpenFont("assets/Montserrat-Regular.ttf", 14);
    if (font == NULL) {
        printf("Couldn't load font.");
        return;
    }
    TTF_SetFontStyle(font, TTF_STYLE_NORMAL);
    TTF_SetFontOutline(font, 0);
    TTF_SetFontKerning(font, 1);
    TTF_SetFontHinting(font, TTF_HINTING_NORMAL);

    int tick = 0;
    while (is_running) {
        if (tick == (60-1)*3) is_running = false;
        tick++;
        start_time = SDL_GetTicks();

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
                        nv_Body *body = example->space->bodies->data[i];
                        if (body->type == nv_BodyType_STATIC) continue;
                        nv_AABB aabb = nv_Body_get_aabb(body);

                        if (nv_collide_aabb_x_point(aabb, (nv_Vector2){example->mouse.px, example->mouse.py})) {
                            selected = body;
                            if (selected->is_sleeping) nv_Body_awake(selected);
                            break;
                        }
                    }
                }
            }

            else if (event.type == SDL_MOUSEBUTTONUP) {
                if (event.button.button == SDL_BUTTON_LEFT) {
                    example->mouse.left = false;
                    selected = NULL;
                }
                else if (event.button.button == SDL_BUTTON_MIDDLE)
                    example->mouse.middle = false;
                else if (event.button.button == SDL_BUTTON_RIGHT)
                    example->mouse.right = false;
            }
        }

        if (selected) {
            nv_Vector2 strength = nv_Vector2_muls(
                nv_Vector2_sub(
                    (nv_Vector2){example->mouse.px, example->mouse.py},
                    selected->position),
                15.0
            );
            selected->linear_velocity = strength;
        }

        // We clear display before stepping because collision callback
        // functions draws things
        SDL_SetRenderDrawColor(example->renderer, 255, 255, 255, 255);
        SDL_RenderClear(example->renderer);

        step_time = SDL_GetTicks();
         nv_Space_step(
            example->space,
            example->hertz,
            example->iters,
            example->substeps
        );
        step_time = SDL_GetTicks() - step_time;



        render_time_start = SDL_GetTicks();

        SDL_SetRenderDrawColor(example->renderer, 255, 0, 0, 255);

        for (size_t i = 0; i < example->space->bodies->size; i++) {
            nv_Body *body = example->space->bodies->data[i];
            nv_AABB aabb = nv_Body_get_aabb(body);

            if (example->draw_aabb){
                SDL_FRect aabb_rect = (SDL_FRect){
                    aabb.min_x*10.0,
                    aabb.min_y*10.0,
                    (aabb.max_x-aabb.min_x)*10.0,
                    (aabb.max_y - aabb.min_y)*10.0
                };

                SDL_SetRenderDrawColor(example->renderer, 0, 255, 190, 255);
                SDL_RenderDrawRectF(example->renderer, &aabb_rect);
            }

            if (body->type == nv_BodyType_STATIC)
                SDL_SetRenderDrawColor(example->renderer, 90, 91, 99, 255);
            else
                SDL_SetRenderDrawColor(example->renderer, 40, 40, 44, 255);

            if (body->is_sleeping)
                SDL_SetRenderDrawColor(example->renderer, 255, 100, 100, 255);

            if (body->shape == nv_BodyShape_CIRCLE) {
                double x = body->position.x * 10.0;
                double y = body->position.y * 10.0;

                nv_Vector2 a = (nv_Vector2){body->radius*10.0, 0.0};
                a = nv_Vector2_rotate(a, body->angle);

                draw_circle(
                    example->renderer,
                    (int32_t)x,
                    (int32_t)y,
                    (int32_t)(body->radius * 10.0)
                    );

                SDL_RenderDrawLineF(example->renderer, x, y, x+a.x, y+a.y);
            }
            else {
                nv_Vector2Array *vertices = nv_Polygon_model_to_world(body);
                draw_polygon(example->renderer, vertices);
                nv_Vector2Array_free(vertices);
            }
        }

        if (selected) {
            SDL_SetRenderDrawColor(example->renderer, 0, 255, 50, 255);
            SDL_RenderDrawLineF(
                example->renderer,
                selected->position.x * 10.0, selected->position.y * 10.0,
                example->mouse.x, example->mouse.y
                );
        };

        struct SDL_version sdl_ver;
        SDL_GetVersion(&sdl_ver);
        char text_sdlver[32];
        sprintf(text_sdlver, "SDL %d.%d.%d", sdl_ver.major, sdl_ver.minor, sdl_ver.patch);

        char text_novaver[32];
        sprintf(text_novaver, "Nova Physics %s", NV_VERSTR);

        char text_instr[21] = "Click & drag bodies";

        char text_fps[32];
        sprintf(text_fps, "FPS: %d", (int)example->fps);

        char text_steptime[32];
        sprintf(text_steptime, "Step time: %dms", step_time);

        char text_rendertime[32];
        sprintf(text_rendertime, "Render time: %dms", render_time);

        char text_bodies[32];
        sprintf(text_bodies, "Bodies: %d", example->space->bodies->size);

        char text_subs[32];
        sprintf(text_subs, "Substeps: %d", example->substeps);

        char text_iters[32];
        sprintf(text_iters, "Iters: %d", example->iters);

        
        draw_text(font, example->renderer, text_sdlver, 1280-80, 5, (SDL_Color){0, 0, 0});
        draw_text(font, example->renderer, text_novaver, 1280-138, 23, (SDL_Color){0, 0, 0});
        draw_text(font, example->renderer, text_instr, 1280-145, 61, (SDL_Color){90, 90, 96});

        draw_text(font, example->renderer, text_fps, 5, 5, (SDL_Color){0, 0, 0});
        draw_text(font, example->renderer, text_steptime, 70, 5, (SDL_Color){0, 0, 0});
        draw_text(font, example->renderer, text_rendertime, 190, 5, (SDL_Color){0, 0, 0});

        draw_text(font, example->renderer, text_bodies, 5, 23, (SDL_Color){0, 0, 0});
        draw_text(font, example->renderer, text_iters, 100, 23, (SDL_Color){0, 0, 0});
        draw_text(font, example->renderer, text_subs, 160, 23, (SDL_Color){0, 0, 0});

        SDL_RenderPresent(example->renderer);
        render_time = SDL_GetTicks() - render_time_start;


        SDL_Delay((1.0 / example->max_fps) * 1000.0);

        end_time = SDL_GetTicks();
        example->dt = (end_time - start_time) / 1000.0;
		example->fps = 1.0 / example->dt;
    }
}