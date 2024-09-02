/*

  This file is a part of the Nova Physics Engine
  project and distributed under the MIT license.

  Copyright © Kadir Aksoy
  https://github.com/kadir014/nova-physics

*/

#ifndef NOVAPHYSICS_CONSTANTS_H
#define NOVAPHYSICS_CONSTANTS_H

#include <math.h>
#include <float.h>


/**
 * @file constants.h
 * 
 * @brief Various common constants used in the Nova Physics Engine.
 */


#define NV_PI 3.141592653589793238462643383279502884

// Inverse golden ratio, for golden-section search.
#define NV_INV_PHI 0.6180339887498948482045868343656

#ifndef INFINITY
    #define NV_INF (1.0 / 0.0)
#else
    #define NV_INF INFINITY
#endif

#ifdef NV_USE_DOUBLE_PRECISION
    #define NV_FLOAT_EPSILON DBL_EPSILON
#else
    #define NV_FLOAT_EPSILON FLT_EPSILON
#endif


// Maximum number of vertices one polygon shape can have.
#define NV_POLYGON_MAX_VERTICES 16


// Maximum number of control points one spline constraint can have.
#define NV_SPLINE_CONSTRAINT_MAX_CONTROL_POINTS 64

// Number of samples used to get the closes spline segment.
#define NV_SPLINE_CONSTRAINT_SAMPLES 500

// Tolerance for golden-section search used in spline constraints.
#define NV_SPLINE_CONSTRAINT_TOLERANCE 0.00001


// How many bodies one leaf node can store before terminating
#define NV_BVH_LEAF_THRESHOLD 1


// Gravitational constant. G = 6.6743 * 10^-11
#define NV_GRAV_CONST 6.6743e-11

// Scaling factor applied to gravitational constant when attractive forces are applied.
#define NV_GRAV_SCALE 1e13

// Various gravitational pulls of different celestial bodies.
#define NV_GRAV_EARTH 9.81
#define NV_GRAV_MOON 1.62
#define NV_GRAV_MARS 3.7
#define NV_GRAV_JUPITER 24.5
#define NV_GRAV_SUN 275.0
#define NV_GRAV_VOID 0.0


// Default capacity of hash maps, must be a power of 2.
#define NV_HASHMAP_CAPACITY 1024


// Initial size for the broadphase memory pool.
// 16B * 10000 =~ 160KB  I think this is sufficient for an arbitrary default size.
#define NV_BPH_POOL_INITIAL_SIZE 10000


#endif