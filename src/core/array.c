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
 * @brief Dynamically growing type-specific array.
 */


nvArray *nvArray_new(size_t elem_size) {
    return nvArray_new_ex(elem_size, 1, 2.0f);
}

nvArray *nvArray_new_ex(
    size_t elem_size,
    size_t default_capacity,
    float growth_factor
) {
    nvArray *array = NV_NEW(nvArray);
    NV_MEM_CHECK(array);

    array->size = 0;
    array->capacity = default_capacity;
    array->element_size = elem_size;
    array->growth_factor = growth_factor;
    array->data = NULL;

    if (growth_factor <= 1.0f || default_capacity == 0 || elem_size == 0) {
        return array;
    }

    array->data = NV_MALLOC(elem_size * default_capacity);

    return array;
}

void nvArray_free(nvArray *array) {
    if (!array) {
        return;
    }

    NV_FREE(array->data);
    NV_FREE(array);
}

nv_bool nvArray_valid(const nvArray *array) {
    return !(
        !array ||
        !array->data ||
        array->growth_factor <= 1.0f ||
        array->capacity == 0 ||
        array->size > array->capacity ||
        array->element_size == 0
    );
}

int nvArray_add(nvArray *array, void *elem) {
    if (!array || !elem) {
        nv_set_error("Invalid arguments, either array or elem is NULL.");
        return 2;
    }

    // Only reallocate when max capacity is reached
    if (array->size == array->capacity) {
        size_t new_capacity = (size_t)((float)array->capacity * array->growth_factor);

        void *new_data = NV_REALLOC(
            array->data,
            new_capacity * array->element_size
        );

        if (!new_data) {
            nv_set_error("Failed to reallocate memory.");
            return 1;
        }

        array->capacity = new_capacity;
        array->data = new_data;
    }

    memcpy(
        (char *)array->data + (array->size++) * array->element_size,
        elem,
        array->element_size
    );

    return 0;
}

void nvArray_clear(nvArray *array) {
    array->size = 0;
}

void nvArray_for_each(
    nvArray *array,
    nvArray_for_each_callback callback,
    void *user_data
) {
    for (size_t i = 0; i < array->size; i++) {
        callback(NV_ARRAY_PTR_AT(array, i, void), user_data);
    }
}

nvArray *nvArray_copy(const nvArray *array) {
    nvArray *copy = nvArray_new_ex(array->element_size, array->capacity, array->growth_factor);
    if (!nvArray_valid(copy)) return copy;

    copy->size = array->size;
    for (size_t i = 0; i < array->size; i++) {
        memcpy(
            NV_ARRAY_PTR_AT(copy, i, char),
            NV_ARRAY_PTR_AT(array, i, char),
            array->element_size
        );
    }

    return copy;
}

int nvArray_resize(nvArray *array) {
    if (!array) {
        nv_set_error("Invalid argument, array is NULL.");
        return 2;
    }

    if (array->size == array->capacity) {
        return 0;
    }

    size_t new_capacity = array->size;

    void *new_data = NV_REALLOC(
        array->data,
        new_capacity * array->element_size
    );

    if (!new_data) {
        nv_set_error("Failed to reallocate memory.");
        return 1;
    }

    array->capacity = new_capacity;
    array->data = new_data;

    return 0;
}

size_t nvArray_total_memory_used(const nvArray *array) {
    size_t size = 0;
    if (!nvArray_valid(array)) return size;

    // nvArray
    size += sizeof(nvArray);

    // nvArray->data
    size += array->capacity * array->element_size;

    return size;
}