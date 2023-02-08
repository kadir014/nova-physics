/*

  This file is a part of the Nova Physics Engine
  project and distributed under the MIT license.

  Copyright © Kadir Aksoy
  https://github.com/kadir014/Nova-Physics

*/

#include <math.h>
#include "novaphysics/vector.h"


/**
 * vector.c
 * 
 * 2D vector type and vector math
 */


bool nv_Vector2_eq(nv_Vector2 a, nv_Vector2 b) {
    return (a.x == b.x && a.y == b.y);
}

nv_Vector2 nv_Vector2_add(nv_Vector2 a, nv_Vector2 b) {
    return (nv_Vector2){a.x + b.x, a.y + b.y};
}

nv_Vector2 nv_Vector2_sub(nv_Vector2 a, nv_Vector2 b) {
    return (nv_Vector2){a.x - b.x, a.y - b.y};
}

nv_Vector2 nv_Vector2_mulv(nv_Vector2 a, nv_Vector2 b) {
    return (nv_Vector2){a.x * b.x, a.y * b.y};
}

nv_Vector2 nv_Vector2_muls(nv_Vector2 v, double s) {
    return (nv_Vector2){v.x * s, v.y * s};
}

nv_Vector2 nv_Vector2_divv(nv_Vector2 a, nv_Vector2 b) {
    return (nv_Vector2){a.x / b.x, a.y / b.y};
}

nv_Vector2 nv_Vector2_divs(nv_Vector2 v, double s) {
    return (nv_Vector2){v.x / s, v.y / s};
}

nv_Vector2 nv_Vector2_neg(nv_Vector2 v) {
    return (nv_Vector2){-v.x, -v.y};
}

nv_Vector2 nv_Vector2_normalize(nv_Vector2 v) {
    return nv_Vector2_divs(v, nv_Vector2_len(v));
}

nv_Vector2 nv_Vector2_rotate(nv_Vector2 v, double a) {
    double c = cos(a);
    double s = sin(a);
    return (nv_Vector2){c * v.x - s * v.y, s * v.x + c * v.y};
}

nv_Vector2 nv_Vector2_perp(nv_Vector2 v) {
    return (nv_Vector2){-v.y, v.x};
}

nv_Vector2 nv_Vector2_perpr(nv_Vector2 v) {
    return (nv_Vector2){v.y, -v.x};
}

double nv_Vector2_len2(nv_Vector2 v) {
    return v.x * v.x + v.y * v.y;
}

double nv_Vector2_len(nv_Vector2 v) {
    return sqrt(nv_Vector2_len2(v));
}

double nv_Vector2_dot(nv_Vector2 a, nv_Vector2 b) {
    return a.x * b.x + a.y * b.y;
}

double nv_Vector2_cross(nv_Vector2 a, nv_Vector2 b) {
    return a.x * b.y - a.y * b.x;
}

double nv_Vector2_dist2(nv_Vector2 a, nv_Vector2 b) {
    return (b.x - a.x) * (b.x - a.x) + (b.y - a.y) * (b.y - a.y);
}

double nv_Vector2_dist(nv_Vector2 a, nv_Vector2 b) {
    return sqrt(nv_Vector2_dist2(a, b));
}


nv_Vector2Array *nv_Vector2Array_new() {
    nv_Vector2Array *array = (nv_Vector2Array *)malloc(sizeof(nv_Vector2Array));

    array->size = 0;

    array->data = (nv_Vector2 *)malloc(sizeof(nv_Vector2));

    return array;
}

void nv_Vector2Array_free(nv_Vector2Array *array) {
    free(array->data);
    array->data = NULL;
    array->size = 0;
    free(array);
}

void nv_Vector2Array_add(nv_Vector2Array *array, nv_Vector2 vector) {
    array->size += 1;

    array->data = (nv_Vector2 *)realloc(array->data, array->size * sizeof(nv_Vector2));

    array->data[array->size - 1] = vector;
}