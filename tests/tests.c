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
 * run_tests.py script to run tests
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


/**
 * nv_Vector2 tests
 */

void TEST__nv_Vector2_eq(TestSuite *test) {
    nv_Vector2 a = (nv_Vector2){3.0, 2.0};
    nv_Vector2 b = (nv_Vector2){3.0, 2.1};
    expect_false(nv_Vector2_eq(a, b), test);
}

void TEST__nv_Vector2_add(TestSuite *test) {
    nv_Vector2 a = (nv_Vector2){-1.0, 4.5};
    nv_Vector2 b = (nv_Vector2){3.0, 2.1};
    expect_vector(nv_Vector2_add(a, b), (nv_Vector2){2.5, 6.6}, test);
}

void TEST__nv_Vector2_sub(TestSuite *test) {
    nv_Vector2 a = (nv_Vector2){-1.0, 4.5};
    nv_Vector2 b = (nv_Vector2){3.0, 2.1};
    expect_vector(nv_Vector2_sub(a, b), (nv_Vector2){-2.0, 2.4}, test);
}

void TEST__nv_Vector2_mulv(TestSuite *test) {
    nv_Vector2 a = (nv_Vector2){-1.0, 4.5};
    nv_Vector2 b = (nv_Vector2){3.0, 2.1};
    expect_vector(nv_Vector2_mulv(a, b), (nv_Vector2){-3.0, 9.45}, test);
}

void TEST__nv_Vector2_muls(TestSuite *test) {
    nv_Vector2 a = (nv_Vector2){-1.0, 4.5};
    double b = 2.46;
    expect_vector(nv_Vector2_muls(a, b), (nv_Vector2){-2.46, 11.07}, test);
}

void TEST__nv_Vector2_divv(TestSuite *test) {
    nv_Vector2 a = (nv_Vector2){-6.0, 5.0};
    nv_Vector2 b = (nv_Vector2){3.0, 2.5};
    expect_vector(nv_Vector2_divv(a, b), (nv_Vector2){-2.0, 2.0}, test);
}

void TEST__nv_Vector2_divs(TestSuite *test) {
    nv_Vector2 a = (nv_Vector2){-1.0, 4.5};
    double b = 2.5;
    expect_vector(nv_Vector2_divs(a, b), (nv_Vector2){-0.4, 1.8}, test);
}

void TEST__nv_Vector2_neg(TestSuite *test) {
    nv_Vector2 a = (nv_Vector2){-1.0, 4.5};
    expect_vector(nv_Vector2_neg(a), (nv_Vector2){1.0, -4.5}, test);
}

void TEST__nv_Vector2_normalize(TestSuite *test) {
    nv_Vector2 a = (nv_Vector2){-1.2, 4.5};
    expect_vector(nv_Vector2_normalize(a), (nv_Vector2){1.0, -4.5}, test);
}


int main() {
    TestSuite test = {.current = "", .total = 0, .fails = 0};

    TEST(nv_Vector2_eq)
    TEST(nv_Vector2_add)
    TEST(nv_Vector2_sub)
    TEST(nv_Vector2_mulv)
    TEST(nv_Vector2_muls)
    TEST(nv_Vector2_divv)
    TEST(nv_Vector2_divs)
    TEST(nv_Vector2_neg)
    TEST(nv_Vector2_normalize)

    printf("total: %d\n", test.total);
    printf("fails: %d\n", test.fails);

    return 0;
}