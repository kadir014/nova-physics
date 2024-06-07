/*

  This file is a part of the Nova Physics Engine
  project and distributed under the MIT license.

  Copyright © Kadir Aksoy
  https://github.com/kadir014/nova-physics

*/

#ifndef NOVAPHYSICS_SIMD_H
#define NOVAPHYSICS_SIMD_H

#include <immintrin.h>


/**
 * @file simd.h
 * 
 * @brief Experimental x86 SIMD intrinsics abstraction header.
 * 
 * nv_vfloat             Float vector, width depends on the target extension.
 * nv_vfloat_[...]       Float vector operations.
 * NV_SIMD_VFLOAT_WIDTH  Width of the float vector.
 *                       For example, the width of a float vector under AVX
 *                       would be 8. (__m256 -> 8x32-bit sp floats)
 * 
 * NV_SIMD_AVX_PRESENT   AVX extension is present.
 * NV_SIMD_SSE2_PRESENT  SSE2 extension is present.
 * NV_SIMD_NONE_PRESENT  No SIMD extension is present.
 * 
 * NV_SIMD_TARGET        The SIMD extension this abstraction targets.
 * NV_SIMD_TARGET_[...]  ^
 */


#ifdef __AVX__

    #define NV_SIMD_AVX_PRESENT

#endif

#if defined(__SSE2__) || defined(_M_AMD64) || defined(_M_X64)

    #define NV_SIMD_SSE2_PRESENT

#endif

#if !defined(NV_SIMD_AVX_PRESENT) && !defined(NV_SIMD_SSE2_PRESENT)

    #define NV_SIMD_NONE_PRESENT

#endif


#define NV_SIMD_TARGET_NONE 0
#define NV_SIMD_TARGET_AVX 1
#define NV_SIMD_TARGET_SSE2 2

#if defined(NV_SIMD_NONE_PRESENT) || defined(NV_SIMD_DISABLED)

    #define NV_SIMD_TARGET NV_SIMD_TARGET_NONE
    #define NV_SIMD_VFLOAT_WIDTH 1
    #define NV_SIMD_VFLOAT_ALIGNMENT 4

#elif defined(NV_SIMD_AVX_PRESENT)

    #define NV_SIMD_TARGET NV_SIMD_TARGET_AVX
    #define NV_SIMD_VFLOAT_WIDTH 8
    #define NV_SIMD_VFLOAT_ALIGNMENT 32

#elif defined(NV_SIMD_SSE2_PRESENT)

    #define NV_SIMD_TARGET NV_SIMD_TARGET_SSE2
    #define NV_SIMD_VFLOAT_WIDTH 4
    #define NV_SIMD_VFLOAT_ALIGNMENT 16

#endif


#if NV_SIMD_TARGET == NV_SIMD_TARGET_NONE

    typedef float nv_vfloat;

#elif NV_SIMD_TARGET == NV_SIMD_TARGET_AVX

    typedef __m256 nv_vfloat;

#elif NV_SIMD_TARGET == NV_SIMD_TARGET_SSE2

    typedef __m128 nv_vfloat;

#endif


#if NV_SIMD_TARGET == NV_SIMD_TARGET_NONE

    #define nv_vfloat_set(a) (a)

#elif NV_SIMD_TARGET == NV_SIMD_TARGET_AVX

    #define nv_vfloat_set(a, b, c, d, e, f, g, h) _mm256_set_ps(a, b, c, d, e, f, g, h)

#elif NV_SIMD_TARGET == NV_SIMD_TARGET_SSE2

    #define nv_vfloat_set(a, b, c, d) _mm_set_ps(a, b, c, d)

#endif


#if NV_SIMD_TARGET == NV_SIMD_TARGET_NONE

    #define nv_vfloat_set1(a) (a)

#elif NV_SIMD_TARGET == NV_SIMD_TARGET_AVX

    #define nv_vfloat_set1(a) _mm256_set1_ps(a)

#elif NV_SIMD_TARGET == NV_SIMD_TARGET_SSE2

    #define nv_vfloat_set1(a) _mm_set1_ps(a)

#endif


#if NV_SIMD_TARGET == NV_SIMD_TARGET_NONE

    #define nv_vfloat_store(arr, v) {arr[0] = v;}

#elif NV_SIMD_TARGET == NV_SIMD_TARGET_AVX

    #define nv_vfloat_store(arr, v) _mm256_store_ps(arr, v)

#elif NV_SIMD_TARGET == NV_SIMD_TARGET_SSE2

    #define nv_vfloat_store(arr, v) _mm_store_ps(arr, v)

#endif


#if NV_SIMD_TARGET == NV_SIMD_TARGET_NONE

    #define nv_vfloat_add(a, b) ((a) + (b))

#elif NV_SIMD_TARGET == NV_SIMD_TARGET_AVX

    #define nv_vfloat_add(a, b) _mm256_add_ps(a, b)

#elif NV_SIMD_TARGET == NV_SIMD_TARGET_SSE2

    #define nv_vfloat_add(a, b) _mm_add_ps(a, b)

#endif


#if NV_SIMD_TARGET == NV_SIMD_TARGET_NONE

    #define nv_vfloat_sub(a, b) ((a) - (b))

#elif NV_SIMD_TARGET == NV_SIMD_TARGET_AVX

    #define nv_vfloat_sub(a, b) _mm256_sub_ps(a, b)

#elif NV_SIMD_TARGET == NV_SIMD_TARGET_SSE2

    #define nv_vfloat_sub(a, b) _mm_sub_ps(a, b)

#endif


#if NV_SIMD_TARGET == NV_SIMD_TARGET_NONE

    #define nv_vfloat_mul(a, b) ((a) * (b))

#elif NV_SIMD_TARGET == NV_SIMD_TARGET_AVX

    #define nv_vfloat_mul(a, b) _mm256_mul_ps(a, b)

#elif NV_SIMD_TARGET == NV_SIMD_TARGET_SSE2

    #define nv_vfloat_mul(a, b) _mm_mul_ps(a, b)

#endif


#if NV_SIMD_TARGET == NV_SIMD_TARGET_NONE

    #define nv_vfloat_div(a, b) ((a) / (b))

#elif NV_SIMD_TARGET == NV_SIMD_TARGET_AVX

    #define nv_vfloat_div(a, b) _mm256_div_ps(a, b)

#elif NV_SIMD_TARGET == NV_SIMD_TARGET_SSE2

    #define nv_vfloat_div(a, b) _mm_div_ps(a, b)

#endif


#endif