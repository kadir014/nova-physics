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
 * @brief Return the minimum of two values
 * 
 * @param a Left-hand value
 * @param b Right-hand value
 * @return double 
 */
double nv_minf(double a, double b);

/**
 * @brief Return the maximum of two values
 * 
 * @param a Left-hand value
 * @param b Right-hand value
 * @return double 
 */
double nv_maxf(double a, double b);


/**
 * @brief Calculate factorial of n (n!)
 * 
 * @param n Value
 * @return int 
 */
int nv_fact(int n);


/**
 * @brief Calculate combination (n! / r! * (n-r)!)
 * 
 * @param n Value
 * @param r Value
 * @return int 
 */
int nv_comb(int n, int r);


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
 * @brief Check if point is inside the polygon
 * 
 * @param point Point
 * @param vertices Vertices of the polygon
 * @return bool
 */
bool nv_point_x_polygon(nv_Vector2 point, nv_Array *vertices);


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