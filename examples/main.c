/*

  This file is a part of the Nova Physics Engine
  project and distributed under the MIT license.

  Copyright © Kadir Aksoy
  https://github.com/kadir014/nova-physics

*/

#include "common.h"
#include "ngl.h"
#include "clock.h"

// I wish #include "demos/*.h" was a standard :(
#include "demos/stack.h"
#include "demos/compound.h"
#include "demos/constraints.h"


/**
 * @file examples/main.c
 * 
 * This is the entry point for example demos app.
 * Look at demos subfolder for individual examples.
 */


#define NUKLEAR_MAX_VERTEX_MEMORY 100 * 1024
#define NUKLEAR_MAX_ELEMENT_MEMORY 25 * 1024

#define EXAMPLE_MAX_TRIANGlES 75 * 1024
#define EXAMPLE_MAX_TRI_VERTICES EXAMPLE_MAX_TRIANGlES * 6
#define EXAMPLE_MAX_TRI_COLORS EXAMPLE_MAX_TRIANGlES * 4 * 3
#define EXAMPLE_MAX_LINE_VERTICES EXAMPLE_MAX_TRIANGlES * 2
#define EXAMPLE_MAX_LINE_COLORS EXAMPLE_MAX_TRIANGlES * 4

#define ZOOM_SCALE 0.075

#define ADD_TRIANGLE(x0, y0, x1, y1, x2, y2, r, g, b, a) { \
    tri_vertices[tri_vertices_index]     = (float)x0;      \
    tri_vertices[tri_vertices_index + 1] = (float)y0;      \
    tri_vertices[tri_vertices_index + 2] = (float)x1;      \
    tri_vertices[tri_vertices_index + 3] = (float)y1;      \
    tri_vertices[tri_vertices_index + 4] = (float)x2;      \
    tri_vertices[tri_vertices_index + 5] = (float)y2;      \
    tri_vertices_index += 6;                               \
                                                           \
    for (size_t j = 0; j < 3; j++) {                       \
        tri_colors[tri_colors_index]     = (float)r;       \
        tri_colors[tri_colors_index + 1] = (float)g;       \
        tri_colors[tri_colors_index + 2] = (float)b;       \
        tri_colors[tri_colors_index + 3] = (float)a;       \
        tri_colors_index += 4;                             \
    }                                                      \
                                                           \
    vao0_count += 3;                                       \
}

#define ADD_LINE(x, y, r, g, b, a) {                   \
    line_vertices[line_vertices_index]     = (float)x; \
    line_vertices[line_vertices_index + 1] = (float)y; \
    line_vertices_index += 2;                          \
                                                       \
    line_colors[line_colors_index]     = (float)r;     \
    line_colors[line_colors_index + 1] = (float)g;     \
    line_colors[line_colors_index + 2] = (float)b;     \
    line_colors[line_colors_index + 3] = (float)a;     \
    line_colors_index += 4;                            \
                                                       \
    vao1_count += 1;                                   \
}

ExampleEntry example_entries[EXAMPLE_MAX_ENTRIES] = {NULL};
size_t example_count = 0;
size_t current_example = 0;

void ExampleEntry_register(
    char *name,
    ExampleCallback setup,
    ExampleCallback update
) {
    example_entries[example_count++] = (ExampleEntry){
        .name=name,
        .setup=setup,
        .update=update
    };
}

void ExampleEntry_set_current(char *name) {
    for (size_t i = 0; i < example_count; i++) {
        if (!strcmp(name, example_entries[i].name)) {
            current_example = i;
            return;
        }
    }
}

void ExampleContext_apply_settings(
    ExampleContext *example,
    ExampleSettings settings
) {
    example->window_width = settings.window_width;
    example->window_height = settings.window_height;
}

void setup_ui(ExampleContext *example) {
    example->ui_ctx = nk_sdl_init(example->window);

    struct nk_color accent = nk_rgb(
        (int)(example->theme.ui_accent.r * 255.0),
        (int)(example->theme.ui_accent.g * 255.0),
        (int)(example->theme.ui_accent.b * 255.0)
    );
    struct nk_color accent_light = nk_rgb(
        (int)((example->theme.ui_accent.r + 0.1) * 255.0),
        (int)((example->theme.ui_accent.g + 0.1) * 255.0),
        (int)((example->theme.ui_accent.b + 0.1) * 255.0)
    );
    struct nk_color text = nk_rgb(
        (int)(example->theme.ui_text.r * 255.0),
        (int)(example->theme.ui_text.g * 255.0),
        (int)(example->theme.ui_text.b * 255.0)
    );

    example->ui_ctx->style.window.fixed_background = nk_style_item_color(nk_rgba(17, 17, 20, 210));
    example->ui_ctx->style.window.border = 0;
    example->ui_ctx->style.window.header.active = nk_style_item_color(accent);
    example->ui_ctx->style.window.header.normal = nk_style_item_color(accent);
    example->ui_ctx->style.window.header.label_active = text;
    example->ui_ctx->style.window.header.label_normal = text;
    example->ui_ctx->style.window.header.label_padding = (struct nk_vec2){5.0, 2.0};
    example->ui_ctx->style.window.header.minimize_button.text_active = text;
    example->ui_ctx->style.window.header.minimize_button.text_normal = text;
    example->ui_ctx->style.window.header.minimize_button.text_hover = text;
    example->ui_ctx->style.window.header.minimize_button.active = nk_style_item_color(nk_rgba(255, 255, 255, 80));
    example->ui_ctx->style.window.header.minimize_button.hover = nk_style_item_color(nk_rgba(255, 255, 255, 80));
    example->ui_ctx->style.window.header.minimize_button.normal = nk_style_item_color(nk_rgba(0, 0, 0, 0));
    example->ui_ctx->style.window.header.padding = (struct nk_vec2){5.0, 2.0};
    example->ui_ctx->style.window.padding = (struct nk_vec2){5.0, 6.0};
    example->ui_ctx->style.text.color = text;

    example->ui_ctx->style.button.rounding = 0;
    example->ui_ctx->style.button.active = nk_style_item_color(accent);
    example->ui_ctx->style.button.text_active = text;

    example->ui_ctx->style.checkbox.text_active = text;
    example->ui_ctx->style.checkbox.text_normal = text;
    example->ui_ctx->style.checkbox.text_hover = text;
    example->ui_ctx->style.checkbox.padding = (struct nk_vec2){3.0, 3.0};
    example->ui_ctx->style.checkbox.active = nk_style_item_color(nk_rgb(37, 36, 38));
    example->ui_ctx->style.checkbox.hover = nk_style_item_color(nk_rgb(55, 53, 56));
    example->ui_ctx->style.checkbox.normal = nk_style_item_color(nk_rgb(37, 36, 38));
    example->ui_ctx->style.checkbox.cursor_normal = nk_style_item_color(accent);
    example->ui_ctx->style.checkbox.cursor_hover = nk_style_item_color(accent);

    example->ui_ctx->style.slider.cursor_normal = nk_style_item_color(accent);
    example->ui_ctx->style.slider.cursor_hover = nk_style_item_color(accent_light);
    example->ui_ctx->style.slider.cursor_active = nk_style_item_color(accent_light);
    example->ui_ctx->style.slider.bar_filled = accent;

    struct nk_font_atlas *atlas;
    nk_sdl_font_stash_begin(&atlas);
    struct nk_font *font = nk_font_atlas_add_from_file(atlas, "C:/Users/bjkka/Desktop/Git/nova-physics/build/assets/FiraCode-Medium.ttf", 16, 0);
    nk_sdl_font_stash_end();

    nk_style_set_font(example->ui_ctx, &font->handle);
}

