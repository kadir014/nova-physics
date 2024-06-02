/*

  This file is a part of the Nova Physics Engine
  project and distributed under the MIT license.

  Copyright © Kadir Aksoy
  https://github.com/kadir014/nova-physics

*/

#include "common.h"
#include "ngl.h"
#include "clock.h"

#include "demos/stack.h"


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
    tri_vertices[tri_vertices_index]     = x0;             \
    tri_vertices[tri_vertices_index + 1] = y0;             \
    tri_vertices[tri_vertices_index + 2] = x1;             \
    tri_vertices[tri_vertices_index + 3] = y1;             \
    tri_vertices[tri_vertices_index + 4] = x2;             \
    tri_vertices[tri_vertices_index + 5] = y2;             \
    tri_vertices_index += 6;                               \
                                                           \
    for (size_t j = 0; j < 3; j++) {                       \
        tri_colors[tri_colors_index]     = r;              \
        tri_colors[tri_colors_index + 1] = g;              \
        tri_colors[tri_colors_index + 2] = b;              \
        tri_colors[tri_colors_index + 3] = a;              \
        tri_colors_index += 4;                             \
    }                                                      \
                                                           \
    vao0_count += 3;                                       \
}

#define ADD_LINE(x, y, r, g, b, a) {            \
    line_vertices[line_vertices_index]     = x; \
    line_vertices[line_vertices_index + 1] = y; \
    line_vertices_index += 2;                   \
                                                \
    line_colors[line_colors_index]     = r;     \
    line_colors[line_colors_index + 1] = g;     \
    line_colors[line_colors_index + 2] = b;     \
    line_colors[line_colors_index + 3] = a;     \
    line_colors_index += 4;                     \
                                                \
    vao1_count += 1;                            \
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

    struct nk_color accent = nk_rgb(124, 62, 247);
    struct nk_color text = nk_rgb(255, 255, 255);

    example->ui_ctx->style.window.fixed_background = nk_style_item_color(nk_rgba(17, 17, 20, 210));
    example->ui_ctx->style.window.border = 0;
    example->ui_ctx->style.window.header.active = nk_style_item_color(accent);
    example->ui_ctx->style.window.header.normal = nk_style_item_color(accent);
    example->ui_ctx->style.window.header.label_active = text;
    example->ui_ctx->style.window.header.label_normal = text;
    example->ui_ctx->style.window.header.label_padding = (struct nk_vec2){5.0, 2.0};
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

    struct nk_font_atlas *atlas;
    nk_sdl_font_stash_begin(&atlas);
    struct nk_font *font = nk_font_atlas_add_from_file(atlas, "assets/FiraCode-Medium.ttf", 16, 0);
    nk_sdl_font_stash_end();

    nk_style_set_font(example->ui_ctx, &font->handle);
}

