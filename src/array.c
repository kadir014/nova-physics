/*

  This file is a part of the Nova Physics Engine
  project and distributed under the MIT license.

  Copyright © Kadir Aksoy
  https://github.com/kadir014/nova-physics

*/

#include "novaphysics/array.h"


/**
 * @file array.c
 * 
 * @brief Type-generic dynamically growing array implementation.
 */


nv_Array *nv_Array_new() {
    nv_Array *array = (nv_Array *)malloc(sizeof(nv_Array));
    if (!array) return NULL;

    array->size = 0;
    array->max = 0;
    array->data = (void **)malloc(sizeof(void *));

    return array;
}

void nv_Array_free(nv_Array *array) {
    free(array->data);
    array->data = NULL;
    array->size = 0;
    free(array);
}

void nv_Array_free_each(nv_Array *array, void (free_func)(void *)) {
    for (size_t i = 0; i < array->size; i++)
        free_func(array->data[i]);
}

void nv_Array_add(nv_Array *array, void *elem) {
    // Only reallocate when max capacity is reached
    if (array->size == array->max) {
        array->size++;
        array->max++;
        array->data = (void **)realloc(array->data, array->size * sizeof(void *));
    }
    else {
        array->size++;
    }

    array->data[array->size - 1] = elem;
}

void *nv_Array_pop(nv_Array *array, size_t index) {
    for (size_t i = 0; i < array->size; i++) {
        if (i == index) {
            array->size--;
            void *elem = array->data[i];

            array->data[i] = array->data[array->size];
            array->data[array->size] = NULL;

            return elem;
        }
    }

    return NULL;
}

size_t nv_Array_remove(nv_Array *array, void *elem) {
    for (size_t i = 0; i < array->size; i++) {
        if (array->data[i] == elem) {
            array->size--;

            array->data[i] = array->data[array->size];
            array->data[array->size] = NULL;

            return i;
        }
    }

    return -1;
}