// Transform (normalize) coordinate from screen space to opengl space [-1, 1]
static inline nvVector2 normalize_coords(ExampleContext *example, nvVector2 v) {
    return NV_VECTOR2(
        (2.0 * v.x / example->window_width) - 1.0,
        1.0 - (2.0 * v.y / example->window_height)
    );
}

// Transform coordinate from world space to screen space
static inline nvVector2 world_to_screen(ExampleContext *example, nvVector2 world_pos) {
    return nvVector2_mul(nvVector2_sub(world_pos, example->camera), example->zoom);
}

// Transform coordinate from screen space to world space
static inline nvVector2 screen_to_world(ExampleContext *example, nvVector2 screen_pos) {
    return nvVector2_add(nvVector2_div(screen_pos, example->zoom), example->camera);
}


int main(int argc, char *argv[]) {
    srand((unsigned int)time(NULL));

    ExampleSettings settings = {
        .window_width = 1280,
        .window_height = 720
    };

    ExampleContext example;
    ExampleContext_apply_settings(&example, settings);

    Clock *clock = Clock_new();

    example.mouse.left = false;
    example.mouse.right = false;
    example.mouse.middle = false;
    example.camera = nvVector2_zero;
    example.zoom = 10.0;

    example.theme.dynamic_body = (FColor){1.0, 0.75, 0.29, 1.0};
    example.theme.static_body = (FColor){0.78, 0.44, 0.23, 1.0};
    example.theme.distance_constraint = (FColor){0.45, 0.87, 1.0, 1.0};
    example.theme.hinge_constraint = (FColor){0.623, 0.47, 0.98, 1.0},
    example.theme.ui_accent = (FColor){0.486, 0.243, 0.968, 1.0};
    example.theme.ui_text = (FColor){1.0, 1.0, 1.0, 1.0};

    if (SDL_Init(SDL_INIT_EVERYTHING) != 0) {
	    fprintf(stderr, "SDL initialization error: %s\n", SDL_GetError());
        exit(EXIT_FAILURE);
	}

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1);
    SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 8);

    example.window = SDL_CreateWindow(
        "Nova Examples",
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        example.window_width,
        example.window_height,
        SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE | SDL_WINDOW_OPENGL | SDL_WINDOW_ALLOW_HIGHDPI
    );
    if (!example.window) {
        fprintf(stderr, SDL_GetError());
        exit(EXIT_FAILURE);
    }

    example.gl_ctx = SDL_GL_CreateContext(example.window);
    if (!example.gl_ctx) {
        fprintf(stderr, SDL_GetError());
        exit(EXIT_FAILURE);
    }
    SDL_GL_MakeCurrent(example.window, example.gl_ctx);

    if (!gladLoadGLLoader((GLADloadproc)SDL_GL_GetProcAddress)) {
        fprintf(stderr, "Failed to initialize GLAD.\n");
        exit(EXIT_FAILURE);
    }

    setup_ui(&example);

    // Enable VSYNC
    SDL_GL_SetSwapInterval(1);

    const char *vertex_shader_src =
"#version 330 core\n"
"layout (location = 0) in vec2 in_pos;\n"
"layout (location = 1) in vec4 in_color;\n"
"out vec4 v_color;\n"
"void main() {\n"
"    gl_Position = vec4(in_pos.x, in_pos.y, 0.0, 1.0);\n"
"    v_color = in_color;\n"
"}\n";

    const char *fragment_shader_src =
