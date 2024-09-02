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
#include "demos/demo_stack.h"
#include "demos/demo_compound.h"
#include "demos/demo_pyramid.h"
#include "demos/demo_softbody.h"
#include "demos/demo_rocks.h"
#include "demos/demo_contact_event.h"

#include "demos/demo_distance_constraint.h"
#include "demos/demo_hinge_constraint.h"
#include "demos/demo_spline_constraint.h"

#include "demos/demo_bouncing.h"
#include "demos/demo_friction.h"
#include "demos/demo_damping.h"
#include "demos/demo_density.h"

/**
 * @file examples/main.c
 * 
 * This file is just the entry point for the opengl app and is pretty crowded.
 * If you are looking individual demos go to demos/ subfolder.
 */


#define NUKLEAR_MAX_VERTEX_MEMORY 100 * 1024
#define NUKLEAR_MAX_ELEMENT_MEMORY 25 * 1024

// 500,000 * 24 * 4(bytes) = ~45 MBs of pre allocated vertex memory
#define EXAMPLE_MAX_TRIANGlES 500000
#define EXAMPLE_MAX_TRI_VERTICES EXAMPLE_MAX_TRIANGlES * 6
#define EXAMPLE_MAX_TRI_COLORS EXAMPLE_MAX_TRIANGlES * 4 * 3
#define EXAMPLE_MAX_LINE_VERTICES EXAMPLE_MAX_TRIANGlES * 2
#define EXAMPLE_MAX_LINE_COLORS EXAMPLE_MAX_TRIANGlES * 4

