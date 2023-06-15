/*

  This file is a part of the Nova Physics Engine
  project and distributed under the MIT license.

  Copyright © Kadir Aksoy
  https://github.com/kadir014/nova-physics

*/

#ifndef NOVAPHYSICS_DEBUG_H
#define NOVAPHYSICS_DEBUG_H

#include <stdbool.h>
#include <stdio.h>
#include "novaphysics/novaphysics.h"


/**
 * @file debug.h
 * 
 * @brief Debug printing functions of Nova Physics objects.
 */


// Utility macro to represent boolean as string
#define __B(x) ((x) ? "true" : "false")


/*
    AABB debug utilities
*/

static inline void nv_print_AABB(nv_AABB aabb) {
    printf("AABB(%.2f, %.2f, %.2f, %2.f)", aabb.min_x, aabb.min_y, aabb.max_x, aabb.max_y);
}

static inline void nv_println_AABB(nv_AABB aabb) {
    nv_print_AABB(aabb);
    printf("\n");
}


/*
    nv_Vector2 debug utilities
*/

static inline void nv_print_Vector2(nv_Vector2 vector) {
    printf("Vector2(%.2f, %.2f)", vector.x, vector.y);
}

static inline void nv_println_Vector2(nv_Vector2 vector) {
    nv_print_Vector2(vector);
    printf("\n");
}


/*
    nv_Body debug utilities
*/

static inline void nv_print_Body(nv_Body *body) {
    char *p0 =
    "Body at 0x%X:\n"
    "  ID:           %u\n"
    "  Type:         %s\n"
    "  Shape:        %s\n"
    "  Position:     ";

    char *p1 =
    "  Angle:        %.4f rad (%.1f deg)\n"
    "  Force:        ";

    char *p2 =
    "  Torque:       %.1f Nm\n"
    "  Mass:         %.1f kg\n"
    "  Inertia:      %.1f kgm^2\n"
    "  Vertices:     %u\n"
    "  Is sleeping?  %s\n"
    "  Is attractor? %s\n"
    "  Material:\n"
    "    Density:     %.2f\n"
    "    Restitution: %.2f\n"
    "    Friction:    %.2f\n";

    printf(
        p0,
        body,
        body->id,
        body->type ? "Dynamic" : "Static",
        body->shape ? "Polygon" : "Circle"
    );

    nv_print_Vector2(body->position);
    printf(" m\n");

    printf(
        p1,
        body->angle,
        body->angle * (180.0 / NV_PI)
    );

    nv_print_Vector2(body->force);
    printf(" N\n");

    nv_uint8 vertices;
    if (body->shape->type == nv_ShapeType_CIRCLE) vertices = 0;
    else if (body->shape->type == nv_ShapeType_POLYGON) vertices = body->shape->vertices->size;

    printf(
        p2,
        body->torque,
        body->mass,
        body->inertia,
        vertices,
        __B(body->is_sleeping),
        __B(body->is_attractor),
        body->material.density,
        body->material.restitution,
        body->material.friction
    );
}


// Do not expose __B macro
#undef __B


#endif