"#version 330 core\n"
"in vec4 v_color;\n"
"out vec4 f_color;\n"
"void main() {\n"
"    f_color = v_color;\n"
"}\n";

    nv_uint32 vertex_shader = ngl_load_shader(vertex_shader_src, GL_VERTEX_SHADER);
    nv_uint32 fragment_shader = ngl_load_shader(fragment_shader_src, GL_FRAGMENT_SHADER);

    nv_uint32 program = glCreateProgram();
    glAttachShader(program, vertex_shader);
    glAttachShader(program, fragment_shader);
    glLinkProgram(program);
    int success;
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if(!success) {
        fprintf(stderr, "Shader program linking error.\n");
        exit(EXIT_FAILURE);
    }
    glUseProgram(program);
    glDeleteShader(vertex_shader);
    glDeleteShader(fragment_shader);

    glLineWidth(1.0);
    glEnable(GL_LINE_SMOOTH);

    size_t tri_vertices_size = sizeof(float) * EXAMPLE_MAX_TRI_VERTICES;
    float *tri_vertices = malloc(tri_vertices_size);
    size_t tri_vertices_index = 0;

    size_t tri_colors_size = sizeof(float) * EXAMPLE_MAX_TRI_COLORS;
    float *tri_colors = malloc(tri_colors_size);
    size_t tri_colors_index = 0;

    size_t line_vertices_size = sizeof(float) * EXAMPLE_MAX_LINE_VERTICES;
    float *line_vertices = malloc(line_vertices_size);
    size_t line_vertices_index = 0;

    size_t line_colors_size = sizeof(float) * EXAMPLE_MAX_LINE_COLORS;
    float *line_colors = malloc(line_colors_size);
    size_t line_colors_index = 0;

    nv_uint32 vbos[4]; 
    vbos[0] = ngl_create_vbo();
    vbos[1] = ngl_create_vbo();
    vbos[2] = ngl_create_vbo();
    vbos[3] = ngl_create_vbo();

    nv_uint32 vaos[2];
    vaos[0] = ngl_create_vao();
    vaos[1] = ngl_create_vao();

    nv_uint32 vertex_attr = 0;
    nv_uint32 color_attr = 1;

    glBindVertexArray(vaos[0]);
    size_t vao0_count = 0;

    glBindBuffer(GL_ARRAY_BUFFER, vbos[0]);
    glBufferData(GL_ARRAY_BUFFER, tri_vertices_size, tri_vertices, GL_DYNAMIC_DRAW);
    glVertexAttribPointer(vertex_attr, 2, GL_FLOAT, GL_FALSE, 0, (void *)0);
    glEnableVertexAttribArray(vertex_attr);

    glBindBuffer(GL_ARRAY_BUFFER, vbos[1]);
    glBufferData(GL_ARRAY_BUFFER, tri_colors_size, tri_colors, GL_DYNAMIC_DRAW);
    glVertexAttribPointer(color_attr, 4, GL_FLOAT, GL_FALSE, 0, (void *)0);
    glEnableVertexAttribArray(color_attr);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    glBindVertexArray(vaos[1]);
    size_t vao1_count = 0;

    glBindBuffer(GL_ARRAY_BUFFER, vbos[2]);
    glBufferData(GL_ARRAY_BUFFER, line_vertices_size, line_vertices, GL_DYNAMIC_DRAW);
    glVertexAttribPointer(vertex_attr, 2, GL_FLOAT, GL_FALSE, 0, (void *)0);
    glEnableVertexAttribArray(vertex_attr);

    glBindBuffer(GL_ARRAY_BUFFER, vbos[3]);
    glBufferData(GL_ARRAY_BUFFER, line_colors_size, line_colors, GL_DYNAMIC_DRAW);
    glVertexAttribPointer(color_attr, 4, GL_FLOAT, GL_FALSE, 0, (void *)0);
    glEnableVertexAttribArray(color_attr);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    nv_bool is_running = true;

    nv_uint64 frame = 0;

    example.space = nvSpace_new();

    // UI settings
    int draw_ui = 1;
    int space_paused = 0;
    int show_bytes = 0;
    int draw_contacts = 0;
    int draw_aabbs = 0;
    int draw_constraints = 1;
    int draw_positions = 1;

    nvPrecisionTimer render_timer;
    double render_time = 0.0;
    double old_render_time = 0.0;

    printf("Nova Physics %s\n", NV_VERSION_STRING);
    printf("nv_float size: %llu bytes\n", (unsigned long long)sizeof(nv_float));

    // Register all example demos
    ExampleEntry_register("Stack", Stack_setup, Stack_update);
    ExampleEntry_register("Compound", Compound_setup, Compound_update);
    ExampleEntry_register("Constraints", Constraints_setup, Constraints_update);
    current_example = 0;

    example_entries[current_example].setup(&example);

    nvConstraint *mouse_cons = NULL;
    nvDistanceConstraintInitializer mouse_cons_init = nvDistanceConstraintInitializer_default;

    while (is_running) {
        Clock_tick(clock, 60.0);

        old_render_time = render_time;
        render_time = 0.0;

        SDL_GetMouseState(&example.mouse.x, &example.mouse.y);
        example.before_zoom = screen_to_world(&example, NV_VECTOR2(example.mouse.x, example.mouse.y));

        SDL_Event event;
        nk_input_begin(example.ui_ctx);
        while (SDL_PollEvent(&event) != 0) {
            if (event.type == SDL_QUIT) {
                is_running = false;
            }

            else if (event.type == SDL_MOUSEBUTTONDOWN) {
                if (event.button.button == SDL_BUTTON_LEFT) {
                    example.mouse.left = true;

                    for (size_t i = 0; i < example.space->bodies->size; i++) {
                        nvRigidBody *body = example.space->bodies->data[i];

                        nvAABB aabb = nvRigidBody_get_aabb(body);

                        if (nv_collide_aabb_x_point(aabb, example.before_zoom)) {
                            nvVector2 anchor = nvVector2_rotate(nvVector2_sub(example.after_zoom, nvRigidBody_get_position(body)), -nvRigidBody_get_angle(body));
                            mouse_cons_init.a = body;
                            mouse_cons_init.b = NULL;
                            mouse_cons_init.length = 0.1;
                            mouse_cons_init.anchor_a = nvVector2_add(anchor, NV_VECTOR2(0.0, 0.01));
                            mouse_cons_init.anchor_b = example.before_zoom;
                            mouse_cons_init.spring = true;
                            mouse_cons_init.hertz = 5.0;
                            mouse_cons_init.damping = 0.9;
                            mouse_cons = nvDistanceConstraint_new(mouse_cons_init);
                            nvSpace_add_constraint(example.space, mouse_cons);
                            break;
                        }
                    }
                }

                else if (event.button.button == SDL_BUTTON_MIDDLE) {
                    example.mouse.middle = true;
                    example.pan_start = NV_VECTOR2(example.mouse.x, example.mouse.y);
                }

                else if (event.button.button == SDL_BUTTON_RIGHT) {
                    example.mouse.right = true;
                }
            }

            else if (event.type == SDL_MOUSEBUTTONUP) {
                if (event.button.button == SDL_BUTTON_LEFT) {
                    example.mouse.left = false;

                    nvSpace_remove_constraint(example.space, mouse_cons);
                    nvConstraint_free(mouse_cons);
                    mouse_cons = NULL;
                }

                else if (event.button.button == SDL_BUTTON_MIDDLE) {
                    example.mouse.middle = false;
                }

                else if (event.button.button == SDL_BUTTON_RIGHT) {
                    example.mouse.right = false;
                }
            }

            else if (event.type == SDL_MOUSEWHEEL) {
                if (event.wheel.y > 0) {
                    example.zoom *= 1 + ZOOM_SCALE;
                }

                else if (event.wheel.y < 0) {
                    example.zoom *= 1 - ZOOM_SCALE;
                }
            }

            else if (
                event.type == SDL_WINDOWEVENT &&
                event.window.event == SDL_WINDOWEVENT_RESIZED
            ) {
                example.window_width = event.window.data1;
                example.window_height = event.window.data2;
            }

            else if (event.type == SDL_KEYDOWN) {
                if (event.key.keysym.scancode == SDL_SCANCODE_ESCAPE) {
                    is_running = false;
                }

                else if (event.key.keysym.scancode == SDL_SCANCODE_U) {
                    draw_ui = !draw_ui;
                }

                else if (event.key.keysym.scancode == SDL_SCANCODE_P) {
                    space_paused = !space_paused;
                }
            }

            nk_sdl_handle_event(&event);
        }
        nk_sdl_handle_grab();
        nk_input_end(example.ui_ctx);

        example.after_zoom = screen_to_world(&example, NV_VECTOR2(example.mouse.x, example.mouse.y));

        if (example.mouse.middle) {
            example.camera = nvVector2_sub(example.camera, nvVector2_div(nvVector2_sub(NV_VECTOR2(example.mouse.x, example.mouse.y), example.pan_start), example.zoom));
            example.pan_start = NV_VECTOR2(example.mouse.x, example.mouse.y);
        }
        example.camera = nvVector2_add(example.camera, nvVector2_sub(example.before_zoom, example.after_zoom));

        if (mouse_cons) {
            nvDistanceConstraint_set_anchor_b(mouse_cons, example.before_zoom);
        }

        if (draw_ui) {
        if (nk_begin(example.ui_ctx, "Simulation", nk_rect(0.0f, 0.0f, 300.0f, (float)example.window_height), NK_WINDOW_TITLE)) {
            char display_buf[16];
            const float ratio[] = {0.40f, 0.47f, 0.13f};

            if (nk_tree_push(example.ui_ctx, NK_TREE_TAB, "Space Settings", NK_MAXIMIZED)) {
                nvSpaceSettings *settings = &example.space->settings;
                {
                    nk_layout_row(example.ui_ctx, NK_DYNAMIC, 16, 3, ratio);

                    nk_label(example.ui_ctx, "Gravity", NK_TEXT_LEFT);

                    nk_slider_float(example.ui_ctx, 0.0f, (float *)&example.space->gravity.y, 50.0f, 0.005f);
                    
                    sprintf(display_buf, "%3.2f", example.space->gravity.y);
                    nk_label(example.ui_ctx, display_buf, NK_TEXT_LEFT);
                }
                {
                    nk_layout_row(example.ui_ctx, NK_DYNAMIC, 16, 3, ratio);

                    nk_label(example.ui_ctx, "Baumgarte", NK_TEXT_LEFT);

                    nk_slider_float(example.ui_ctx, 0.0f, (float *)&settings->baumgarte, 1.0f, 0.005f);
                    
                    sprintf(display_buf, "%3.2f", settings->baumgarte);
                    nk_label(example.ui_ctx, display_buf, NK_TEXT_LEFT);
                }
                {
                    nk_layout_row(example.ui_ctx, NK_DYNAMIC, 16, 3, ratio);

                    nk_label(example.ui_ctx, "Velocity Iters", NK_TEXT_LEFT);

                    nk_slider_int(example.ui_ctx, 0, (nv_uint32 *)&settings->velocity_iterations, 30, 1);
                    
                    sprintf(display_buf, "%u", settings->velocity_iterations);
                    nk_label(example.ui_ctx, display_buf, NK_TEXT_LEFT);
                }
                {
                    nk_layout_row(example.ui_ctx, NK_DYNAMIC, 16, 3, ratio);

                    nk_label(example.ui_ctx, "Substeps", NK_TEXT_LEFT);

                    nk_slider_int(example.ui_ctx, 1, (nv_uint32 *)&settings->substeps, 5, 1);
                    
                    sprintf(display_buf, "%u", settings->substeps);
                    nk_label(example.ui_ctx, display_buf, NK_TEXT_LEFT);
                }

                nk_layout_row_dynamic(example.ui_ctx, 20, 1);
                nk_checkbox_label(example.ui_ctx, "Warmstarting", &settings->warmstarting);

                nk_layout_row_static(example.ui_ctx, 30, 120, 1);
                nk_checkbox_label(example.ui_ctx, "Paused", &space_paused);

                nk_tree_pop(example.ui_ctx);
            }

            if (nk_tree_push(example.ui_ctx, NK_TREE_TAB, "Drawing", NK_MINIMIZED)) {
                nk_layout_row_dynamic(example.ui_ctx, 16, 1);

                nk_checkbox_label(example.ui_ctx, "AABBs", &draw_aabbs);
                nk_checkbox_label(example.ui_ctx, "Contacts", &draw_contacts);
                nk_checkbox_label(example.ui_ctx, "Constraints", &draw_constraints);
                nk_checkbox_label(example.ui_ctx, "Positions", &draw_positions);
                nk_checkbox_label(example.ui_ctx, "Velocities", &space_paused);
                nk_checkbox_label(example.ui_ctx, "Normal impulses", &space_paused);
                nk_checkbox_label(example.ui_ctx, "Friction impulses", &space_paused);

                nk_tree_pop(example.ui_ctx);
            }

            if (nk_tree_push(example.ui_ctx, NK_TREE_TAB, "Demos", NK_MINIMIZED)) {
                nk_layout_row_dynamic(example.ui_ctx, 16, 1);

                for (size_t i = 0; i < example_count; i++) {
                    if (nk_button_label(example.ui_ctx, example_entries[i].name)) {
                        current_example = i;
                        nvSpace_clear(example.space, true);
                        mouse_cons = NULL;
                        example_entries[current_example].setup(&example);
                    }
                }

                nk_tree_pop(example.ui_ctx);
            }
        }
        nk_end(example.ui_ctx);

        if (
            nk_begin(
                example.ui_ctx,
                "Profile",
                nk_rect((float)example.window_width - 250.0f, 0.0f, 250.0f, 400.0f),
                NK_WINDOW_TITLE | NK_WINDOW_MINIMIZABLE
            )
        ) {
            char fmt_buffer[32];

            if (nk_tree_push(example.ui_ctx, NK_TREE_TAB, "Overview", NK_MAXIMIZED)) {
                nk_layout_row_dynamic(example.ui_ctx, 16, 1);

                sprintf(fmt_buffer, "FPS: %.1f", clock->fps);
                nk_label(example.ui_ctx, fmt_buffer, NK_TEXT_LEFT);

                sprintf(fmt_buffer, "Physics: %.3fms", example.space->profiler.step * 1000.0);
                nk_label(example.ui_ctx, fmt_buffer, NK_TEXT_LEFT);

                sprintf(fmt_buffer, "Render: %.3fms", old_render_time);
                nk_label(example.ui_ctx, fmt_buffer, NK_TEXT_LEFT);

                sprintf(fmt_buffer, "Frame: %llu", (unsigned long long)frame);
                nk_label(example.ui_ctx, fmt_buffer, NK_TEXT_LEFT);

                nk_tree_pop(example.ui_ctx);
            }

            if (nk_tree_push(example.ui_ctx, NK_TREE_TAB, "Physics", NK_MINIMIZED)) {
                nk_layout_row_dynamic(example.ui_ctx, 16, 1);

                sprintf(fmt_buffer, "Step: %.3fms", example.space->profiler.step * 1000.0);
                nk_label(example.ui_ctx, fmt_buffer, NK_TEXT_LEFT);

                sprintf(fmt_buffer, "Broadphase: %.3fms", example.space->profiler.broadphase * 1000.0);
                nk_label(example.ui_ctx, fmt_buffer, NK_TEXT_LEFT);

                sprintf(fmt_buffer, "Narrowphase: %.3fms", example.space->profiler.narrowphase * 1000.0);
                nk_label(example.ui_ctx, fmt_buffer, NK_TEXT_LEFT);

                sprintf(fmt_buffer, "Presolve: %.3fms", example.space->profiler.presolve * 1000.0);
                nk_label(example.ui_ctx, fmt_buffer, NK_TEXT_LEFT);

                sprintf(fmt_buffer, "Warmstart: %.3fms", example.space->profiler.warmstart * 1000.0);
                nk_label(example.ui_ctx, fmt_buffer, NK_TEXT_LEFT);

                sprintf(fmt_buffer, "Solve velocity: %.3fms", example.space->profiler.solve_velocities * 1000.0);
                nk_label(example.ui_ctx, fmt_buffer, NK_TEXT_LEFT);

                sprintf(fmt_buffer, "Solve position: %.3fms", example.space->profiler.solve_positions * 1000.0);
                nk_label(example.ui_ctx, fmt_buffer, NK_TEXT_LEFT);

                sprintf(fmt_buffer, "Integrate vels.: %.3fms", example.space->profiler.integrate_velocities * 1000.0);
                nk_label(example.ui_ctx, fmt_buffer, NK_TEXT_LEFT);

                sprintf(fmt_buffer, "Integrate accels.: %.3fms", example.space->profiler.integrate_velocities * 1000.0);
                nk_label(example.ui_ctx, fmt_buffer, NK_TEXT_LEFT);

                nk_tree_pop(example.ui_ctx);
            }

            if (nk_tree_push(example.ui_ctx, NK_TREE_TAB, "Memory", NK_MINIMIZED)) {
                nk_layout_row_dynamic(example.ui_ctx, 16, 1);

                size_t process_mem = get_current_memory_usage();
                sprintf(fmt_buffer, "Process: %1.f MB", (double)process_mem / 1048576.0);
                nk_label(example.ui_ctx, fmt_buffer, NK_TEXT_LEFT);

                nk_checkbox_label(example.ui_ctx, "Show in bytes", &show_bytes);
                char *unit;
                if (show_bytes) unit = "B";
                else unit = "KB";

                double unit_size;
                if (show_bytes) unit_size = 1.0;
                else unit_size = 1024.0;

                size_t num_bodies = example.space->bodies->size;
                size_t bodies_bytes = num_bodies * sizeof(nvRigidBody);
                double bodies_s = (double)(bodies_bytes) / unit_size;

                size_t num_cons = example.space->constraints->size;
                size_t cons_bytes = num_cons * sizeof(nvConstraint);
                double cons_s = (double)(cons_bytes) / unit_size;

                size_t num_contacts = example.space->contacts->count;
                size_t contacts_bytes = num_contacts * sizeof(nvPersistentContactPair);
                double contacts_s = (double)(contacts_bytes) / unit_size;

                size_t space_bytes =
                    sizeof(nvSpace) +
                    bodies_bytes +
                    cons_bytes +
                    example.space->broadphase_pairs->size * sizeof(nvBroadPhasePair) +
                    contacts_bytes;
                double space_s = (double)space_bytes / unit_size;

                if (!show_bytes && space_s > 1024.0) {
                    space_s /= 1024.0;
                    unit = "MB";
                }
                sprintf(fmt_buffer, "Space: %.1f %s", space_s, unit);
                nk_label(example.ui_ctx, fmt_buffer, NK_TEXT_LEFT);

                if (!show_bytes && bodies_s > 1024.0) {
                    bodies_s /= 1024.0;
                    unit = "MB";
                }
                sprintf(fmt_buffer, "Bodies: %llu (%.1f %s)", (unsigned long long)num_bodies, bodies_s, unit);
                nk_label(example.ui_ctx, fmt_buffer, NK_TEXT_LEFT);

                if (!show_bytes && cons_s > 1024.0) {
                    cons_s /= 1024.0;
                    unit = "MB";
                }
                sprintf(fmt_buffer, "Constraints: %llu (%.1f %s)", (unsigned long long)num_cons, cons_s, unit);
                nk_label(example.ui_ctx, fmt_buffer, NK_TEXT_LEFT);

                if (!show_bytes && contacts_s > 1024.0) {
                    contacts_s /= 1024.0;
                    unit = "MB";
                }
                sprintf(fmt_buffer, "Contacts: %llu (%.1f %s)", (unsigned long long)num_contacts, contacts_s, unit);
                nk_label(example.ui_ctx, fmt_buffer, NK_TEXT_LEFT);

                nk_tree_pop(example.ui_ctx);
            }
        }
        nk_end(example.ui_ctx);

        if (
            nk_begin(
                example.ui_ctx,
                example_entries[current_example].name,
                nk_rect((float)example.window_width - 250.0f, example.window_height - 300.0f, 250.0f, 300.0f),
                NK_WINDOW_TITLE | NK_WINDOW_MOVABLE | NK_WINDOW_MINIMIZABLE
            )
        ) {
            example_entries[current_example].update(&example);
        }
        nk_end(example.ui_ctx);
        }

        if (!space_paused) {
            nvSpace_step(example.space, 1.0 / 60.0);
        }

        nvPrecisionTimer_start(&render_timer);

        tri_vertices_index = 0;
        tri_colors_index = 0;
        vao0_count = 0;
        line_vertices_index = 0;
        line_colors_index = 0;
        vao1_count = 0;

        for (size_t i = 0; i < example.space->bodies->size; i++) {
            nvRigidBody *body = example.space->bodies->data[i];
            nvAABB aabb = nvRigidBody_get_aabb(body);

            double r, g, b;
            if (body->type == nvRigidBodyType_DYNAMIC) {
                r = example.theme.dynamic_body.r;
                g = example.theme.dynamic_body.g;
                b = example.theme.dynamic_body.b;
            }
            else {
                r = example.theme.static_body.r;
                g = example.theme.static_body.g;
                b = example.theme.static_body.b;
            }

            for (size_t k = 0; k < body->shapes->size; k++) {
                nvShape *shape = body->shapes->data[k];

                if (shape->type == nvShapeType_POLYGON) {
                    nvPolygon_transform(shape, (nvTransform){body->origin, nvRigidBody_get_angle(body)});
                    nvPolygon polygon = shape->polygon;
                    nvVector2 v0 = polygon.xvertices[0];
                    nvVector2 v0t = world_to_screen(&example, v0);

                    v0t = normalize_coords(&example, v0t);
                    for (size_t j = 0; j < polygon.num_vertices - 2; j++) {
                        nvVector2 v1 = polygon.xvertices[j + 1];
                        nvVector2 v2 = polygon.xvertices[j + 2];

                        nvVector2 v1t = world_to_screen(&example, v1);
                        nvVector2 v2t = world_to_screen(&example, v2);

                        v1t = normalize_coords(&example, v1t);
                        v2t = normalize_coords(&example, v2t);

                        ADD_TRIANGLE(
                            v0t.x, v0t.y,
                            v1t.x, v1t.y,
                            v2t.x, v2t.y,
                            r,
                            g,
                            b,
                            0.1
                        );
                    }

                    ADD_LINE(v0t.x, v0t.y, 0.0, 0.0, 0.0, 0.0);

                    for (size_t j = 0; j < polygon.num_vertices; j++) {
                        nvVector2 va = polygon.xvertices[j];
                        nvVector2 vat = world_to_screen(&example, va);
                        vat = normalize_coords(&example, vat);

                        ADD_LINE(vat.x, vat.y, r, g, b, 1.0;)
                    }

                    // The reason we add 2 more extra vertices per object is to
                    // basically a transparent line between objects. I currently
                    // have no idea how to remove the linked lines in GL_LINE_STRIP
                    // drawing mode but I believe this is efficient enough.
                    ADD_LINE(v0t.x, v0t.y, r, g, b, 1.0);
                    ADD_LINE(v0t.x, v0t.y, 0.0, 0.0, 0.0, 0.0);
                }

                if (draw_aabbs) {
                    nvAABB saabb = nvShape_get_aabb(shape, (nvTransform){body->origin, body->angle});
                    nvVector2 p0 = NV_VECTOR2(saabb.min_x, saabb.min_y);
                    nvVector2 p1 = NV_VECTOR2(saabb.max_x, saabb.min_y);
                    nvVector2 p2 = NV_VECTOR2(saabb.max_x, saabb.max_y);
                    nvVector2 p3 = NV_VECTOR2(saabb.min_x, saabb.max_y);
                    p0 = world_to_screen(&example, p0);
                    p0 = normalize_coords(&example, p0);
                    p1 = world_to_screen(&example, p1);
                    p1 = normalize_coords(&example, p1);
                    p2 = world_to_screen(&example, p2);
                    p2 = normalize_coords(&example, p2);
                    p3 = world_to_screen(&example, p3);
                    p3 = normalize_coords(&example, p3);

                    ADD_LINE(p0.x, p0.y, 0.0, 0.0, 0.0, 0.0);
                    ADD_LINE(p0.x, p0.y, 0.0, 1.0, 0.0, 0.4);
                    ADD_LINE(p1.x, p1.y, 0.0, 1.0, 0.0, 0.4);
                    ADD_LINE(p2.x, p2.y, 0.0, 1.0, 0.0, 0.4);
                    ADD_LINE(p3.x, p3.y, 0.0, 1.0, 0.0, 0.4);
                    ADD_LINE(p0.x, p0.y, 0.0, 1.0, 0.0, 0.4);
                    ADD_LINE(p0.x, p0.y, 0.0, 0.0, 0.0, 0.0);
                }
            }

            if (draw_aabbs) {
                nvVector2 p0 = NV_VECTOR2(aabb.min_x, aabb.min_y);
                nvVector2 p1 = NV_VECTOR2(aabb.max_x, aabb.min_y);
                nvVector2 p2 = NV_VECTOR2(aabb.max_x, aabb.max_y);
                nvVector2 p3 = NV_VECTOR2(aabb.min_x, aabb.max_y);
                p0 = world_to_screen(&example, p0);
                p0 = normalize_coords(&example, p0);
                p1 = world_to_screen(&example, p1);
                p1 = normalize_coords(&example, p1);
                p2 = world_to_screen(&example, p2);
                p2 = normalize_coords(&example, p2);
                p3 = world_to_screen(&example, p3);
                p3 = normalize_coords(&example, p3);

                ADD_LINE(p0.x, p0.y, 0.0, 0.0, 0.0, 0.0);
                ADD_LINE(p0.x, p0.y, 0.0, 1.0, 0.0, 1.0);
                ADD_LINE(p1.x, p1.y, 0.0, 1.0, 0.0, 1.0);
                ADD_LINE(p2.x, p2.y, 0.0, 1.0, 0.0, 1.0);
                ADD_LINE(p3.x, p3.y, 0.0, 1.0, 0.0, 1.0);
                ADD_LINE(p0.x, p0.y, 0.0, 1.0, 0.0, 1.0);
                ADD_LINE(p0.x, p0.y, 0.0, 0.0, 0.0, 0.0);
            }

            if (draw_positions) {
                nvVector2 com = nvRigidBody_get_position(body);
                nvVector2 arm0 = nvVector2_rotate(NV_VECTOR2(0.5, 0.0), nvRigidBody_get_angle(body));
                nvVector2 arm1 = nvVector2_perpr(arm0);
                arm0 = nvVector2_add(arm0, com);
                arm1 = nvVector2_add(arm1, com);

                com = world_to_screen(&example, com);
                com = normalize_coords(&example, com);
                arm0 = world_to_screen(&example, arm0);
                arm0 = normalize_coords(&example, arm0);
                arm1 = world_to_screen(&example, arm1);
                arm1 = normalize_coords(&example, arm1);

                ADD_LINE(arm0.x, arm0.y, 0.0, 0.0, 0.0, 0.0);
                ADD_LINE(arm0.x, arm0.y, 1.0, 0.0, 0.0, 1.0);
                ADD_LINE(com.x, com.y, 1.0, 0.0, 0.0, 1.0);
                ADD_LINE(com.x, com.y, 0.0, 1.0, 0.0, 1.0);
                ADD_LINE(arm1.x, arm1.y, 0.0, 1.0, 0.0, 1.0);
                ADD_LINE(arm1.x, arm1.y, 0.0, 1.0, 0.0, 0.0);
            }

        }

        if (draw_constraints) {
            for (size_t i = 0; i < example.space->constraints->size; i++) {
                nvConstraint *cons = example.space->constraints->data[i];

                switch (cons->type) {
                    case nvConstraintType_DISTANCE:
                        nvVector2 a = nvDistanceConstraint_get_anchor_a(cons);
                        nvVector2 b = nvDistanceConstraint_get_anchor_b(cons);

                        if (cons->a) a = nvVector2_add(nvVector2_rotate(a, nvRigidBody_get_angle(cons->a)), nvRigidBody_get_position(cons->a));
                        if (cons->b) b = nvVector2_add(nvVector2_rotate(b, nvRigidBody_get_angle(cons->b)), nvRigidBody_get_position(cons->b));

                        a = world_to_screen(&example, a);
                        b = world_to_screen(&example, b);
                        a = normalize_coords(&example, a);
                        b = normalize_coords(&example, b);

                        ADD_LINE(a.x, a.y, 0.0, 0.0, 0.0, 0.0);
                        ADD_LINE(
                            a.x, a.y,
                            example.theme.distance_constraint.r,
                            example.theme.distance_constraint.g,
                            example.theme.distance_constraint.b,
                            1.0
                        );
                        ADD_LINE(
                            b.x, b.y,
                            example.theme.distance_constraint.r,
                            example.theme.distance_constraint.g,
                            example.theme.distance_constraint.b,
                            1.0
                        );
                        ADD_LINE(b.x, b.y, 0.0, 0.0, 0.0, 0.0);
                        break;

                    case nvConstraintType_HINGE:
                        nvHingeConstraint *hinge_cons = cons->def;

                        nvVector2 anchor = nvHingeConstraint_get_anchor(cons);
                        anchor = world_to_screen(&example, anchor);
                        anchor = normalize_coords(&example, anchor);

                        if (cons->a) {
                            //nvVector2 p = nvRigidBody_get_position(cons->a);
                            nvVector2 p = nvVector2_add(anchor, hinge_cons->xanchor_a);
                            p = world_to_screen(&example, p);
                            p = normalize_coords(&example, p);

                            ADD_LINE(p.x, p.y, 0.0, 0.0, 0.0, 0.0);
                            ADD_LINE(
                                p.x, p.y,
                                example.theme.hinge_constraint.r,
                                example.theme.hinge_constraint.g,
                                example.theme.hinge_constraint.b,
                                1.0
                            );
                            ADD_LINE(
                                anchor.x, anchor.y,
                                example.theme.hinge_constraint.r,
                                example.theme.hinge_constraint.g,
                                example.theme.hinge_constraint.b,
                                1.0
                            );
                            ADD_LINE(anchor.x, anchor.y, 0.0, 0.0, 0.0, 0.0);
                        }

                        if (cons->b) {
                            //nvVector2 p = nvRigidBody_get_position(cons->b);
                            nvVector2 p = nvVector2_add(anchor, hinge_cons->xanchor_b);
                            p = world_to_screen(&example, p);
                            p = normalize_coords(&example, p);

                            ADD_LINE(p.x, p.y, 0.0, 0.0, 0.0, 0.0);
                            ADD_LINE(
                                p.x, p.y,
                                example.theme.hinge_constraint.r,
                                example.theme.hinge_constraint.g,
                                example.theme.hinge_constraint.b,
                                1.0
                            );
                            ADD_LINE(
                                anchor.x, anchor.y,
                                example.theme.hinge_constraint.r,
                                example.theme.hinge_constraint.g,
                                example.theme.hinge_constraint.b,
                                1.0
                            );
                            ADD_LINE(anchor.x, anchor.y, 0.0, 0.0, 0.0, 0.0);
                        }
                }
            }
        }

        if (draw_contacts) {
            void *map_val;
            size_t l = 0;
            while (nvHashMap_iter(example.space->contacts, &l, &map_val)) {
                nvPersistentContactPair *pcp = map_val;
                for (size_t c = 0; c < pcp->contact_count; c++) {
                    nvContact contact = pcp->contacts[c];

                    FColor color;
                    if (contact.separation > 0) {
                        color = (FColor){1.0, 0.2, 0.0, 0.1};
                    }
                    else {
                        color = (FColor){1.0, 0.2, 0.0, 1.0};
                    }

                    nvVector2 pa = pcp->body_b->position;
                    nvVector2 p = nvVector2_add(pa, contact.anchor_b);

                    if (example.mouse.right && nvVector2_len(nvVector2_sub(p, example.before_zoom)) < 0.1) {
                        printf(
                            "Contact %llu\n"
                            " Shape A: %u\n"
                            " Shape B: %u\n"
                            " Body A:  %llu\n"
                            " Body B:  %llu\n"
                            " Depth:   %f\n",
                            (unsigned long long)contact.id,
                            (unsigned int)pcp->shape_a->id,
                            (unsigned int)pcp->shape_b->id,
                            (unsigned long long)pcp->body_a->id,
                            (unsigned long long)pcp->body_b->id,
                            contact.separation
                        );
                    }

                    nv_float w = 3.2 / example.zoom;
                    nv_float h = w * 2.5;
                    nv_float a = nv_atan2(pcp->normal.y, pcp->normal.x);
                    nvVector2 r0 = nvVector2_rotate(NV_VECTOR2(0.0, w), a);
                    nvVector2 r1 = nvVector2_rotate(NV_VECTOR2(h, 0.0), a);
                    nvVector2 r2 = nvVector2_rotate(NV_VECTOR2(0.0, -w), a);
                    nvVector2 r3 = nvVector2_rotate(NV_VECTOR2(-h, 0.0), a);
                    nvVector2 p0 = nvVector2_add(p, r0);
                    nvVector2 p1 = nvVector2_add(p, r1);
                    nvVector2 p2 = nvVector2_add(p, r2);
                    nvVector2 p3 = nvVector2_add(p, r3);

                    nvVector2 pn = nvVector2_add(p, nvVector2_mul(pcp->normal, -contact.separation));

                    p0 = world_to_screen(&example, p0);
                    p0 = normalize_coords(&example, p0);
                    p1 = world_to_screen(&example, p1);
                    p1 = normalize_coords(&example, p1);
                    p2 = world_to_screen(&example, p2);
                    p2 = normalize_coords(&example, p2);
                    p3 = world_to_screen(&example, p3);
                    p3 = normalize_coords(&example, p3);
                    pn = world_to_screen(&example, pn);
                    pn = normalize_coords(&example, pn);
                    p = world_to_screen(&example, p);
                    p = normalize_coords(&example, p);

                    ADD_LINE(p.x, p.y, 0.0, 0.0, 0.0, 0.0);
                    ADD_LINE(
                        p.x, p.y,
                        1.0,
                        0.7,
                        0.7,
                        color.a
                    );
                    ADD_LINE(
                        pn.x, pn.y,
                        1.0,
                        0.7,
                        0.7,
                        color.a
                    );
                    ADD_LINE(p.x, p.y, 0.0, 0.0, 0.0, 0.0);

                    ADD_TRIANGLE(
                        p0.x, p0.y,
                        p1.x, p1.y,
                        p2.x, p2.y,
                        color.r,
                        color.g,
                        color.b,
                        color.a
                    );

                    ADD_TRIANGLE(
                        p0.x, p0.y,
                        p2.x, p2.y,
                        p3.x, p3.y,
                        color.r,
                        color.g,
                        color.b,
                        color.a
                    );
                }
            }
        }

        glBindBuffer(GL_ARRAY_BUFFER, vbos[0]);
        glBufferSubData(GL_ARRAY_BUFFER, 0, (tri_vertices_index) * sizeof(float), tri_vertices);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindBuffer(GL_ARRAY_BUFFER, vbos[1]);
        glBufferSubData(GL_ARRAY_BUFFER, 0, (tri_colors_index) * sizeof(float), tri_colors);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindBuffer(GL_ARRAY_BUFFER, vbos[2]);
        glBufferSubData(GL_ARRAY_BUFFER, 0, (line_vertices_index) * sizeof(float), line_vertices);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindBuffer(GL_ARRAY_BUFFER, vbos[3]);
        glBufferSubData(GL_ARRAY_BUFFER, 0, (line_colors_index) * sizeof(float), line_colors);
        glBindBuffer(GL_ARRAY_BUFFER, 0);

        nvPrecisionTimer_stop(&render_timer);
        render_time += render_timer.elapsed,

        nvPrecisionTimer_start(&render_timer);
        ngl_clear(30.0f/255.0f, 27.0f/255.0f, 36.0f/255.0f, 1.0f);

        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glEnable(GL_BLEND);

        glUseProgram(program);
        ngl_vao_render(vaos[0], GL_TRIANGLES, vao0_count);
        ngl_vao_render(vaos[1], GL_LINE_STRIP, vao1_count);
        glUseProgram(0);

        nk_sdl_render(
            NK_ANTI_ALIASING_ON,
            NUKLEAR_MAX_VERTEX_MEMORY,
            NUKLEAR_MAX_ELEMENT_MEMORY
        );

        SDL_GL_SwapWindow(example.window);
        nvPrecisionTimer_stop(&render_timer);
        render_time += render_timer.elapsed,

        frame++;
    }

    nvSpace_free(example.space);

    nk_sdl_shutdown();

    free(tri_vertices);
    free(tri_colors);
    free(line_vertices);
    free(line_colors);

    glDeleteVertexArrays(2, vaos);
    glDeleteBuffers(4, vbos);
    glDeleteProgram(program);

    SDL_GL_DeleteContext(example.gl_ctx);
    SDL_DestroyWindow(example.window);
    SDL_Quit();

    return EXIT_SUCCESS;
}