/*

  This file is a part of the Nova Physics Engine
  project and distributed under the MIT license.

  Copyright © Kadir Aksoy
  https://github.com/kadir014/nova-physics

*/

#include "tinytest/tinytest.h"
#include "novaphysics/novaphysics.h"

#include "units/refarray.h"


/**
 * @file tests/main.c
 * 
 * @brief Nova unit tests entry point.
 */


int main(int argc, char *argv[]) {
    ttUnitTestSuite test = {0};
    test.colored_output = true;

    run_nvRefArray_tests(&test);

    tt_print_report(&test);

    nv_check_leaks();

    return 0;
}