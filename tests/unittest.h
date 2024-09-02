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
 * @file unittest.h
 * 
 * @brief Header-only unit test suite for Nova Physics.
 * 
 * Maybe I should make this file more generic so people can use it in
 * their projects as well.
 */


/**
 * @brief Unit test suite.
 */
typedef struct {
    char *current; /**< Current function that is being tested. */
    int total; /**< Amount of tests done so far. */
    int fails; /**< Failed tests so far. */
} UnitTestSuite;

/* Update the total of tests done. */
#define UPDATE_TOTAL (test->total++)

/* Update the amount of failed tests. */
#define FAIL (test->fails++)

/* Get the current testing function name. */
#define CURRENT (test->current)

/* Test a function. */
#define TEST(x) {test.current = #x; TEST__##x(&test);}


/**
 * @brief Compare two integers.
 * 
 * @param value Value
 * @param expect Expected value
 * @param test Pointer to UnitTestSuite object
 */
void expect_int(int value, int expect, UnitTestSuite *test) {
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
 * @brief Compare two floats.
 * 
 * @param value Value
 * @param expect Expected value
 * @param test Pointer to UnitTestSuite object
 */
void expect_float(float value, float expect, UnitTestSuite *test) {
    UPDATE_TOTAL;

    if (value == expect)
        printf("[PASSED] %s\n", CURRENT);

    else {
        printf("[FAILED] %s: Expected (float)%f but got (float)%f\n",
                CURRENT, expect, value);
        FAIL;
    }
}

/**
 * @brief Compare two doubles.
 * 
 * @param value Value
 * @param expect Expected value
 * @param test Pointer to UnitTestSuite object
 */
void expect_double(double value, double expect, UnitTestSuite *test) {
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
 * @brief Expect true boolean value.
 * 
 * @param value Value
 * @param test Pointer to UnitTestSuite object
 */
void expect_true(nv_bool value, UnitTestSuite *test) {
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
 * @brief Expect false boolean value.
 * 
 * @param value Value
 * @param test Pointer to UnitTestSuite object
 */
void expect_false(nv_bool value, UnitTestSuite *test) {
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
 * @brief Compare two vectors.
 * 
 * @param value Value
 * @param expect Expected value
 * @param test Pointer to UnitTestSuite object
 */
void expect_Vector2(nvVector2 value, nvVector2 expect, UnitTestSuite *test) {
    UPDATE_TOTAL;

    if (value.x == expect.x && value.y == expect.y)
        printf("[PASSED] %s\n", CURRENT);

    else {
        printf("[FAILED] %s: Expected (nvVector2){%f, %f} but got (nvVector2){%f, %f}\n",
                CURRENT, expect.x, expect.y, value.x, value.y);
        FAIL;
    }
}