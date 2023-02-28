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
 * Debug printing functions of Nova Physics objects
 */


// Utility macro to represent boolean as string
#define __B(x) ((x) ? "true" : "false")


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
    "  Is sleeping?  %s\n"
    "  Is attractor? %s\n"
    "  Material:\n"
    "    Density:     %.2f\n"
    "    Restitution: %.2f\n"
    "    Friction:    %.2f\n";

    printf(
        p0,
        body,
        body->type ? "Static" : "Dynamic",
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

    printf(
        p2,
        body->torque,
        body->mass,
        body->inertia,
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