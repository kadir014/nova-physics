/*

  This file is a part of the Nova Physics Engine
  project and distributed under the MIT license.

  Copyright © Kadir Aksoy
  https://github.com/kadir014/Nova-Physics

*/

#include "novaphysics/resolution.h"

/**
 * resolution.h
 * 
 * Collision resolution (manifold) data structure
 */


nv_ResolutionArray *nv_ResolutionArray_new() {
    nv_ResolutionArray *array = (nv_ResolutionArray *)malloc(sizeof(nv_ResolutionArray));

    array->size = 0;

    array->data = (nv_Resolution *)malloc(sizeof(nv_Resolution));

    return array;
}

void nv_ResolutionArray_free(nv_ResolutionArray *array) {
    free(array->data);
    array->data = NULL;
    array->size = 0;
    free(array);
}

void nv_ResolutionArray_add(nv_ResolutionArray *array, nv_Resolution res) {
    array->size += 1;

    array->data = (nv_Resolution *)realloc(array->data, array->size * sizeof(nv_Resolution));

    array->data[array->size - 1] = res;
}