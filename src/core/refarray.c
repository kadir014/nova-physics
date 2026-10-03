/*

  This file is a part of the Nova Physics Engine
  project and distributed under the MIT license.

  Copyright © Kadir Aksoy
  https://github.com/kadir014/nova-physics

*/

#include "novaphysics/core/refarray.h"


/**
 * @file core/refarray.c
 * 
 * @brief Dynamically growing type-generic reference array.
 */


nvRefArray *nvRefArray_new() {
    return nvRefArray_new_ex(1, 2.0f);
}

nvRefArray *nvRefArray_new_ex(size_t default_capacity, float growth_factor) {
    nvRefArray *refarray = NV_NEW(nvRefArray);
    NV_MEM_CHECK(refarray);

    refarray->size = 0;
    refarray->capacity = default_capacity;
    refarray->growth_factor = growth_factor;
    refarray->data = NULL;

    if (growth_factor <= 1.0f || default_capacity == 0) {
        return refarray;
    }

    refarray->data = NV_MALLOC(sizeof(void *) * default_capacity);

    /*
        The allocated data is not zeroed out for performance reasons.
        The usable space in the array is set whenever add function is used, rest
        must be assumed not usable.
    */

    return refarray;
}

void nvRefArray_free(nvRefArray *refarray) {
    if (!refarray) {
        return;
    }

    NV_FREE(refarray->data);
    NV_FREE(refarray);
}

nv_bool nvRefArray_valid(const nvRefArray *refarray) {
    return !(
        !refarray ||
        !refarray->data ||
        refarray->growth_factor <= 1.0f ||
        refarray->capacity == 0 ||
        refarray->size > refarray->capacity
    );
}

int nvRefArray_add(nvRefArray *refarray, void *elem) {
    if (!refarray || !elem) {
        return 2;
    }

    // Only reallocate when max capacity is reached
    if (refarray->size == refarray->capacity) {
        size_t new_capacity = (size_t)((float)refarray->capacity * refarray->growth_factor);

        void **new_data = NV_REALLOC(
            refarray->data,
            new_capacity * sizeof(void *)
        );

        if (!new_data) {
            return 1;
        }

        refarray->capacity = new_capacity;
        refarray->data = new_data;
    }

    refarray->data[refarray->size++] = elem;

    return 0;
}

void *nvRefArray_pop(nvRefArray *refarray, size_t index) {
    if (refarray->size == 0 || index >= refarray->size) {
        return NULL;
    }

    void *elem = refarray->data[index];

    // Shift everything after index left by one position.
    if (index < refarray->size - 1) {
        memmove(
            &refarray->data[index],
            &refarray->data[index + 1],
            (refarray->size - index - 1) * sizeof(void *)
        );
    }

    refarray->size--;
    refarray->data[refarray->size] = NULL;

    return elem;
}

size_t nvRefArray_remove(nvRefArray *refarray, void *elem) {
    size_t index = NV_INVALID_INDEX_Z;
    for (size_t i = 0; i < refarray->size; i++) {
        if (refarray->data[i] == elem) {
            index = i;
            break;
        }
    }

    if (index == NV_INVALID_INDEX_Z) {
        return index;
    }

    if (!nvRefArray_pop(refarray, index)) {
        return NV_INVALID_INDEX_Z;
    }
    else {
        return index;
    }
}

void nvRefArray_clear(nvRefArray *refarray, void (free_func)(void *)) {
    if (refarray->size == 0) {
        return;
    }

    if (free_func) {
        for (size_t i = 0; i < refarray->size; i++) {
            if (refarray->data[i]) {
                free_func(refarray->data[i]);
            }
        }
    }

    refarray->size = 0;
}

void nvRefArray_for_each(
    nvRefArray *refarray,
    nvRefArray_for_each_callback callback,
    void *user_data
) {
    for (size_t i = 0; i < refarray->size; i++) {
        callback(refarray->data[i], user_data);
    }
}

nvRefArray *nvRefArray_copy(const nvRefArray *refarray) {
    nvRefArray *copy = nvRefArray_new_ex(refarray->capacity, refarray->growth_factor);
    if (!nvRefArray_valid(copy)) return copy;

    copy->size = refarray->size;
    for (size_t i = 0; i < refarray->size; i++) {
        copy->data[i] = refarray->data[i];
    }

    return copy;
}

int nvRefArray_resize(nvRefArray *refarray) {
    if (!refarray) {
        return 2;
    }

    if (refarray->size == refarray->capacity) {
        return 0;
    }

    size_t new_capacity = refarray->size;

    void **new_data = NV_REALLOC(
        refarray->data,
        new_capacity * sizeof(void *)
    );

    if (!new_data) {
        return 1;
    }

    refarray->capacity = new_capacity;
    refarray->data = new_data;

    return 0;
}

size_t nvRefArray_total_memory_used(const nvRefArray *refarray) {
    size_t size = 0;
    if (!nvRefArray_valid(refarray)) return size;

    // nvRefArray
    size += sizeof(nvRefArray);

    // nvRefArray->data
    size += refarray->capacity * sizeof(void *);

    return size;
}