// Transform (normalize) coordinate from screen space to opengl space [-1, 1]
static inline nvVector2 normalize_coords(ExampleContext *example, nvVector2 v) {
    return NV_VEC2(
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
    srand(time(NULL));

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
    int space_paused = 0;
    int show_bytes = 0;
    int draw_contacts = 0;

    nvPrecisionTimer render_timer;
    double render_time = 0.0;
    double old_render_time = 0.0;

    printf("Nova Physics %s\n", NV_VERSION_STRING);
    printf("nv_float size: %llu bytes\n", (unsigned long long)sizeof(nv_float));

    // Register all example demos
    ExampleEntry_register("Stack", Stack_setup, Stack_update);

    example_entries[0].setup(&example);

    while (is_running) {
        Clock_tick(clock, 60.0);

        old_render_time = render_time;
        render_time = 0.0;

        SDL_GetMouseState(&example.mouse.x, &example.mouse.y);
        example.before_zoom = screen_to_world(&example, NV_VEC2(example.mouse.x, example.mouse.y));

        SDL_Event event;
        nk_input_begin(example.ui_ctx);
        while (SDL_PollEvent(&event) != 0) {
            if (event.type == SDL_QUIT) {
                is_running = false;
            }

            else if (event.type == SDL_MOUSEBUTTONDOWN) {
                if (event.button.button == SDL_BUTTON_LEFT) {
                    example.mouse.left = true;
                }

                else if (event.button.button == SDL_BUTTON_MIDDLE) {
                    example.mouse.middle = true;
                    example.pan_start = NV_VEC2(example.mouse.x, example.mouse.y);
                }

                else if (event.button.button == SDL_BUTTON_RIGHT) {
                    example.mouse.right = true;
                }
            }

            else if (event.type == SDL_MOUSEBUTTONUP) {
                if (event.button.button == SDL_BUTTON_LEFT) {
                    example.mouse.left = false;
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

            nk_sdl_handle_event(&event);
        }
        nk_sdl_handle_grab();
        nk_input_end(example.ui_ctx);

        example.after_zoom = screen_to_world(&example, NV_VEC2(example.mouse.x, example.mouse.y));

        if (example.mouse.middle) {
            example.camera = nvVector2_sub(example.camera, nvVector2_div(nvVector2_sub(NV_VEC2(example.mouse.x, example.mouse.y), example.pan_start), example.zoom));
            example.pan_start = NV_VEC2(example.mouse.x, example.mouse.y);
        }
        example.camera = nvVector2_add(example.camera, nvVector2_sub(example.before_zoom, example.after_zoom));

        if (nk_begin(example.ui_ctx, "Simulation", nk_rect(0, 0, 250, example.window_height), NK_WINDOW_TITLE)) {
            if (nk_tree_push(example.ui_ctx, NK_TREE_TAB, "Drawing", NK_MINIMIZED)) {
                nk_layout_row_dynamic(example.ui_ctx, 16, 1);

                nk_checkbox_label(example.ui_ctx, "AABBs", &space_paused);
                nk_checkbox_label(example.ui_ctx, "Contacts", &draw_contacts);
                nk_checkbox_label(example.ui_ctx, "Constraints", &space_paused);
                nk_checkbox_label(example.ui_ctx, "Positions", &space_paused);
                nk_checkbox_label(example.ui_ctx, "Velocities", &space_paused);
                nk_checkbox_label(example.ui_ctx, "Normal impulses", &space_paused);
                nk_checkbox_label(example.ui_ctx, "Friction impulses", &space_paused);

                nk_tree_pop(example.ui_ctx);
            }

            nk_layout_row_static(example.ui_ctx, 30, 120, 1);
            nk_checkbox_label(example.ui_ctx, "Paused", &space_paused);
        }
        nk_end(example.ui_ctx);

        if (nk_begin(example.ui_ctx, "Profile", nk_rect(example.window_width - 250, 0, 250, 350), NK_WINDOW_TITLE | NK_WINDOW_MINIMIZABLE)) {
            char fmt_buffer[32];

            if (nk_tree_push(example.ui_ctx, NK_TREE_TAB, "Overview", NK_MAXIMIZED)) {
                nk_layout_row_dynamic(example.ui_ctx, 16, 1);

                sprintf(fmt_buffer, "FPS: %.1f", clock->fps);
                nk_label(example.ui_ctx, fmt_buffer, NK_TEXT_LEFT);

                sprintf(fmt_buffer, "Physics: %.3fms", example.space->profiler.step * 1000.0);
                nk_label(example.ui_ctx, fmt_buffer, NK_TEXT_LEFT);

                sprintf(fmt_buffer, "Render: %.3fms", old_render_time);
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

                sprintf(fmt_buffer, "Presolve: %.3fms", example.space->profiler.presolve_collisions * 1000.0);
                nk_label(example.ui_ctx, fmt_buffer, NK_TEXT_LEFT);

                sprintf(fmt_buffer, "Solve velocity: %.3fms", example.space->profiler.solve_velocities * 1000.0);
                nk_label(example.ui_ctx, fmt_buffer, NK_TEXT_LEFT);

                sprintf(fmt_buffer, "Solve position: %.3fms", example.space->profiler.solve_positions * 1000.0);
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

                size_t space_bytes =
                    sizeof(nvSpace) +
                    bodies_bytes +
                    example.space->broadphase_pairs->size * sizeof(nvBroadPhasePair) +
                    example.space->contacts->count * sizeof(nvPersistentContactPair);
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

                nk_tree_pop(example.ui_ctx);
            }
        }
        nk_end(example.ui_ctx);

        if (nk_begin(example.ui_ctx, example_entries[current_example].name, nk_rect(example.window_width - 250, example.window_height - 300, 250, 300), NK_WINDOW_TITLE | NK_WINDOW_MOVABLE | NK_WINDOW_MINIMIZABLE)) {
            example_entries[current_example].update(&example);
        }
        nk_end(example.ui_ctx);

        nvRigidBody *m = example.space->bodies->data[1];
        //nvRigidBody_set_position(m, example.before_zoom);
        //nvRigidBody_set_angle(m, nvRigidBody_get_angle(m) + 0.01);
        //nvRigidBody_reset_velocities(m);

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

            for (size_t k = 0; k < body->shapes->size; k++) {
                nvShape *shape = body->shapes->data[k];

                if (shape->type == nvShapeType_POLYGON) {
                    nvPolygon_transform(shape, (nvTransform){nvRigidBody_get_position(body), nvRigidBody_get_angle(body)});
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
                            255.0 / 255.0,
                            192.0 / 255.0,
                            74.0 / 255.0,
                            1.0
                        );
                    }

                    // The reason we add 2 more extra vertices per object is to
                    // basically a transparent line between objects. I currently
                    // have no idea how to remove the linked lines in GL_LINE_STRIP
                    // drawing mode but I believe this is efficient enough.

                    ADD_LINE(v0t.x, v0t.y, 0.0, 0.0, 0.0, 0.0);

                    for (size_t j = 0; j < polygon.num_vertices; j++) {
                        nvVector2 va = polygon.xvertices[j];
                        nvVector2 vat = world_to_screen(&example, va);
                        vat = normalize_coords(&example, vat);

                        ADD_LINE(vat.x, vat.y, 1.0, 1.0, 1.0, 1.0;)
                    }

                    ADD_LINE(v0t.x, v0t.y, 1.0, 1.0, 1.0, 1.0);
                    ADD_LINE(v0t.x, v0t.y, 0.0, 0.0, 0.0, 0.0);
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
                    if (contact.separation > 0) continue;

                    nvVector2 pa = nvRigidBody_get_position(pcp->body_a);
                    nvVector2 p = nvVector2_add(pa, contact.anchor_a);
                    nv_float w = 0.15;
                    nv_float h = w * 2.5;
                    nv_float a = nv_atan2(pcp->normal.y, pcp->normal.x);
                    nvVector2 r0 = nvVector2_rotate(NV_VEC2(0.0, w), a);
                    nvVector2 r1 = nvVector2_rotate(NV_VEC2(h, 0.0), a);
                    nvVector2 r2 = nvVector2_rotate(NV_VEC2(0.0, -w), a);
                    nvVector2 r3 = nvVector2_rotate(NV_VEC2(-h, 0.0), a);
                    nvVector2 p0 = nvVector2_add(p, r0);
                    nvVector2 p1 = nvVector2_add(p, r1);
                    nvVector2 p2 = nvVector2_add(p, r2);
                    nvVector2 p3 = nvVector2_add(p, r3);

                    p0 = world_to_screen(&example, p0);
                    p0 = normalize_coords(&example, p0);
                    p1 = world_to_screen(&example, p1);
                    p1 = normalize_coords(&example, p1);
                    p2 = world_to_screen(&example, p2);
                    p2 = normalize_coords(&example, p2);
                    p3 = world_to_screen(&example, p3);
                    p3 = normalize_coords(&example, p3);



                    ADD_TRIANGLE(
                        p0.x, p0.y,
                        p1.x, p1.y,
                        p2.x, p2.y,
                        1.0,
                        0.2,
                        0.0,
                        1.0
                    );

                    ADD_TRIANGLE(
                        p0.x, p0.y,
                        p2.x, p2.y,
                        p3.x, p3.y,
                        1.0,
                        0.2,
                        0.0,
                        1.0
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
        ngl_clear(30.0/255.0, 27.0/255.0, 36.0/255.0, 1.0);

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
        if (frame == 500) {
            nvRigidBody *body = example.space->bodies->data[40];
            printf("body100 %f %f %f %f", body->position.x, body->position.y, body->linear_velocity.x, body->linear_velocity.y);
        }
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