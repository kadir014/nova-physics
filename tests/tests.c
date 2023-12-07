/*

  This file is a part of the Nova Physics Engine
  project and distributed under the MIT license.

  Copyright © Kadir Aksoy
  https://github.com/kadir014/nova-physics

*/

#include <stdio.h>
#include <stdbool.h>
#include "novaphysics/novaphysics.h"


/**
 * @brief Test suite.
 */
typedef struct {
    char *current; /**< Current function that is being tested. */
    int total; /**< Amount of tests done so far. */
    int fails; /**< Failed tests so far. */
} TestSuite;

// Helper macros

/* Update the total of tests done. */
#define UPDATE_TOTAL (test->total++)

/* Update the amount of failed tests. */
#define FAIL (test->fails++)

/* Get the current testing function name. */
#define CURRENT (test->current)

/* Test a function. */
#define TEST(x) {test.current = #x;TEST__##x(&test);}


/**
 * @brief Compare two integers and print the test message
 * 
 * @param value Value
 * @param expect Expected value
 * @param test Pointer to TestSuite object
 */
void expect_int(int value, int expect, TestSuite *test) {
    UPDATE_TOTAL;

    if (value == expect)
        printf("[PASSED] %s\n", CURRENT);

    else {
        printf("[FAILED] %s: Expected (int)%d but got (int)%d\n",
                CURRENT, expect, value);
        FAIL;
    }
}

/**
 * @brief Compare two doubles and print the test message
 * 
 * @param value Value
 * @param expect Expected value
 * @param test Pointer to TestSuite object
 */
void expect_double(double value, double expect, TestSuite *test) {
    UPDATE_TOTAL;

    if (value == expect)
        printf("[PASSED] %s\n", CURRENT);

    else {
        printf("[FAILED] %s: Expected (double)%f but got (double)%f\n",
                CURRENT, expect, value);
        FAIL;
    }
}

/**
 * @brief Expect true boolean value
 * 
 * @param value Value
 */
void expect_true(bool value, TestSuite *test) {
    UPDATE_TOTAL;

    if (value) {
        printf("[PASSED] %s\n", CURRENT);
    }
    else {
        printf("[FAILED] %s: Expected true\n", CURRENT);
        FAIL;
    }
}

/**
 * @brief Expect false boolean value
 * 
 * @param value Value
 */
void expect_false(bool value, TestSuite *test) {
    UPDATE_TOTAL;

    if (!value) {
        printf("[PASSED] %s\n", CURRENT);
    }
    else {
        printf("[FAILED] %s: Expected false\n", CURRENT);
        FAIL;
    }
}

/**
 * @brief Compare two vector and print the test message
 * 
 * @param value Value
 * @param expect Expected value
 * @param test Pointer to TestSuite object
 */
void expect_vector(nvVector2 value, nvVector2 expect, TestSuite *test) {
    UPDATE_TOTAL;

    if (value.x == expect.x && value.y == expect.y)
        printf("[PASSED] %s\n", CURRENT);

    else {
        printf("[FAILED] %s: Expected (nvVector2){%f, %f} but got (nvVector2){%f, %f}\n",
                CURRENT, expect.x, expect.y, value.x, value.y);
        FAIL;
    }
}


/*
    nvVector2 tests
*/

void TEST__nvVector2_eq(TestSuite *test) {
    nvVector2 a = NV_VEC2(3.0, 2.0);
    nvVector2 b = NV_VEC2(3.0, 2.1);
    expect_false(nvVector2_eq(a, b), test);
}

void TEST__nvVector2_add(TestSuite *test) {
    nvVector2 a = NV_VEC2(-1.0, 4.5);
    nvVector2 b = NV_VEC2(3.0, 2.1);
    expect_vector(nvVector2_add(a, b), NV_VEC2(2.0, 6.6), test);
}

void TEST__nvVector2_sub(TestSuite *test) {
    nvVector2 a = NV_VEC2(-1.0, 4.5);
    nvVector2 b = NV_VEC2(3.0, 2.1);
    expect_vector(nvVector2_sub(a, b), NV_VEC2(-4.0, 2.4), test);
}

void TEST__nvVector2_mul(TestSuite *test) {
    nvVector2 a = NV_VEC2(-1.0, 4.5);
    double b = 2.46;
    expect_vector(nvVector2_mul(a, b), NV_VEC2(-2.46, 11.07), test);
}

void TEST__nvVector2_div(TestSuite *test) {
    nvVector2 a = NV_VEC2(-1.0, 4.5);
    double b = 2.5;
    expect_vector(nvVector2_div(a, b), NV_VEC2(-0.4, 1.8), test);
}

void TEST__nvVector2_neg(TestSuite *test) {
    nvVector2 a = NV_VEC2(-1.0, 4.5);
    expect_vector(nvVector2_neg(a), NV_VEC2(1.0, -4.5), test);
}

void TEST__nvVector2_rotate(TestSuite *test) {
    nvVector2 a = NV_VEC2(-1.5, 2.0);
    double angle = NV_PI / 4.0;
    expect_vector(nvVector2_rotate(a, angle), NV_VEC2(-2.474874, 0.353553), test);
}

void TEST__nvVector2_perp(TestSuite *test) {
    nvVector2 a = NV_VEC2(-1.5, 2.0);
    expect_vector(nvVector2_perp(a), NV_VEC2(-2.0, -1.5), test);
}

void TEST__nvVector2_perpr(TestSuite *test) {
    nvVector2 a = NV_VEC2(-1.5, 2.0);
    expect_vector(nvVector2_perpr(a), NV_VEC2(2.0, 1.5), test);
}

void TEST__nvVector2_len2(TestSuite *test) {
    nvVector2 a = NV_VEC2(-1.5, 2.0);
    expect_double(nvVector2_len2(a), 6.25, test);
}

void TEST__nvVector2_len(TestSuite *test) {
    nvVector2 a = NV_VEC2(-1.5, 2.0);
    expect_double(nvVector2_len(a), 2.5, test);
}

void TEST__nvVector2_dot(TestSuite *test) {
    nvVector2 a = NV_VEC2(-1.5, 2.0);
    nvVector2 b = NV_VEC2(5.0, 8.5);
    expect_double(nvVector2_dot(a, b), 9.5, test);
}

void TEST__nvVector2_cross(TestSuite *test) {
    nvVector2 a = NV_VEC2(-1.5, 2.0);
    nvVector2 b = NV_VEC2(5.0, 8.5);
    expect_double(nvVector2_cross(a, b), -22.75, test);
}

void TEST__nvVector2_dist2(TestSuite *test) {
    nvVector2 a = NV_VEC2(-1.5, 2.0);
    nvVector2 b = NV_VEC2(5.3, 8.4);
    expect_double(nvVector2_dist2(a, b), 87.2, test);
}

void TEST__nvVector2_dist(TestSuite *test) {
    nvVector2 a = NV_VEC2(-1.5, 2.0);
    nvVector2 b = NV_VEC2(5.0, 8.5);
    expect_double(nvVector2_dist(a, b), 9.19239, test);
}

void TEST__nvVector2_normalize(TestSuite *test) {
    nvVector2 a = NV_VEC2(-1.2, 4.5);
    expect_vector(nvVector2_normalize(a), NV_VEC2(-0.257663, 0.966235), test);
}


/*
    nvArray tests
*/

void TEST__nvArray_add(TestSuite *test) {
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

void TEST__nvArray_pop(TestSuite *test) {
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

void TEST__nvArray_remove(TestSuite *test) {
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


int main() {
    TestSuite test = {.current = "", .total = 0, .fails = 0};

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

    return 0;
}