#define CIRCLE_VERTICES 20

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
    example->ui_ctx->style.button.text_normal = text;
    example->ui_ctx->style.button.text_hover = text;

    example->ui_ctx->style.checkbox.text_active = text;
    example->ui_ctx->style.checkbox.text_normal = text;
    example->ui_ctx->style.checkbox.text_hover = text;
    example->ui_ctx->style.checkbox.padding = (struct nk_vec2){3.0, 3.0};
    example->ui_ctx->style.checkbox.active = nk_style_item_color(nk_rgb(37, 36, 38));
    example->ui_ctx->style.checkbox.hover = nk_style_item_color(nk_rgb(55, 53, 56));
    example->ui_ctx->style.checkbox.normal = nk_style_item_color(nk_rgb(37, 36, 38));
    example->ui_ctx->style.checkbox.cursor_normal = nk_style_item_color(accent);
    example->ui_ctx->style.checkbox.cursor_hover = nk_style_item_color(accent);

    example->ui_ctx->style.option.text_active = text;
    example->ui_ctx->style.option.text_normal = text;
    example->ui_ctx->style.option.text_hover = text;
    example->ui_ctx->style.option.active = nk_style_item_color(nk_rgb(37, 36, 38));
    example->ui_ctx->style.option.hover = nk_style_item_color(nk_rgb(55, 53, 56));
    example->ui_ctx->style.option.normal = nk_style_item_color(nk_rgb(37, 36, 38));
    example->ui_ctx->style.option.cursor_normal = nk_style_item_color(accent);
    example->ui_ctx->style.option.cursor_hover = nk_style_item_color(accent);

    example->ui_ctx->style.slider.cursor_normal = nk_style_item_color(accent);
    example->ui_ctx->style.slider.cursor_hover = nk_style_item_color(accent_light);
    example->ui_ctx->style.slider.cursor_active = nk_style_item_color(accent_light);
    example->ui_ctx->style.slider.bar_filled = accent;

    example->ui_ctx->style.tab.node_maximize_button.active = nk_style_item_color(nk_rgba(0, 0, 0, 0));
    example->ui_ctx->style.tab.node_maximize_button.normal = nk_style_item_color(nk_rgba(0, 0, 0, 0));
    example->ui_ctx->style.tab.node_maximize_button.hover = nk_style_item_color(nk_rgba(0, 0, 0, 0));
    example->ui_ctx->style.tab.node_minimize_button.active = nk_style_item_color(nk_rgba(0, 0, 0, 0));
    example->ui_ctx->style.tab.node_minimize_button.normal = nk_style_item_color(nk_rgba(0, 0, 0, 0));
    example->ui_ctx->style.tab.node_minimize_button.hover = nk_style_item_color(nk_rgba(0, 0, 0, 0));
    example->ui_ctx->style.tab.indent = 12.0;

    struct nk_font_atlas *atlas;
    nk_sdl_font_stash_begin(&atlas);
    struct nk_font *font;

    FILE *f = fopen("assets/FiraCode-Medium.ttf", "r");
    if (f) {
        fclose(f);
        font = nk_font_atlas_add_from_file(atlas, "assets/FiraCode-Medium.ttf", 16, NULL);
    }
    else {
        font = nk_font_atlas_add_default(atlas, 16, NULL);
        printf("Couldn't access 'assets/FiraCode-Medium.ttf'");
    }

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
    example.fullscreen = false;

    example.theme.dynamic_body = (FColor){1.0, 0.75, 0.29, 1.0};
    example.theme.static_body = (FColor){0.78, 0.44, 0.23, 1.0};
    example.theme.distance_constraint = (FColor){0.45, 0.87, 1.0, 1.0};
    example.theme.hinge_constraint = (FColor){0.623, 0.47, 0.98, 1.0},
    example.theme.spline_constraint = (FColor){0.76, 0.949, 0.247, 1.0};
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

    SDL_Surface *window_icon = SDL_LoadBMP("assets/novaicon.bmp");
    SDL_SetWindowIcon(example.window, window_icon);
    SDL_FreeSurface(window_icon);

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
    float *tri_vertices = NV_MALLOC(tri_vertices_size);
    size_t tri_vertices_index = 0;

    size_t tri_colors_size = sizeof(float) * EXAMPLE_MAX_TRI_COLORS;
    float *tri_colors = NV_MALLOC(tri_colors_size);
    size_t tri_colors_index = 0;

    size_t line_vertices_size = sizeof(float) * EXAMPLE_MAX_LINE_VERTICES;
    float *line_vertices = NV_MALLOC(line_vertices_size);
    size_t line_vertices_index = 0;

    size_t line_colors_size = sizeof(float) * EXAMPLE_MAX_LINE_COLORS;
    float *line_colors = NV_MALLOC(line_colors_size);
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
    nvSpace_set_broadphase(example.space, nvBroadPhaseAlg_BVH);

    int space_paused = 0;
    nv_bool space_one_step = false;
    nv_float space_dt = 1.0 / 60.0;
    nv_float space_hertz = 60.0;

    // UI settings
    int draw_ui = 1;
    int show_bytes = 0;
    int draw_shapes = 1;
    int draw_contacts = 0;
    int draw_aabbs = 0;
    int draw_constraints = 1;
    int draw_positions = 0;
    int draw_broadphase = 0;
    int draw_normal_impulses = 0;
    int draw_friction_impulses = 0;

    nv_bool raycast = false;

    nvPrecisionTimer render_timer;
    double render_time = 0.0;
    double old_render_time = 0.0;

    int gl_major;
    int gl_minor;
    int gl_profile_mask;
    SDL_GL_GetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, &gl_major);
    SDL_GL_GetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, &gl_minor);
    SDL_GL_GetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, &gl_profile_mask);

    char *gl_profile_mask_str;
    switch (gl_profile_mask) {
        case (SDL_GL_CONTEXT_PROFILE_CORE):
            gl_profile_mask_str = "Core";
            break;

        case (SDL_GL_CONTEXT_PROFILE_COMPATIBILITY):
            gl_profile_mask_str = "Compatibility";
            break;

        case (SDL_GL_CONTEXT_PROFILE_ES):
            gl_profile_mask_str = "ES";
            break;
    }

    printf("Nova Physics %s\n", NV_VERSION_STRING);
    printf("SDL          %d.%d.%d\n", SDL_MAJOR_VERSION, SDL_MINOR_VERSION, SDL_PATCHLEVEL);
    printf("OpenGL       %d.%d %s\n", gl_major, gl_minor, gl_profile_mask_str);
    printf("\n");
    printf("nv_float size: %llu bytes\n", (unsigned long long)sizeof(nv_float));
    printf("Vendor: %s\n", glGetString(GL_VENDOR));
    printf("Renderer: %s\n", glGetString(GL_RENDERER));

    // Register all example demos

    // General demos
    ExampleEntry_register("Stack", Stack_setup, Stack_update);
    ExampleEntry_register("Compound", Compound_setup, Compound_update);
    ExampleEntry_register("Pyramid", Pyramid_setup, Pyramid_update);
    ExampleEntry_register("Rocks", Rocks_setup, Rocks_update);
    ExampleEntry_register("SoftBody", SoftBody_setup, SoftBody_update);
    ExampleEntry_register("Contact Events", ContactEvent_setup, ContactEvent_update);

    // Constraint demos
    ExampleEntry_register("Distance", DistanceConstraint_setup, DistanceConstraint_update);
    ExampleEntry_register("Hinge", HingeConstraint_setup, HingeConstraint_update);
    ExampleEntry_register("Spline", SplineConstraint_setup, SplineConstraint_update);

    // Material demos
    ExampleEntry_register("Bouncing", Bouncing_setup, Bouncing_update);
    ExampleEntry_register("Friction", Friction_setup, Friction_update);
    ExampleEntry_register("Density", Density_setup, Density_update);
    ExampleEntry_register("Damping", Damping_setup, Damping_update);

    current_example = 2;

    // TODO: OH MY GOD PLEASE FIND A MORE ELEGANT SOLUTION
    int row_i = 0;
    int row0[] = {row_i++, row_i++, row_i++, row_i++, row_i++, row_i++};
    int row1[] = {row_i++, row_i++, row_i++};
    int row2[] = {row_i++, row_i++, row_i++, row_i++};
    #define CATEGORIES 3
    int *categories[CATEGORIES];
    size_t row_sizes[CATEGORIES] = {sizeof(row0)/sizeof(int), sizeof(row1)/sizeof(int), sizeof(row2)/sizeof(int)};
    size_t demo_rows = CATEGORIES;
    categories[0] = row0;
    categories[1] = row1;
    categories[2] = row2;
    char *category_names[CATEGORIES] = {"General", "Constraints", "Material"};

    example_entries[current_example].setup(&example);

    nvConstraint *mouse_cons = NULL;
    nvDistanceConstraintInitializer mouse_cons_init = nvDistanceConstraintInitializer_default;

    while (is_running) {
        Clock_tick(clock, 60.0);

        old_render_time = render_time;
        render_time = 0.0;

        SDL_GetMouseState(&example.mouse.x, &example.mouse.y);
        example.before_zoom = screen_to_world(&example, NV_VECTOR2((nv_float)example.mouse.x, (nv_float)example.mouse.y));

        SDL_Event event;
        nk_input_begin(example.ui_ctx);
        while (SDL_PollEvent(&event) != 0) {
            if (event.type == SDL_QUIT) {
                is_running = false;
            }

            else if (event.type == SDL_MOUSEBUTTONDOWN) {
                if (event.button.button == SDL_BUTTON_LEFT) {
                    example.mouse.left = true;

                    nvRigidBody *selected = NULL;
                    nvRigidBody *body;
                    size_t body_iter = 0;
                    while (nvSpace_iter_bodies(example.space, &body, &body_iter)) {
                        if (body->type == nvRigidBodyType_STATIC) continue;

                        nvTransform xform = (nvTransform){body->origin, body->angle};
                        nvAABB aabb = nvRigidBody_get_aabb(body);

                        if (nv_collide_aabb_x_point(aabb, example.before_zoom)) {

                            for (size_t j = 0; j < body->shapes->size; j++) {
                                nvShape *shape = body->shapes->data[j];
                                nvAABB saabb = nvShape_get_aabb(shape, xform);

                                if (nv_collide_aabb_x_point(saabb, example.before_zoom)) {
                                    if (shape->type == nvShapeType_CIRCLE) {
                                        if (nv_collide_circle_x_point(shape, xform, example.before_zoom)) {
                                            selected = body;
                                            break;
                                        }
                                    }
                                    else if (shape->type == nvShapeType_POLYGON) {
                                        if (nv_collide_polygon_x_point(shape, xform, example.before_zoom)) {
                                            selected = body;
                                            break;
                                        }
                                    }
                                }
                            }

                            if (selected) break;
                        }
                    }

                    if (selected) {
                        nvVector2 anchor = nvVector2_rotate(nvVector2_sub(example.before_zoom, nvRigidBody_get_position(selected)), -nvRigidBody_get_angle(selected));
                        mouse_cons_init.a = selected;
                        mouse_cons_init.b = NULL;
                        mouse_cons_init.length = 0.1;
                        mouse_cons_init.anchor_a = nvVector2_add(anchor, NV_VECTOR2(0.0, 0.01));
                        mouse_cons_init.anchor_b = example.before_zoom;
                        mouse_cons_init.spring = true;
                        mouse_cons_init.hertz = 1.0;
                        mouse_cons_init.damping = 0.5;
                        mouse_cons = nvDistanceConstraint_new(mouse_cons_init);
                        nvSpace_add_constraint(example.space, mouse_cons);
                    }
                }

                else if (event.button.button == SDL_BUTTON_MIDDLE) {
                    example.mouse.middle = true;
                    example.pan_start = NV_VECTOR2((nv_float)example.mouse.x, (nv_float)example.mouse.y);
                }

                else if (event.button.button == SDL_BUTTON_RIGHT) {
                    example.mouse.right = true;
                }
            }

            else if (event.type == SDL_MOUSEBUTTONUP) {
                if (event.button.button == SDL_BUTTON_LEFT) {
                    example.mouse.left = false;

                    if (mouse_cons) {
                        nvSpace_remove_constraint(example.space, mouse_cons);
                        nvConstraint_free(mouse_cons);
                        mouse_cons = NULL;
                    }
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

                else if (event.key.keysym.scancode == SDL_SCANCODE_O) {
                    space_one_step = true;
                }

                else if (event.key.keysym.scancode == SDL_SCANCODE_DELETE) {
                    nvRigidBody *selected = NULL;
                    nvRigidBody *body;
                    size_t body_iter = 0;
                    while (nvSpace_iter_bodies(example.space, &body, &body_iter)) {
                        nvTransform xform = (nvTransform){body->origin, body->angle};
                        nvAABB aabb = nvRigidBody_get_aabb(body);

                        if (nv_collide_aabb_x_point(aabb, example.before_zoom)) {

                            for (size_t j = 0; j < body->shapes->size; j++) {
                                nvShape *shape = body->shapes->data[j];
                                nvAABB saabb = nvShape_get_aabb(shape, xform);

                                if (nv_collide_aabb_x_point(saabb, example.before_zoom)) {
                                    if (shape->type == nvShapeType_CIRCLE) {
                                        if (nv_collide_circle_x_point(shape, xform, example.before_zoom)) {
                                            selected = body;
                                            break;
                                        }
                                    }
                                    else if (shape->type == nvShapeType_POLYGON) {
                                        if (nv_collide_polygon_x_point(shape, xform, example.before_zoom)) {
                                            selected = body;
                                            break;
                                        }
                                    }
                                }
                            }

                            if (selected) break;
                        }
                    }

                    if (selected) {
                        nvSpace_remove_rigidbody(example.space, selected);
                    }
                }

                else if (event.key.keysym.scancode == SDL_SCANCODE_RETURN && event.key.keysym.mod == KMOD_LALT) {
                    example.fullscreen = !example.fullscreen;
                    if (example.fullscreen) {
                        SDL_SetWindowFullscreen(example.window, SDL_WINDOW_FULLSCREEN_DESKTOP);
                    }
                    else {
                        SDL_SetWindowFullscreen(example.window, 0);
                    }
                }

                else if (event.key.keysym.scancode == SDL_SCANCODE_F1) {
                    nvRigidBody *body;
                    size_t body_iter = 0;
                    while (nvSpace_iter_bodies(example.space, &body, &body_iter)) {
                        if (body->type == nvRigidBodyType_STATIC) continue;

                        nvVector2 pos = nvRigidBody_get_position(body);
                        pos.x += frand(-5.0, 5.0);
                        pos.y += frand(-5.0, 5.0);
                        nvRigidBody_set_position(body, pos);
                    }
                }

                else if (event.key.keysym.scancode == SDL_SCANCODE_F2) {
                    nvRigidBodyInitializer f2init = nvRigidBodyInitializer_default;
                    f2init.type = nvRigidBodyType_DYNAMIC;
                    f2init.position = example.before_zoom;
                    nvRigidBody *f2box = nvRigidBody_new(f2init);

                    nvShape *f2box_shape = nvRectShape_new(1.0, 1.0, nvVector2_zero);
                    nvRigidBody_add_shape(f2box, f2box_shape);

                    nvSpace_add_rigidbody(example.space, f2box);
                }

                else if (event.key.keysym.scancode == SDL_SCANCODE_F3) {
                    nvRigidBodyInitializer f2init = nvRigidBodyInitializer_default;
                    f2init.type = nvRigidBodyType_DYNAMIC;
                    f2init.position = example.before_zoom;
                    nvRigidBody *f2box = nvRigidBody_new(f2init);

                    nvShape *f2box_shape = nvCircleShape_new(nvVector2_zero, 1.0);
                    nvRigidBody_add_shape(f2box, f2box_shape);

                    nvSpace_add_rigidbody(example.space, f2box);
                }

                else if (event.key.keysym.scancode == SDL_SCANCODE_F4) {
                    create_circle_softbody(&example, example.before_zoom, 12, 2.5, 0.6);
                }

                else if (event.key.keysym.scancode == SDL_SCANCODE_F5) {
                    raycast = !raycast;
                }
            }

            nk_sdl_handle_event(&event);
        }
        nk_sdl_handle_grab();
        nk_input_end(example.ui_ctx);

        example.after_zoom = screen_to_world(&example, NV_VECTOR2((nv_float)example.mouse.x, (nv_float)example.mouse.y));

        if (example.mouse.middle) {
            example.camera = nvVector2_sub(example.camera, nvVector2_div(nvVector2_sub(NV_VECTOR2((nv_float)example.mouse.x, (nv_float)example.mouse.y), example.pan_start), example.zoom));
            example.pan_start = NV_VECTOR2((nv_float)example.mouse.x, (nv_float)example.mouse.y);
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

                    nk_slider_int(example.ui_ctx, 0, (int *)&settings->velocity_iterations, 30, 1);
                    
                    sprintf(display_buf, "%u", settings->velocity_iterations);
                    nk_label(example.ui_ctx, display_buf, NK_TEXT_LEFT);
                }
                {
                    nk_layout_row(example.ui_ctx, NK_DYNAMIC, 16, 3, ratio);

                    nk_label(example.ui_ctx, "Hertz", NK_TEXT_LEFT);

                    if (nk_slider_float(example.ui_ctx, 7.5f, (float *)&space_hertz, 180.0f, 0.005f))
                        space_dt = 1.0 / space_hertz;
                    
                    sprintf(display_buf, "%f", space_hertz);
                    nk_label(example.ui_ctx, display_buf, NK_TEXT_LEFT);
                }
                {
                    nk_layout_row(example.ui_ctx, NK_DYNAMIC, 16, 3, ratio);

                    nk_label(example.ui_ctx, "Substeps", NK_TEXT_LEFT);

                    nk_slider_int(example.ui_ctx, 1, (int *)&settings->substeps, 5, 1);
                    
                    sprintf(display_buf, "%u", settings->substeps);
                    nk_label(example.ui_ctx, display_buf, NK_TEXT_LEFT);
                }

                nk_layout_row_dynamic(example.ui_ctx, 20, 1);
                nk_checkbox_label(example.ui_ctx, "Warmstarting", &settings->warmstarting);

                nk_layout_row_static(example.ui_ctx, 25, 120, 1);
                nk_checkbox_label(example.ui_ctx, "Paused", &space_paused);

                if (nk_tree_push(example.ui_ctx, NK_TREE_NODE, "Broadphase", NK_MAXIMIZED)) {
                    nk_layout_row_dynamic(example.ui_ctx, 16, 1);
                    if (nk_option_label(example.ui_ctx, "Bruteforce", nvSpace_get_broadphase(example.space) == nvBroadPhaseAlg_BRUTE_FORCE))
                        nvSpace_set_broadphase(example.space, nvBroadPhaseAlg_BRUTE_FORCE);
                    if (nk_option_label(example.ui_ctx, "BVH", nvSpace_get_broadphase(example.space) == nvBroadPhaseAlg_BVH))
                        nvSpace_set_broadphase(example.space, nvBroadPhaseAlg_BVH);

                    nk_tree_pop(example.ui_ctx);
                }

                nk_layout_row_dynamic(example.ui_ctx, 8, 1);
                nk_spacer(example.ui_ctx);

                nk_tree_pop(example.ui_ctx);
            }

            if (nk_tree_push(example.ui_ctx, NK_TREE_TAB, "Drawing", NK_MINIMIZED)) {
                nk_layout_row_dynamic(example.ui_ctx, 16, 1);

                nk_checkbox_label(example.ui_ctx, "Shapes", &draw_shapes);
                nk_checkbox_label(example.ui_ctx, "AABBs", &draw_aabbs);
                nk_checkbox_label(example.ui_ctx, "Broadphase", &draw_broadphase);
                nk_checkbox_label(example.ui_ctx, "Contacts", &draw_contacts);
                nk_checkbox_label(example.ui_ctx, "Constraints", &draw_constraints);
                nk_checkbox_label(example.ui_ctx, "Positions", &draw_positions);
                nk_checkbox_label(example.ui_ctx, "Velocities", &space_paused);
                nk_checkbox_label(example.ui_ctx, "Normal impulses", &draw_normal_impulses);
                nk_checkbox_label(example.ui_ctx, "Friction impulses", &draw_friction_impulses);

                nk_layout_row_dynamic(example.ui_ctx, 8, 1);
                nk_spacer(example.ui_ctx);

                nk_tree_pop(example.ui_ctx);
            }

            if (nk_tree_push(example.ui_ctx, NK_TREE_TAB, "Demos", NK_MINIMIZED)) {
                for (size_t row = 0; row < demo_rows; row++) {
                    char *title = category_names[row];

                    if (nk_tree_push_id(example.ui_ctx, NK_TREE_TAB, title, NK_MINIMIZED, row)) {
                        size_t row_size = row_sizes[row];
                        for (size_t demo_i = 0; demo_i < row_size; demo_i++) {
                            size_t demo = categories[row][demo_i];
                            nk_layout_row_dynamic(example.ui_ctx, 22, 1);
                            if (nk_button_label(example.ui_ctx, example_entries[demo].name)) {
                                current_example = demo;
                                nvSpace_clear(example.space, true);
                                nvSpace_set_gravity(example.space, NV_VECTOR2(0.0, 9.81));
                                mouse_cons = NULL;
                                NV_FREE(example.space->listener);
                                example.space->listener = NULL;
                                example_entries[current_example].setup(&example);
                            }
                        }

                        nk_layout_row_dynamic(example.ui_ctx, 8, 1);
                        nk_spacer(example.ui_ctx);

                        nk_tree_pop(example.ui_ctx);
                    }
                }

                nk_tree_pop(example.ui_ctx);
            }

            if (nk_tree_push(example.ui_ctx, NK_TREE_TAB, "Controls", NK_MINIMIZED)) {
                nk_layout_row_dynamic(example.ui_ctx, 16, 1);
  
                nk_label(example.ui_ctx, "[LMB] to drag objects.", NK_TEXT_LEFT);
                nk_label(example.ui_ctx, "[MWHEEL] to move & zoom camera.", NK_TEXT_LEFT);
                nk_label(example.ui_ctx, "[ESC] to exit.", NK_TEXT_LEFT);
                nk_label(example.ui_ctx, "[P] to pause simulation.", NK_TEXT_LEFT);
                nk_label(example.ui_ctx, "[U] to toggle UI.", NK_TEXT_LEFT);
                nk_label(example.ui_ctx, "[ALT+ENTER] to toggle fullscreen.", NK_TEXT_LEFT);
                nk_label(example.ui_ctx, "[DEL] to remove bodies.", NK_TEXT_LEFT);
                nk_label(example.ui_ctx, "[F1] to teleport everything.", NK_TEXT_LEFT);
                nk_label(example.ui_ctx, "[F2] to create box.", NK_TEXT_LEFT);
                nk_label(example.ui_ctx, "[F3] to create ball.", NK_TEXT_LEFT);
                nk_label(example.ui_ctx, "[F4] to create soft-body.", NK_TEXT_LEFT);
                nk_label(example.ui_ctx, "[F5] to cast ray.", NK_TEXT_LEFT);

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

                sprintf(fmt_buffer, "BPh finalize: %.3fms", example.space->profiler.broadphase_finalize * 1000.0);
                nk_label(example.ui_ctx, fmt_buffer, NK_TEXT_LEFT);

                sprintf(fmt_buffer, "BVH build: %.3fms", example.space->profiler.bvh_build * 1000.0);
                nk_label(example.ui_ctx, fmt_buffer, NK_TEXT_LEFT);

                sprintf(fmt_buffer, "BVH traverse: %.3fms", example.space->profiler.bvh_traverse * 1000.0);
                nk_label(example.ui_ctx, fmt_buffer, NK_TEXT_LEFT);

                sprintf(fmt_buffer, "BVH NV_FREE: %.3fms", example.space->profiler.bvh_free * 1000.0);
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

                size_t num_shapes = 0;
                size_t bodies_bytes = 0;

                nvRigidBody *body;
                size_t body_iter = 0;
                while (nvSpace_iter_bodies(example.space, &body, &body_iter)) {
                    bodies_bytes += sizeof(nvArray); // Shape array
                    num_shapes += body->shapes->size;
                }

                size_t shapes_bytes = num_shapes * sizeof(nvShape);
                double shapes_s = (double)(shapes_bytes) / unit_size;

                size_t num_bodies = example.space->bodies->size;
                bodies_bytes += num_bodies * sizeof(nvRigidBody);
                double bodies_s = (double)(bodies_bytes) / unit_size;

                size_t num_cons = example.space->constraints->size;
                size_t cons_bytes = num_cons * sizeof(nvConstraint);
                double cons_s = (double)(cons_bytes) / unit_size;

                size_t num_contacts = example.space->contacts->count;
                size_t contacts_bytes = num_contacts * sizeof(nvPersistentContactPair);
                double contacts_s = (double)(contacts_bytes) / unit_size;

                size_t pairs_bytes = example.space->broadphase_pairs->pool_size;
                double pairs_s = (double)(pairs_bytes) / unit_size;

                size_t space_bytes =
                    sizeof(nvSpace) +
                    bodies_bytes + sizeof(nvArray) +
                    cons_bytes + sizeof(nvArray) +
                    pairs_bytes + sizeof(nvMemoryPool) +
                    contacts_bytes + sizeof(nvHashMap);
                double space_s = (double)space_bytes / unit_size;

                if (!show_bytes && space_s > 1024.0) {
                    space_s /= 1024.0;
                    unit = "MB";
                }
                sprintf(fmt_buffer, "Space: %.1f %s", space_s, unit);
                nk_label(example.ui_ctx, fmt_buffer, NK_TEXT_LEFT);
                unit = "KB";

                if (!show_bytes && bodies_s > 1024.0) {
                    bodies_s /= 1024.0;
                    unit = "MB";
                }
                sprintf(fmt_buffer, "Bodies: %llu (%.1f %s)", (unsigned long long)num_bodies, bodies_s, unit);
                nk_label(example.ui_ctx, fmt_buffer, NK_TEXT_LEFT);
                unit = "KB";

                if (!show_bytes && shapes_s > 1024.0) {
                    shapes_s /= 1024.0;
                    unit = "MB";
                }
                sprintf(fmt_buffer, "Shapes: %llu (%.1f %s)", (unsigned long long)num_shapes, shapes_s, unit);
                nk_label(example.ui_ctx, fmt_buffer, NK_TEXT_LEFT);
                unit = "KB";

                if (!show_bytes && cons_s > 1024.0) {
                    cons_s /= 1024.0;
                    unit = "MB";
                }
                sprintf(fmt_buffer, "Constraints: %llu (%.1f %s)", (unsigned long long)num_cons, cons_s, unit);
                nk_label(example.ui_ctx, fmt_buffer, NK_TEXT_LEFT);
                unit = "KB";

                if (!show_bytes && pairs_s > 1024.0) {
                    pairs_s /= 1024.0;
                    unit = "MB";
                }
                unsigned long long pairs_n = example.space->broadphase_pairs->pool_size / example.space->broadphase_pairs->chunk_size;
                sprintf(fmt_buffer, "BPh: %llu/%llu (%.1f %s)", (unsigned long long)example.space->broadphase_pairs->current_size, pairs_n, pairs_s, unit);
                nk_label(example.ui_ctx, fmt_buffer, NK_TEXT_LEFT);
                unit = "KB";

                if (!show_bytes && contacts_s > 1024.0) {
                    contacts_s /= 1024.0;
                    unit = "MB";
                }
                sprintf(fmt_buffer, "Contacts: %llu (%.1f %s)", (unsigned long long)num_contacts, contacts_s, unit);
                nk_label(example.ui_ctx, fmt_buffer, NK_TEXT_LEFT);
                unit = "KB";

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

        if (!space_paused || (space_paused && space_one_step)) {
            nvSpace_step(example.space, space_dt);
            space_one_step = false;
        }

        nvPrecisionTimer_start(&render_timer);

        tri_vertices_index = 0;
        tri_colors_index = 0;
        vao0_count = 0;
        line_vertices_index = 0;
        line_colors_index = 0;
        vao1_count = 0;

        if (raycast) {
            nvRayCastResult results[256];
            size_t num_results;
            nvSpace_cast_ray(
                example.space,
                NV_VECTOR2(64.0, 36.0),
                example.before_zoom,
                results,
                &num_results,
                256
            );

            nvVector2 p0 = NV_VECTOR2(64.0, 36.0);
            nvVector2 p1 = example.before_zoom;
            p0 = world_to_screen(&example, p0);
            p0 = normalize_coords(&example, p0);
            p1 = world_to_screen(&example, p1);
            p1 = normalize_coords(&example, p1);

            ADD_LINE(p0.x, p0.y, 0.0, 0.0, 0.0, 0.0);
            ADD_LINE(p0.x, p0.y, 0.0, 1.0, 0.0, 1.0);
            ADD_LINE(p1.x, p1.y, 0.0, 1.0, 0.0, 1.0);
            ADD_LINE(p1.x, p1.y, 0.0, 0.0, 0.0, 0.0);

            for (size_t i = 0; i < num_results; i++) {
                nvRayCastResult result = results[i];

                nvVector2 p0 = result.position;
                nvVector2 p1 = nvVector2_add(p0, result.normal);
                p0 = world_to_screen(&example, p0);
                p0 = normalize_coords(&example, p0);
                p1 = world_to_screen(&example, p1);
                p1 = normalize_coords(&example, p1);

                ADD_LINE(p0.x, p0.y, 0.0, 0.0, 0.0, 0.0);
                ADD_LINE(p0.x, p0.y, 1.0, 0.0, 0.0, 1.0);
                ADD_LINE(p1.x, p1.y, 1.0, 0.0, 0.0, 1.0);
                ADD_LINE(p1.x, p1.y, 0.0, 0.0, 0.0, 0.0);
            }
        }

        if (draw_shapes) {
            nvRigidBody *body;
            size_t body_iter = 0;
            while (nvSpace_iter_bodies(example.space, &body, &body_iter)) {
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

                nvShape *shape;
                size_t shape_iter = 0;
                while (nvRigidBody_iter_shapes(body, &shape, &shape_iter)) {

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
                    else if (shape->type == nvShapeType_CIRCLE) {
                        nvVector2 c = nvVector2_add(nvVector2_rotate(shape->circle.center, body->angle), body->origin);

                        nvVector2 vertices[CIRCLE_VERTICES];
                        nvVector2 arm = NV_VECTOR2(shape->circle.radius, 0.0);

                        for (size_t i = 0; i < CIRCLE_VERTICES; i++) {
                            vertices[i] = nvVector2_add(c, arm);
                            arm = nvVector2_rotate(arm, 2.0 * NV_PI / (nv_float)CIRCLE_VERTICES);
                        }

                        nvVector2 v0 = vertices[0];
                        nvVector2 v0t = world_to_screen(&example, v0);
                        v0t = normalize_coords(&example, v0t);

                        for (size_t j = 0; j < CIRCLE_VERTICES - 2; j++) {
                            nvVector2 v1 = vertices[j + 1];
                            nvVector2 v2 = vertices[j + 2];

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

                        for (size_t j = 0; j < CIRCLE_VERTICES; j++) {
                            nvVector2 va = vertices[j];
                            nvVector2 vat = world_to_screen(&example, va);
                            vat = normalize_coords(&example, vat);

                            ADD_LINE(vat.x, vat.y, r, g, b, 1.0;)
                        }

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
                    nvAABB aabb = nvRigidBody_get_aabb(body);
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
        }

        if (draw_constraints) {
            nvConstraint *cons;
            size_t cons_iter = 0;
            while (nvSpace_iter_constraints(example.space, &cons, &cons_iter)) {
                switch (cons->type) {
                    case nvConstraintType_DISTANCE: {
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
                    }
                    case nvConstraintType_HINGE: {
                        nvHingeConstraint *hinge_cons = cons->def;

                        nvVector2 pa, pb;
                        nvVector2 p;

                        if (cons->a) {
                            p = nvRigidBody_get_position(cons->a);
                            pa = nvVector2_add(p, hinge_cons->xanchor_a);
                        }
                        else {
                            p = hinge_cons->anchor;
                            pa = hinge_cons->anchor;
                        }
                        p = world_to_screen(&example, p);
                        p = normalize_coords(&example, p);
                        pa = world_to_screen(&example, pa);
                        pa = normalize_coords(&example, pa);

                        ADD_LINE(p.x, p.y, 0.0, 0.0, 0.0, 0.0);
                        ADD_LINE(
                            p.x, p.y,
                            example.theme.hinge_constraint.r,
                            example.theme.hinge_constraint.g,
                            example.theme.hinge_constraint.b,
                            1.0
                        );
                        ADD_LINE(
                            pa.x, pa.y,
                            example.theme.hinge_constraint.r,
                            example.theme.hinge_constraint.g,
                            example.theme.hinge_constraint.b,
                            1.0
                        );
                        ADD_LINE(pa.x, pa.y, 0.0, 0.0, 0.0, 0.0);

                        if (cons->b) {
                            p = nvRigidBody_get_position(cons->b);
                            pb = nvVector2_add(p, hinge_cons->xanchor_b);
                        }
                        else {
                            p = hinge_cons->anchor;
                            pb = hinge_cons->anchor;
                        }
                        p = world_to_screen(&example, p);
                        p = normalize_coords(&example, p);
                        pb = world_to_screen(&example, pb);
                        pb = normalize_coords(&example, pb);

                        ADD_LINE(p.x, p.y, 0.0, 0.0, 0.0, 0.0);
                        ADD_LINE(
                            p.x, p.y,
                            example.theme.hinge_constraint.r,
                            example.theme.hinge_constraint.g,
                            example.theme.hinge_constraint.b,
                            1.0
                        );
                        ADD_LINE(
                            pb.x, pb.y,
                            example.theme.hinge_constraint.r,
                            example.theme.hinge_constraint.g,
                            example.theme.hinge_constraint.b,
                            1.0
                        );
                        ADD_LINE(pb.x, pb.y, 0.0, 0.0, 0.0, 0.0);

                        nvVector2 r = nvVector2_mul(nvVector2_add(pa, pb), 0.5);
                        float ar = (float)example.window_height / (float)example.window_width;
                        
                        nvVector2 radius = NV_VECTOR2(0.025, 0.0);
                        ADD_LINE(
                            r.x + radius.x * ar,
                            r.y + radius.y,
                            0.0,
                            0.0,
                            0.0,
                            0.0
                        );
                        for (size_t i = 0; i < 13; i++) {
                            radius = nvVector2_rotate(radius, 2.0 * NV_PI / 12.0);
                            ADD_LINE(
                                r.x + radius.x * ar,
                                r.y + radius.y,
                                example.theme.hinge_constraint.r * 2.0,
                                example.theme.hinge_constraint.g * 2.0,
                                example.theme.hinge_constraint.b * 2.0,
                                1.0
                            );
                        }
                        ADD_LINE(
                            r.x + radius.x * ar,
                            r.y + radius.y,
                            0.0,
                            0.0,
                            0.0,
                            0.0
                        );

                        if (hinge_cons->enable_limits) {
                            nvVector2 upper = nvVector2_rotate(NV_VECTOR2(0.025 * 1.5, 0.0), -hinge_cons->upper_limit + NV_PI);
                            nvVector2 lower = nvVector2_rotate(NV_VECTOR2(0.025 * 1.5, 0.0), -hinge_cons->lower_limit + NV_PI);
                            upper.x *= ar;
                            lower.x *= ar;
                            upper = nvVector2_add(r, upper);
                            lower = nvVector2_add(r, lower);

                            ADD_LINE(upper.x, upper.y, 0.0, 0.0, 0.0, 0.0);
                            ADD_LINE(upper.x, upper.y, 1.0, 0.376, 0.25, 1.0);
                            ADD_LINE(r.x, r.y, 1.0, 0.376, 0.25, 1.0);
                            ADD_LINE(r.x, r.y, 0.325, 0.615, 0.988, 1.0);
                            ADD_LINE(lower.x, lower.y, 0.325, 0.615, 0.988, 1.0);
                            ADD_LINE(lower.x, lower.y, 0.0, 0.0, 0.0, 0.0);
                        }

                        break;
                    }
                    case nvConstraintType_SPLINE: {
                        nvSplineConstraint *spline_cons = cons->def;

                        nvVector2 va = spline_cons->controls[0];
                        nvVector2 vb = spline_cons->controls[1];

                        va = world_to_screen(&example, va);
                        va = normalize_coords(&example, va);
                        vb = world_to_screen(&example, vb);
                        vb = normalize_coords(&example, vb);

                        ADD_LINE(va.x, va.y, 0.0, 0.0, 0.0, 0.0);

                        for (int i = 0; i < spline_cons->num_controls; i++) {
                            va = spline_cons->controls[i];

                            nv_float s = 3.0 / example.zoom;
                            nvVector2 p0 = nvVector2_add(va, NV_VECTOR2(-s, -s));
                            nvVector2 p1 = nvVector2_add(va, NV_VECTOR2(-s, s));
                            nvVector2 p2 = nvVector2_add(va, NV_VECTOR2(s, s));
                            nvVector2 p3 = nvVector2_add(va, NV_VECTOR2(s, -s));

                            va = world_to_screen(&example, va);
                            va = normalize_coords(&example, va);
                            p0 = world_to_screen(&example, p0);
                            p0 = normalize_coords(&example, p0);
                            p1 = world_to_screen(&example, p1);
                            p1 = normalize_coords(&example, p1);
                            p2 = world_to_screen(&example, p2);
                            p2 = normalize_coords(&example, p2);
                            p3 = world_to_screen(&example, p3);
                            p3 = normalize_coords(&example, p3);

                            ADD_LINE(
                                va.x, va.y,
                                example.theme.spline_constraint.r,
                                example.theme.spline_constraint.g,
                                example.theme.spline_constraint.b,
                                0.19
                            );

                            ADD_TRIANGLE(
                                p0.x, p0.y,
                                p1.x, p1.y,
                                p2.x, p2.y,
                                example.theme.spline_constraint.r,
                                example.theme.spline_constraint.g,
                                example.theme.spline_constraint.b,
                                1.0
                            );

                            ADD_TRIANGLE(
                                p0.x, p0.y,
                                p2.x, p2.y,
                                p3.x, p3.y,
                                example.theme.spline_constraint.r,
                                example.theme.spline_constraint.g,
                                example.theme.spline_constraint.b,
                                1.0
                            );
                        }

                        ADD_LINE(va.x, va.y, 0.0, 0.0, 0.0, 0.0);

                        #define SPLINE_SAMPLES 200
                        nvVector2 sampled[SPLINE_SAMPLES];
                        sample_spline(spline_cons, sampled, SPLINE_SAMPLES);

                        va = sampled[0];
                        va = world_to_screen(&example, va);
                        va = normalize_coords(&example, va);
                        ADD_LINE(va.x, va.y, 0.0, 0.0, 0.0, 0.0);

                        for (size_t i = 0; i < SPLINE_SAMPLES; i++) {
                            va = sampled[i];

                            va = world_to_screen(&example, va);
                            va = normalize_coords(&example, va);

                            ADD_LINE(va.x, va.y, 1.0, 1.0, 1.0, 1.0);
                        }

                        ADD_LINE(va.x, va.y, 0.0, 0.0, 0.0, 0.0);

                        break;
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

                    // Draw either penetration depth or normal impulse
                    nv_float normal_mag;
                    if (draw_normal_impulses)
                        normal_mag = contact.solver_info.normal_impulse;
                    else
                        normal_mag = -contact.separation;

                    nvVector2 pn = nvVector2_add(p, nvVector2_mul(pcp->normal, normal_mag));

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

                    // Draw friction impulses
                    if (draw_friction_impulses) {
                        nv_float tangent_mag = contact.solver_info.tangent_impulse;
                        nvVector2 pt = nvVector2_add(
                            nvVector2_add(pa, contact.anchor_b), nvVector2_mul(nvVector2_perpr(pcp->normal), tangent_mag));
                        pt = world_to_screen(&example, pt);
                        pt = normalize_coords(&example, pt);

                        ADD_LINE(p.x, p.y, 0.0, 0.0, 0.0, 0.0);
                        ADD_LINE(
                            p.x, p.y,
                            1.0,
                            0.8,
                            0.7,
                            color.a
                        );
                        ADD_LINE(
                            pt.x, pt.y,
                            1.0,
                            0.8,
                            0.7,
                            color.a
                        );
                        ADD_LINE(p.x, p.y, 0.0, 0.0, 0.0, 0.0);
                    }

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

        if (draw_broadphase) {
            if (nvSpace_get_broadphase(example.space) == nvBroadPhaseAlg_BVH) {
                nvBVHNode *bvh = nvBVHTree_new(example.space->bodies);
                bvh_calc_depth(bvh, 0);
                nv_int64 max_depth = bvh_max_depth(bvh);

                nvArray *stack = nvArray_new();
                nvBVHNode *current = bvh;

                while (stack->size != 0 || current) {
                    while (current) {
                        nvArray_add(stack, current);
                        current = current->left;
                    }
                    // Current node is NULL at this point
                    // continue from stack

                    current = nvArray_pop(stack, stack->size - 1);

                    nvAABB saabb = current->aabb;
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

                    double t = (double)current->depth / (double)max_depth;

                    FColor color = FColor_lerp((FColor){0.0, 0.0, 1.0, 1.0}, (FColor){1.0, 0.0, 0.0, 1.0}, t);

                    ADD_TRIANGLE(p0.x, p0.y, p1.x, p1.y, p2.x, p2.y, color.r, color.g, color.b, 0.1);
                    ADD_TRIANGLE(p0.x, p0.y, p3.x, p3.y, p2.x, p2.y, color.r, color.g, color.b, 0.1);

                    ADD_LINE(p0.x, p0.y, color.r, color.g, color.b, 0.0);
                    ADD_LINE(p0.x, p0.y, color.r, color.g, color.b, 0.7);
                    ADD_LINE(p1.x, p1.y, color.r, color.g, color.b, 0.7);
                    ADD_LINE(p2.x, p2.y, color.r, color.g, color.b, 0.7);
                    ADD_LINE(p3.x, p3.y, color.r, color.g, color.b, 0.7);
                    ADD_LINE(p0.x, p0.y, color.r, color.g, color.b, 0.7);
                    ADD_LINE(p0.x, p0.y, color.r, color.g, color.b, 0.0);

                    current = current->right;
                }

                nvArray_free(stack);
                nvBVHTree_free(bvh);
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

    NV_FREE(tri_vertices);
    NV_FREE(tri_colors);
    NV_FREE(line_vertices);
    NV_FREE(line_colors);

    glDeleteVertexArrays(2, vaos);
    glDeleteBuffers(4, vbos);
    glDeleteProgram(program);

    SDL_GL_DeleteContext(example.gl_ctx);
    SDL_DestroyWindow(example.window);
    SDL_Quit();

    return EXIT_SUCCESS;
}