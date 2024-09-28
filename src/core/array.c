/*

  This file is a part of the Nova Physics Engine
  project and distributed under the MIT license.

  Copyright © Kadir Aksoy
  https://github.com/kadir014/nova-physics

*/

#include "novaphysics/core/array.h"


/**
 * @file core/array.c
 * 
 * @brief Type-generic dynamically growing array implementation.
 */


nvArray *nvArray_new() {
    nvArray *array = NV_NEW(nvArray);
    NV_MEM_CHECK(array);

    array->size = 0;
    array->max = 1;
    array->growth_factor = 2.0;
    array->data = (void **)NV_MALLOC(sizeof(void *));
    if (!array->data) NV_FREE(array);
    NV_MEM_CHECK(array->data);

    return array;
}

nvArray *nvArray_new_ex(size_t default_capacity, float growth_factor) {
    nvArray *array = NV_NEW(nvArray);
    NV_MEM_CHECK(array);

    array->size = 0;
    array->max = default_capacity;
    array->growth_factor = growth_factor;
    array->data = (void **)NV_MALLOC(sizeof(void *) * default_capacity);
    if (!array->data) NV_FREE(array);
    NV_MEM_CHECK(array->data);

    return array;
}

void nvArray_free(nvArray *array) {
    if (!array) return;

    NV_FREE(array->data);
    NV_FREE(array);
}

void nvArray_free_each(nvArray *array, nvArray_free_each_callback free_func) {
    for (size_t i = 0; i < array->size; i++)
        free_func(array->data[i]);
}

int nvArray_add(nvArray *array, void *elem) {
    // Only reallocate when max capacity is reached
    if (array->size == array->max) {
        array->size++;
        array->max = (size_t)((float)array->max * array->growth_factor);
        array->data = NV_REALLOC(array->data, array->max * sizeof(void *));
        NV_MEM_CHECKI(array->data);
    }
    else {
        array->size++;
    }

    array->data[array->size - 1] = elem;

    return 0;
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

int nvArray_clear(nvArray *array, void (free_func)(void *)) {
    /*
        We can set array->max to 0 and reallocate but
        not doing it might be more efficient for the developer
        since they will probably fill the array up again.
        Maybe a separate parameter for this?
    */

    if (array->size == 0) return 0;
   
    if (!free_func) {
        while (array->size > 0) {
            if (!nvArray_pop(array, 0))
                return 1;
        }
    }

    else {
        while (array->size > 0) {
            void *p = nvArray_pop(array, 0);
            if (!p) return 1;
            free_func(p);
        }
    }

    return 0;
}