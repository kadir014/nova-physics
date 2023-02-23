/*

  This file is a part of the Nova Physics Engine
  project and distributed under the MIT license.

  Copyright © Kadir Aksoy
  https://github.com/kadir014/nova-physics

*/

#ifndef NOVAPHYSICS_MATH_H
#define NOVAPHYSICS_MATH_H

#include <stdbool.h>
#include "novaphysics/array.h"
#include "novaphysics/vector.h"


/**
 * math.h
 * 
 * Nova physics math utilities
 */


/**
 * @brief Check if two values are close enough to count as equal
 * 
 * @param a Left-hand value
 * @param b Right-hand value
 * @return bool
 */
bool nv_nearly_eq(double a, double b);

/**
 * @overload Check if two vectors are close enough to count as equal
 * 
 * @param a Left-hand vector
 * @param b Right-hand vector
 * @return bool
 */
bool nv_nearly_eqv(nv_Vector2 a, nv_Vector2 b);


/**
 * @brief Calculate relative velocity
 * 
 * @param linear_velocity_a Linear velocity of body A
 * @param anuglar_velocity_a Angular velocity of body A
 * @param ra Vector from body A position to its local anchor point 
 * @param linear_velocity_b Linear velocity of body B
 * @param anuglar_velocity_b Angular velocity of body B
 * @param rb Vector from body B position to its local anchor point 
 * @return nv_Vector2 
 */
nv_Vector2 nv_calc_relative_velocity(
    nv_Vector2 linear_velocity_a,
    double angular_velocity_a,
    nv_Vector2 ra,
    nv_Vector2 linear_velocity_b,
    double angular_velocity_b,
    nv_Vector2 rb
);

/**
 * @brief Calculate normal / tangential mass
 * 
 * @param normal Normal
 * @param ra Vector from body A position to contact point
 * @param rb vector from body B position to contact point
 * @param invmass_a Inverse mass (1/M) of body A
 * @param invmass_b Inverse mass (1/M) of body B
 * @param invinertia_a Inverse moment of inertia (1/I) of body A
 * @param invinertia_b Inverse moment of inertia (1/I) of body B
 * @return double 
 */
double nv_calc_mass_k(
    nv_Vector2 normal,
    nv_Vector2 ra,
    nv_Vector2 rb,
    double invmass_a,
    double invmass_b,
    double invinertia_a,
    double invinertia_b
);


/**
 * @brief Calculate area of a circle (πr²)
 * 
 * @param radius  Radius of the circle
 * @return double 
 */
double nv_circle_area(double radius);

/**
 * @brief Calculate moment of inertia of a circle (1/2 mr²)
 * 
 * @param mass Mass of the circles
 * @param radius Radius of the circle
 * @return double 
 */
double nv_circle_inertia(double mass, double radius);

/**
 * @brief Calculate area of a polygon (Shoelace formula)
 * 
 * @param vertices Array of vertices of polygon
 * @return double 
 */
double nv_polygon_area(nv_Array *vertices);

/**
 * @brief Calculate moment of inertia of a polygon
 * 
 * @param mass Mass of the polygon
 * @param vertices Array of vertices of polygon
 * @return double 
 */
double nv_polygon_inertia(double mass, nv_Array *vertices);

/**
 * @brief Calculate centroid of a polygon
 * 
 * @param vertices Array of vertices of polygon
 * @return nv_Vector2
 */
nv_Vector2 nv_polygon_centroid(nv_Array *vertices);


/**
 * @brief Project circle onto axis and return min & max points
 * 
 * @param center Center of circle
 * @param radius Radius of circle
 * @param axis Axis vector to project on
 * @param min_out Pointer for out min value
 * @param max_out Pointer for out max value
 */
void nv_project_circle(
    nv_Vector2 center,
    double radius,
    nv_Vector2 axis,
    double *min_out,
    double *max_out
);

/**
 * @brief Project polygon onto axis and return min & max points
 * 
 * @param vertices Vertices of the polygon
 * @param axis Axis vector to project on
 * @param min_out Pointer for out min value
 * @param max_out Pointer for out max value
 */
void nv_project_polyon(
    nv_Array *vertices,
    nv_Vector2 axis,
    double *min_out,
    double *max_out
);


/**
 * @brief Perp distance between point and line segment
 * 
 * @param center
 * @param a 
 * @param b 
 * @param dist_out 
 * @param edge_out 
 */
void nv_point_segment_dist(
    nv_Vector2 center,
    nv_Vector2 a,
    nv_Vector2 b,
    double *dist_out,
    nv_Vector2 *edge_out
);


/**
 * @brief Find closest vertex of the polygon to the circle
 * 
 *      X -         _____
 *     /   -       /     \
 *    /     X --- |   o   |
 *   X     /       \_____/
 *    -   /
 *     - X
 * 
 * @param center Center of the circle
 * @param vertices Vertices of the polygon
 * @return nv_Vector2 
 */
nv_Vector2 nv_polygon_closest_vertex_to_circle(
    nv_Vector2 center,
    nv_Array *vertices
);


#endif