/*

  This file is a part of the Nova Physics Engine
  project and distributed under the MIT license.

  Copyright © Kadir Aksoy
  https://github.com/kadir014/nova-physics

*/

#ifndef NOVAPHYSICS_DEBUG_H
#define NOVAPHYSICS_DEBUG_H

#include "novaphysics/internal.h"
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

static inline void nv_print_AABB(nvAABB aabb) {
    printf("AABB(%.2f, %.2f, %.2f, %2.f)", aabb.min_x, aabb.min_y, aabb.max_x, aabb.max_y);
}

static inline void nv_println_AABB(nvAABB aabb) {
    nv_print_AABB(aabb);
    printf("\n");
}


/*
    nvVector2 debug utilities
*/

static inline void nv_print_Vector2(nvVector2 vector) {
    printf("Vector2(%.2f, %.2f)", vector.x, vector.y);
}

static inline void nv_println_Vector2(nvVector2 vector) {
    nv_print_Vector2(vector);
    printf("\n");
}


/*
    nvRigidBody debug utilities
*/

// static inline void nv_print_Body(nvRigidBody *body) {
//     char *p0 =
//     "Body at 0x%X:\n"
//     "  ID:           %u\n"
//     "  Type:         %s\n"
//     "  Shape:        %s\n"
//     "  Position:     ";

//     char *p1 =
//     "  Angle:        %.4f rad (%.1f deg)\n"
//     "  Force:        ";

//     char *p2 =
//     "  Torque:       %.1f Nm\n"
//     "  Mass:         %.1f kg\n"
//     "  Inertia:      %.1f kgm^2\n"
//     "  Vertices:     %u\n"
//     "  Is sleeping?  %s\n"
//     "  Is attractor? %s\n"
//     "  Material:\n"
//     "    Density:     %.2f\n"
//     "    Restitution: %.2f\n"
//     "    Friction:    %.2f\n";

//     printf(
//         p0,
//         body,
//         body->id,
//         body->type ? "Dynamic" : "Static",
//         body->shape->type ? "Polygon" : "Circle"
//     );

//     nv_print_Vector2(body->position);
//     printf(" m\n");

//     printf(
//         p1,
//         body->angle,
//         body->angle * (180.0 / NV_PI)
//     );

//     nv_print_Vector2(body->force);
//     printf(" N\n");

//     size_t vertices;
//     if (body->shape->type == nvShapeType_CIRCLE) vertices = 0;
//     else if (body->shape->type == nvShapeType_POLYGON) vertices = body->shape->vertices->size;

//     printf(
//         p2,
//         body->torque,
//         body->mass,
//         body->inertia,
//         vertices,
//         __B(body->is_sleeping),
//         __B(body->is_attractor),
//         body->material.density,
//         body->material.restitution,
//         body->material.friction
//     );
// }


/*
    nvResolution debug utilities
*/

static inline void nv_print_Resolution(nvResolution *res) {
    nvContact c0 = res->contacts[0];
    nvContact c1 = res->contacts[1];
    
    char *p0 =
    "Resolution at 0x%X:\n"
    "  State:          %u\n"
    "  Lifetime:       %u\n"
    "  Collision:      %s\n"
    "  Body A ID:      %u\n"
    "  Body B ID:      %u\n"
    "  Contact count:  %u\n"
    "  Velocity bias:  %f, %f\n"
    "  Effective mass: %f, %f\n"
    "  Jn:             %f, %f\n"
    "  Jt:             %f, %f\n"
    "  ID:             %llu, %llu\n";

    printf(
        p0,
        res,
        res->state,
        res->lifetime,
        __B(res->collision),
        res->a->id,
        res->b->id,
        res->contact_count,
        c0.velocity_bias, c1.velocity_bias,
        c0.mass_normal, c1.mass_normal,
        c0.jn, c1.jn,
        c0.jt, c1.jt,
        c0.id, c1.id
    );
}


/*
    nvBVH debug utilities
*/

static inline void nv_print_BVH(nvBVHNode *node, size_t indent) {
    #ifdef NV_COMPILER_MSVC

        char *indent_str = NV_MALLOC(sizeof(char) * (indent + 1));

    #else

        char indent_str[indent + 1];

    #endif

    for (size_t i = 0; i < indent; i++) indent_str[i] = ' '; 
    indent_str[indent] = '\0';

    if (node == NULL) {
        printf("\n%sNULL\n", indent_str);
    }
    else if (node->is_leaf) {
        printf("\n%sLeaf (%llu bodies)\n", indent_str, (unsigned long long)node->bodies->size);
    }
    else {
        if (indent == 0)
            printf("%sRoot (%llu bodies, %llu size):\n", indent_str, (unsigned long long)node->bodies->size, (unsigned long long)nvBVHNode_size(node));
 
        else
            printf(" (%llu bodies, %llu size):\n", (unsigned long long)node->bodies->size, (unsigned long long)nvBVHNode_size(node));

        printf("%s  Left", indent_str);
        nv_print_BVH(node->left, indent + 4);
        printf("%s  Right", indent_str);
        nv_print_BVH(node->right, indent + 4);
    }

    #ifdef NV_COMPILER_MSVC

        NV_FREEindent_str);

    #endif
}


// Do not expose __B macro
#undef __B


#endif