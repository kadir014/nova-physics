/*

  This file is a part of the Nova Physics Engine
  project and distributed under the MIT license.

  Copyright © Kadir Aksoy
  https://github.com/kadir014/nova-physics

*/

#include "tinytest/tinytest.h"
#include "novaphysics/novaphysics.h"


void test_nvRefArray_new(ttUnitTestSuite *test) {
    {
        nvRefArray *refarray = nvRefArray_new();
        tt_expect(nvRefArray_valid(refarray), test);
        tt_expect_size_t(refarray->size, 0, test);
        tt_expect_size_t(refarray->capacity, 1, test);
        tt_expect_float(refarray->growth_factor, 2.0f, test);
        nvRefArray_free(refarray);
    }
    {
        nvRefArray *refarray = nvRefArray_new_ex(5, 1.65f);
        tt_expect(nvRefArray_valid(refarray), test);
        tt_expect_size_t(refarray->size, 0, test);
        tt_expect_size_t(refarray->capacity, 5, test);
        tt_expect_float(refarray->growth_factor, 1.65f, test);
        nvRefArray_free(refarray);
    }
}

void test_nvRefArray_add(ttUnitTestSuite *test) {
    nvRefArray *refarray = nvRefArray_new();

    int a = 42;
    nvRefArray_add(refarray, &a);

    tt_expect_int(*(int *)refarray->data[0], 42, test);
    tt_expect_size_t(refarray->capacity, 1, test);
    tt_expect_size_t(refarray->size, 1, test);

    nvRefArray_free(refarray);
}

void test_nvRefArray_pop(ttUnitTestSuite *test) {
    nvRefArray *refarray = nvRefArray_new();

    int a = 42;
    nvRefArray_add(refarray, &a);

    int b = 69;
    nvRefArray_add(refarray, &b);

    int c = *(int *)nvRefArray_pop(refarray, 0);

    tt_expect_int(c, 42, test);
    tt_expect_int(*(int *)refarray->data[0], 69, test);
    tt_expect_size_t(refarray->capacity, 2, test);
    tt_expect_size_t(refarray->size, 1, test);

    nvRefArray_free(refarray);
}

void test_nvRefArray_remove(ttUnitTestSuite *test) {
    nvRefArray *refarray = nvRefArray_new();

    int a = 42;
    nvRefArray_add(refarray, &a);

    int b = 69;
    nvRefArray_add(refarray, &b);

    size_t idx = nvRefArray_remove(refarray, &a);

    tt_expect_size_t(idx, 0, test);
    tt_expect_int(*(int *)refarray->data[0], 69, test);
    tt_expect_size_t(refarray->capacity, 2, test);
    tt_expect_size_t(refarray->size, 1, test);

    size_t wrong_idx = nvRefArray_remove(refarray, &a);
    tt_expect_size_t(wrong_idx, NV_INVALID_INDEX_Z, test);

    nvRefArray_free(refarray);
}

void test_nvRefArray_clear(ttUnitTestSuite *test) {
    nvRefArray *refarray = nvRefArray_new();

    int a = 42;
    nvRefArray_add(refarray, &a);

    int b = 69;
    nvRefArray_add(refarray, &b);

    nvRefArray_clear(refarray, NULL);

    tt_expect_size_t(refarray->capacity, 2, test);
    tt_expect_size_t(refarray->size, 0, test);

    nvRefArray_free(refarray);
}

void test_nvRefArray_copy(ttUnitTestSuite *test) {
    nvRefArray *refarray = nvRefArray_new();

    int a = 42;
    nvRefArray_add(refarray, &a);

    int b = 69;
    nvRefArray_add(refarray, &b);

    nvRefArray *copy = nvRefArray_copy(refarray);

    tt_expect_p(refarray->data[0], copy->data[0], test);
    tt_expect_p(refarray->data[1], copy->data[1], test);

    nvRefArray_free(refarray);
    nvRefArray_free(copy);
}

void test_nvRefArray_resize(ttUnitTestSuite *test) {
    nvRefArray *refarray = nvRefArray_new();

    refarray->size = 31;
    nvRefArray_resize(refarray);

    tt_expect(nvRefArray_valid(refarray), test);
    tt_expect_size_t(refarray->capacity, 31, test);
    tt_expect_size_t(refarray->size, 31, test);

    nvRefArray_free(refarray);
}


void run_nvRefArray_tests(ttUnitTestSuite *test) {
    TT_RUN_TEST_P(test_nvRefArray_new);
    TT_RUN_TEST_P(test_nvRefArray_add);
    TT_RUN_TEST_P(test_nvRefArray_pop);
    TT_RUN_TEST_P(test_nvRefArray_remove);
    TT_RUN_TEST_P(test_nvRefArray_clear);
    TT_RUN_TEST_P(test_nvRefArray_copy);
    TT_RUN_TEST_P(test_nvRefArray_resize);
}