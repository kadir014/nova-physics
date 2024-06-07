/*

  This file is a part of the Nova Physics Engine
  project and distributed under the MIT license.

  Copyright © Kadir Aksoy
  https://github.com/kadir014/nova-physics

*/

#include "unittest.h"


/******************************************************************************

                                nvVector2 tests
    
******************************************************************************/

void TEST__nvVector2_eq(UnitTestSuite *test) {
    nvVector2 a = NV_VECTOR2(3.0, 2.0);
    nvVector2 b = NV_VECTOR2(3.0, 2.1);
    expect_false(nvVector2_eq(a, b), test);
}

void TEST__nvVector2_add(UnitTestSuite *test) {
    nvVector2 a = NV_VECTOR2(-1.0, 4.5);
    nvVector2 b = NV_VECTOR2(3.0, 2.1);
    expect_Vector2(nvVector2_add(a, b), NV_VECTOR2(2.0, 6.6), test);
}

void TEST__nvVector2_sub(UnitTestSuite *test) {
    nvVector2 a = NV_VECTOR2(-1.0, 4.5);
    nvVector2 b = NV_VECTOR2(3.0, 2.1);
    expect_Vector2(nvVector2_sub(a, b), NV_VECTOR2(-4.0, 2.4), test);
}

void TEST__nvVector2_mul(UnitTestSuite *test) {
    nvVector2 a = NV_VECTOR2(-1.0, 4.5);
    double b = 2.46;
    expect_Vector2(nvVector2_mul(a, b), NV_VECTOR2(-2.46, 11.07), test);
}

void TEST__nvVector2_div(UnitTestSuite *test) {
    nvVector2 a = NV_VECTOR2(-1.0, 4.5);
    double b = 2.5;
    expect_Vector2(nvVector2_div(a, b), NV_VECTOR2(-0.4, 1.8), test);
}

void TEST__nvVector2_neg(UnitTestSuite *test) {
    nvVector2 a = NV_VECTOR2(-1.0, 4.5);
    expect_Vector2(nvVector2_neg(a), NV_VECTOR2(1.0, -4.5), test);
}

void TEST__nvVector2_rotate(UnitTestSuite *test) {
    nvVector2 a = NV_VECTOR2(-1.5, 2.0);
    double angle = NV_PI / 4.0;
    expect_Vector2(nvVector2_rotate(a, angle), NV_VECTOR2(-2.474874, 0.353553), test);
}

void TEST__nvVector2_perp(UnitTestSuite *test) {
    nvVector2 a = NV_VECTOR2(-1.5, 2.0);
    expect_Vector2(nvVector2_perp(a), NV_VECTOR2(-2.0, -1.5), test);
}

void TEST__nvVector2_perpr(UnitTestSuite *test) {
    nvVector2 a = NV_VECTOR2(-1.5, 2.0);
    expect_Vector2(nvVector2_perpr(a), NV_VECTOR2(2.0, 1.5), test);
}

void TEST__nvVector2_len2(UnitTestSuite *test) {
    nvVector2 a = NV_VECTOR2(-1.5, 2.0);
    expect_double(nvVector2_len2(a), 6.25, test);
}

void TEST__nvVector2_len(UnitTestSuite *test) {
    nvVector2 a = NV_VECTOR2(-1.5, 2.0);
    expect_double(nvVector2_len(a), 2.5, test);
}

void TEST__nvVector2_dot(UnitTestSuite *test) {
    nvVector2 a = NV_VECTOR2(-1.5, 2.0);
    nvVector2 b = NV_VECTOR2(5.0, 8.5);
    expect_double(nvVector2_dot(a, b), 9.5, test);
}

void TEST__nvVector2_cross(UnitTestSuite *test) {
    nvVector2 a = NV_VECTOR2(-1.5, 2.0);
    nvVector2 b = NV_VECTOR2(5.0, 8.5);
    expect_double(nvVector2_cross(a, b), -22.75, test);
}

void TEST__nvVector2_dist2(UnitTestSuite *test) {
    nvVector2 a = NV_VECTOR2(-1.5, 2.0);
    nvVector2 b = NV_VECTOR2(5.3, 8.4);
    expect_double(nvVector2_dist2(a, b), 87.2, test);
}

void TEST__nvVector2_dist(UnitTestSuite *test) {
    nvVector2 a = NV_VECTOR2(-1.5, 2.0);
    nvVector2 b = NV_VECTOR2(5.0, 8.5);
    expect_double(nvVector2_dist(a, b), 9.19239, test);
}

void TEST__nvVector2_normalize(UnitTestSuite *test) {
    nvVector2 a = NV_VECTOR2(-1.2, 4.5);
    expect_Vector2(nvVector2_normalize(a), NV_VECTOR2(-0.257663, 0.966235), test);
}


/******************************************************************************

                                 nvArray tests
    
******************************************************************************/

void TEST__nvArray_add(UnitTestSuite *test) {
    nvArray *array = nvArray_new();

    double a = 2.0;
    double b = -1.6;
    double c = 14.25;

    nvArray_add(array, &a);
    nvArray_add(array, &b);
    nvArray_add(array, &c);

    expect_true(
        *(double *)array->data[0] == a &&
        *(double *)array->data[1] == b &&
        *(double *)array->data[2] == c &&
        array->size == 3,
        test
    );

    nvArray_free(array);
}

void TEST__nvArray_pop(UnitTestSuite *test) {
    nvArray *array = nvArray_new();

    double a = 2.0;
    double b = -1.6;
    double c = 14.25;

    nvArray_add(array, &a);
    nvArray_add(array, &b);
    nvArray_add(array, &c);

    nvArray_pop(array, 2);

    expect_true(
        *(double *)array->data[0] == a &&
        *(double *)array->data[1] == b &&
        array->size == 2,
        test
    );

    nvArray_free(array);
}

void TEST__nvArray_remove(UnitTestSuite *test) {
    nvArray *array = nvArray_new();

    double a = 2.0;
    double b = -1.6;
    double c = 14.25;

    nvArray_add(array, &a);
    nvArray_add(array, &b);
    nvArray_add(array, &c);

    nvArray_remove(array, &c);

    expect_true(
        *(double *)array->data[0] == a &&
        *(double *)array->data[1] == b &&
        array->size == 2,
        test
    );

    nvArray_free(array);
}


int main(int argc, char *argv[]) {
    UnitTestSuite test = {.current = "", .total = 0, .fails = 0};

    TEST(nvVector2_eq)
    TEST(nvVector2_add)
    TEST(nvVector2_sub)
    TEST(nvVector2_mul)
    TEST(nvVector2_div)
    TEST(nvVector2_neg)
    TEST(nvVector2_rotate)
    TEST(nvVector2_perp)
    TEST(nvVector2_perpr)
    TEST(nvVector2_len2)
    TEST(nvVector2_len)
    TEST(nvVector2_dot)
    TEST(nvVector2_cross)
    TEST(nvVector2_dist2)
    TEST(nvVector2_dist)
    TEST(nvVector2_normalize)

    TEST(nvArray_add)
    TEST(nvArray_pop)
    TEST(nvArray_remove)

    printf("total: %d\n", test.total);
    printf("fails: %d\n", test.fails);

    return EXIT_SUCCESS;
}