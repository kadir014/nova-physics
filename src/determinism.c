/*

  This file is a part of the Nova Physics Engine
  project and distributed under the MIT license.

  Copyright © Kadir Aksoy
  https://github.com/kadir014/nova-physics

*/

#include "novaphysics/determinism.h"
#include "novaphysics/space.h"


/**
 * @file determinism.c
 *
 * @brief Same-machine determinism helpers.
 */


// At which number to choose between quicksort and insertion sort
// TODO: Move to constants.h?
#define SORT_INSERTION_THRESHOLD 16


static inline void swap(nvSortedContactPair *arr, size_t i, size_t j) {
    nvSortedContactPair tmp = arr[i];
    arr[i] = arr[j];
    arr[j] = tmp;
}

static inline void insertion_sort(nvSortedContactPair *arr, size_t lo, size_t hi) {
    for (size_t i = lo + 1; i < hi; i++) {
        nvSortedContactPair item = arr[i];

        size_t j = i;
        while (j > lo && arr[j - 1].id > item.id) {
            arr[j] = arr[j - 1];
            j--;
        }

        arr[j] = item;
    }
}

static inline size_t quicksort_partition(
    nvSortedContactPair *arr,
    size_t lo,
    size_t hi
) {
    // Median of three
    // leaves the median of (lo, mid, hi-1) at hi-1 as the pivot
    size_t mid = lo + (hi - lo) / 2;
    if (arr[mid].id < arr[lo].id) {
        swap(arr, mid, lo);
    }
    if (arr[hi - 1].id < arr[lo].id) {
        swap(arr, hi - 1, lo);
    }
    if (arr[mid].id < arr[hi - 1].id) {
        swap(arr, mid, hi - 1);
    }

    nv_uint64 pivot = arr[hi - 1].id;
    size_t i = lo;

    for (size_t j = lo; j < hi - 1; j++) {
        if (arr[j].id < pivot) {
            swap(arr, i, j);
            i++;
        }
    }

    swap(arr, i, hi - 1);
    return i;
}

static inline void quicksort(nvSortedContactPair *arr, size_t lo, size_t hi) {
    while (hi - lo > SORT_INSERTION_THRESHOLD) {
        size_t p = quicksort_partition(arr, lo, hi);

        // Recurse into the before and after of partition
        if (p - lo < hi - (p + 1)) {
            quicksort(arr, lo, p);
            lo = p + 1;
        }
        else {
            quicksort(arr, p + 1, hi);
            hi = p;
        }
    }

    insertion_sort(arr, lo, hi);
}

int nv_sort_contacts(nvSpace *space) {
    if (!space) return 0;

    size_t n = space->sorted_contacts->size;
    if (n <= 1) return 0;

    // Cast the dense array to avoid NV_ARRAY_AT calculations (one multiplication per lookup)
    nvSortedContactPair *arr = (nvSortedContactPair *)space->sorted_contacts->data;
    quicksort(arr, 0, n);

    return 0;
}