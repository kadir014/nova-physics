/*

  This file is a part of the Nova Physics Engine
  project and distributed under the MIT license.

  Copyright © Kadir Aksoy
  https://github.com/kadir014/nova-physics

*/

#include "tinytest/tinytest.h"
#include "novaphysics/novaphysics.h"


void test_nvArray_new(ttUnitTestSuite *test) {
    {
        nvArray *array = nvArray_new(sizeof(int));
        tt_expect(nvArray_valid(array), test);
        tt_expect_size_t(array->size, 0, test);
        tt_expect_size_t(array->capacity, 1, test);
        tt_expect_float(array->growth_factor, 2.0f, test);
        nvArray_free(array);
    }
    {
        nvArray *array = nvArray_new_ex(sizeof(int), 5, 1.65f);
        tt_expect(nvArray_valid(array), test);
        tt_expect_size_t(array->size, 0, test);
        tt_expect_size_t(array->capacity, 5, test);
        tt_expect_float(array->growth_factor, 1.65f, test);
        nvArray_free(array);
    }
}

void test_nvArray_add(ttUnitTestSuite *test) {
    nvArray *array = nvArray_new(sizeof(int));

    int a = 42;
    nvArray_add(array, &a);

    tt_expect_int(NV_ARRAY_AT(array, 0, int), 42, test);
    tt_expect_size_t(array->capacity, 1, test);
    tt_expect_size_t(array->size, 1, test);

    nvArray_free(array);
}

void test_nvArray_pop(ttUnitTestSuite *test) {
    nvArray *array = nvArray_new(sizeof(int));

    int a = 42;
    nvArray_add(array, &a);

    int b = 69;
    nvArray_add(array, &b);

    tt_expect(nvArray_pop(array, 0), test);
    tt_expect_int(NV_ARRAY_AT(array, 0, int), 69, test);
    tt_expect_size_t(array->capacity, 2, test);
    tt_expect_size_t(array->size, 1, test);

    nvArray_free(array);
}

void test_nvArray_clear(ttUnitTestSuite *test) {
    nvArray *array = nvArray_new(sizeof(int));

    int a = 42;
    nvArray_add(array, &a);

    int b = 69;
    nvArray_add(array, &b);

    nvArray_clear(array);

    tt_expect_size_t(array->capacity, 2, test);
    tt_expect_size_t(array->size, 0, test);

    nvArray_free(array);
}

void test_nvArray_copy(ttUnitTestSuite *test) {
    nvArray *array = nvArray_new(sizeof(int));

    int a = 42;
    nvArray_add(array, &a);

    int b = 69;
    nvArray_add(array, &b);

    nvArray *copy = nvArray_copy(array);

    tt_expect(nvArray_valid(copy), test);
    tt_expect_int(NV_ARRAY_AT(array, 0, int), NV_ARRAY_AT(copy, 0, int), test);
    tt_expect_int(NV_ARRAY_AT(array, 1, int), NV_ARRAY_AT(copy, 1, int), test);

    nvArray_free(array);
    nvArray_free(copy);
}

void test_nvArray_resize(ttUnitTestSuite *test) {
    nvArray *array = nvArray_new(sizeof(int));

    array->size = 31;
    nvArray_resize(array);

    tt_expect_size_t(array->capacity, 31, test);
    tt_expect_size_t(array->size, 31, test);

    nvArray_free(array);
}


void run_nvArray_tests(ttUnitTestSuite *test) {
    TT_RUN_TEST_P(test_nvArray_new);
    TT_RUN_TEST_P(test_nvArray_add);
    TT_RUN_TEST_P(test_nvArray_pop);
    TT_RUN_TEST_P(test_nvArray_clear);
    TT_RUN_TEST_P(test_nvArray_copy);
    TT_RUN_TEST_P(test_nvArray_resize);
}