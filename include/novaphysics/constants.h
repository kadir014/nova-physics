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
#include "novaphysics/types.h"


/**
 * @file constants.h
 * 
 * @brief Various common constants used in the Nova Physics Engine.
 */


#define NV_PI      ((nv_float)3.141592653589793238462643383279502884)
#define NV_HALF_PI ((nv_float)1.570796326794896619231321691639751442)
#define NV_TAU     ((nv_float)6.283185307179586476925286766559005768)

// Inverse golden ratio, for golden-section search.
#define NV_INV_PHI ((nv_float)0.618033988749894848204586834365638117)

#ifndef INFINITY
    #define NV_INF ((nv_float)1.0f / (nv_float)0.0f)
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
#define NV_SPLINE_CONSTRAINT_TOLERANCE ((nv_float)0.00001)


// How many bodies one leaf node can store before terminating.
#define NV_BVH_LEAF_THRESHOLD 1

// Initial size for flat node array for the BVH-tree.
// 64B * 10000 =~ 625KB
#define NV_BVH_NODES_INITIAL_SIZE 10000


// Default capacity of hash maps, must be a power of 2.
#define NV_HASHMAP_CAPACITY 1024


// Initial size for the broadphase memory pool. (16B * 10000 =~ 160KB)
#define NV_BPH_POOL_INITIAL_SIZE 10000


#endif