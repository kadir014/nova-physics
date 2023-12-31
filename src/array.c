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


nvArray *nvArray_new() {
    nvArray *array = (nvArray *)malloc(sizeof(nvArray));
    if (!array) return NULL;

    array->size = 0;
    array->max = 0;
    array->data = (void **)malloc(sizeof(void *));
    if (!array->data) {
        free(array);
        return NULL;
    }

    return array;
}

void nvArray_free(nvArray *array) {
    free(array->data);
    array->data = NULL;
    array->size = 0;
    free(array);
}

void nvArray_free_each(nvArray *array, void (free_func)(void *)) {
    for (size_t i = 0; i < array->size; i++)
        free_func(array->data[i]);
}

void nvArray_add(nvArray *array, void *elem) {
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

void *nvArray_pop(nvArray *array, size_t index) {
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

size_t nvArray_remove(nvArray *array, void *elem) {
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

void nvArray_clear(nvArray *array, void (free_func)(void *)) {
    /*
        We can set array->max to 0 and reallocate but
        not doing it might be more efficient for the developer
        since they will probably fill the array up again.
        Maybe a separate parameter for this?
    */

    if (array->size == 0) return;
   
    if (!free_func) {
        while (array->size > 0) {
            nvArray_pop(array, 0);
        }
    }

    else {
        while (array->size > 0) {
            free_func(nvArray_pop(array, 0));
        }
    }
}