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
 * Test suite for Nova Physics Engine
 * 
 * Use run_tests.py script to run tests
 */


/**
 * @brief TestSuite struct
 * 
 * @param current Current function being tested
 * @param total Amount of tests done so far
 * @param fails Failed tests so far
 */
typedef struct {
    char *current;
    int total;
    int fails;
} TestSuite;

// Helper macros
#define TOTAL (test->total++);
#define FAIL (test->fails++);
#define CURRENT (test->current)
#define TEST(x) {test.current = #x;TEST__##x(&test);}


/**
 * @brief Compare two integers and print the test message
 * 
 * @param value Value
 * @param expect Expected value
 * @param test Pointer to TestSuite object
 */
void expect_int(int value, int expect, TestSuite *test) {
    TOTAL

    if (value == expect)
        printf("[PASSED] %s\n", CURRENT);

    else {
        printf("[FAILED] %s: Expected (int)%d but got (int)%d\n",
                CURRENT, expect, value);
        FAIL
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
    TOTAL

    if (value == expect)
        printf("[PASSED] %s\n", CURRENT);

    else {
        printf("[FAILED] %s: Expected (double)%f but got (double)%f\n",
                CURRENT, expect, value);
        FAIL
    }
}

/**
 * @brief Expect true boolean value
 * 
 * @param value Value
 */
void expect_true(bool value, TestSuite *test) {
    TOTAL

    if (value) {
        printf("[PASSED] %s\n", CURRENT);
    }
    else {
        printf("[FAILED] %s: Expected true\n", CURRENT);
        FAIL
    }
}

/**
 * @brief Expect false boolean value
 * 
 * @param value Value
 */
void expect_false(bool value, TestSuite *test) {
    TOTAL

    if (!value) {
        printf("[PASSED] %s\n", CURRENT);
    }
    else {
        printf("[FAILED] %s: Expected false\n", CURRENT);
        FAIL
    }
}

/**
 * @brief Compare two vector and print the test message
 * 
 * @param value Value
 * @param expect Expected value
 * @param test Pointer to TestSuite object
 */
void expect_vector(nv_Vector2 value, nv_Vector2 expect, TestSuite *test) {
    TOTAL

    if (value.x == expect.x && value.y == expect.y)
        printf("[PASSED] %s\n", CURRENT);

    else {
        printf("[FAILED] %s: Expected (nv_Vector2){%f, %f} but got (nv_Vector2){%f, %f}\n",
                CURRENT, expect.x, expect.y, value.x, value.y);
        FAIL
    }
}


/*
    nv_Vector2 tests
*/

void TEST__nv_Vector2_eq(TestSuite *test) {
    nv_Vector2 a = NV_VEC2(3.0, 2.0);
    nv_Vector2 b = NV_VEC2(3.0, 2.1);
    expect_false(nv_Vector2_eq(a, b), test);
}

void TEST__nv_Vector2_add(TestSuite *test) {
    nv_Vector2 a = NV_VEC2(-1.0, 4.5);
    nv_Vector2 b = NV_VEC2(3.0, 2.1);
    expect_vector(nv_Vector2_add(a, b), NV_VEC2(2.0, 6.6), test);
}

void TEST__nv_Vector2_sub(TestSuite *test) {
    nv_Vector2 a = NV_VEC2(-1.0, 4.5);
    nv_Vector2 b = NV_VEC2(3.0, 2.1);
    expect_vector(nv_Vector2_sub(a, b), NV_VEC2(-4.0, 2.4), test);
}

void TEST__nv_Vector2_muls(TestSuite *test) {
    nv_Vector2 a = NV_VEC2(-1.0, 4.5);
    double b = 2.46;
    expect_vector(nv_Vector2_muls(a, b), NV_VEC2(-2.46, 11.07), test);
}

void TEST__nv_Vector2_divs(TestSuite *test) {
    nv_Vector2 a = NV_VEC2(-1.0, 4.5);
    double b = 2.5;
    expect_vector(nv_Vector2_divs(a, b), NV_VEC2(-0.4, 1.8), test);
}

void TEST__nv_Vector2_neg(TestSuite *test) {
    nv_Vector2 a = NV_VEC2(-1.0, 4.5);
    expect_vector(nv_Vector2_neg(a), NV_VEC2(1.0, -4.5), test);
}

void TEST__nv_Vector2_rotate(TestSuite *test) {
    nv_Vector2 a = NV_VEC2(-1.5, 2.0);
    double angle = NV_PI / 4.0;
    expect_vector(nv_Vector2_rotate(a, angle), NV_VEC2(-2.474874, 0.353553), test);
}

void TEST__nv_Vector2_perp(TestSuite *test) {
    nv_Vector2 a = NV_VEC2(-1.5, 2.0);
    expect_vector(nv_Vector2_perp(a), NV_VEC2(-2.0, -1.5), test);
}

void TEST__nv_Vector2_perpr(TestSuite *test) {
    nv_Vector2 a = NV_VEC2(-1.5, 2.0);
    expect_vector(nv_Vector2_perpr(a), NV_VEC2(2.0, 1.5), test);
}

void TEST__nv_Vector2_len2(TestSuite *test) {
    nv_Vector2 a = NV_VEC2(-1.5, 2.0);
    expect_double(nv_Vector2_len2(a), 6.25, test);
}

void TEST__nv_Vector2_len(TestSuite *test) {
    nv_Vector2 a = NV_VEC2(-1.5, 2.0);
    expect_double(nv_Vector2_len(a), 2.5, test);
}

void TEST__nv_Vector2_dot(TestSuite *test) {
    nv_Vector2 a = NV_VEC2(-1.5, 2.0);
    nv_Vector2 b = NV_VEC2(5.0, 8.5);
    expect_double(nv_Vector2_dot(a, b), 9.5, test);
}

void TEST__nv_Vector2_cross(TestSuite *test) {
    nv_Vector2 a = NV_VEC2(-1.5, 2.0);
    nv_Vector2 b = NV_VEC2(5.0, 8.5);
    expect_double(nv_Vector2_cross(a, b), -22.75, test);
}

void TEST__nv_Vector2_dist2(TestSuite *test) {
    nv_Vector2 a = NV_VEC2(-1.5, 2.0);
    nv_Vector2 b = NV_VEC2(5.3, 8.4);
    expect_double(nv_Vector2_dist2(a, b), 87.2, test);
}

void TEST__nv_Vector2_dist(TestSuite *test) {
    nv_Vector2 a = NV_VEC2(-1.5, 2.0);
    nv_Vector2 b = NV_VEC2(5.0, 8.5);
    expect_double(nv_Vector2_dist(a, b), 9.19239, test);
}

void TEST__nv_Vector2_normalize(TestSuite *test) {
    nv_Vector2 a = NV_VEC2(-1.2, 4.5);
    expect_vector(nv_Vector2_normalize(a), NV_VEC2(-0.257663, 0.966235), test);
}


/*
    nv_Array tests
*/

void TEST__nv_Array_add(TestSuite *test) {
    nv_Array *array = nv_Array_new();

    double a = 2.0;
    double b = -1.6;
    double c = 14.25;

    nv_Array_add(array, &a);
    nv_Array_add(array, &b);
    nv_Array_add(array, &c);

    expect_true(
        *(double *)array->data[0] == a &&
        *(double *)array->data[1] == b &&
        *(double *)array->data[2] == c &&
        array->size == 3,
        test
    );

    nv_Array_free(array);
}

void TEST__nv_Array_pop(TestSuite *test) {
    nv_Array *array = nv_Array_new();

    double a = 2.0;
    double b = -1.6;
    double c = 14.25;

    nv_Array_add(array, &a);
    nv_Array_add(array, &b);
    nv_Array_add(array, &c);

    nv_Array_pop(array, 2);

    expect_true(
        *(double *)array->data[0] == a &&
        *(double *)array->data[1] == b &&
        array->size == 2,
        test
    );

    nv_Array_free(array);
}

void TEST__nv_Array_remove(TestSuite *test) {
    nv_Array *array = nv_Array_new();

    double a = 2.0;
    double b = -1.6;
    double c = 14.25;

    nv_Array_add(array, &a);
    nv_Array_add(array, &b);
    nv_Array_add(array, &c);

    nv_Array_remove(array, &c);

    expect_true(
        *(double *)array->data[0] == a &&
        *(double *)array->data[1] == b &&
        array->size == 2,
        test
    );

    nv_Array_free(array);
}


int main() {
    TestSuite test = {.current = "", .total = 0, .fails = 0};

    TEST(nv_Vector2_eq)
    TEST(nv_Vector2_add)
    TEST(nv_Vector2_sub)
    TEST(nv_Vector2_muls)
    TEST(nv_Vector2_divs)
    TEST(nv_Vector2_neg)
    TEST(nv_Vector2_rotate)
    TEST(nv_Vector2_perp)
    TEST(nv_Vector2_perpr)
    TEST(nv_Vector2_len2)
    TEST(nv_Vector2_len)
    TEST(nv_Vector2_dot)
    TEST(nv_Vector2_cross)
    TEST(nv_Vector2_dist2)
    TEST(nv_Vector2_dist)
    TEST(nv_Vector2_normalize)

    TEST(nv_Array_add)
    TEST(nv_Array_pop)
    TEST(nv_Array_remove)

    printf("total: %d\n", test.total);
    printf("fails: %d\n", test.fails);

    return 0;
}