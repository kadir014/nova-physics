#ifndef NOVAPHYSICS_EXAMPLE_GL_HELPER_H
#define NOVAPHYSICS_EXAMPLE_GL_HELPER_H

#include <stdio.h>
#include <stdlib.h>

#include <GL/gl.h>
#include <glad/glad.h>

#include "common.h"


nv_uint32 ngl_load_shader(const char *source, int shader_type) {
    nv_uint32 shader_id = glCreateShader(shader_type);
    glShaderSource(shader_id, 1, &source, NULL);
    glCompileShader(shader_id);

    int success;
    glGetShaderiv(shader_id, GL_COMPILE_STATUS, &success);
    if (!success) {
        fprintf(stderr, "Shader compilation error.\n");
        exit(EXIT_FAILURE);
    }

    return shader_id;
}

nv_uint32 ngl_create_vbo() {
    nv_uint32 vbo_id;
    glGenBuffers(1, &vbo_id);
    return vbo_id;
}

nv_uint32 ngl_create_vao() {
    nv_uint32 vao_id;
    glGenVertexArrays(1, &vao_id);
    return vao_id;
}

void ngl_clear(float red, float green, float blue, float alpha) {
    glClearColor(red, green, blue, alpha);
    glClear(GL_COLOR_BUFFER_BIT);
}

void ngl_vao_render(nv_uint32 vao_id, nv_uint32 mode, size_t count) {
    glBindVertexArray(vao_id);
    glDrawArrays(mode, 0, count);
    glBindVertexArray(0);
}


#endif