/*

  This file is a part of the Nova Physics Engine
  project and distributed under the MIT license.

  Copyright © Kadir Aksoy
  https://github.com/kadir014/nova-physics

*/

#include "tinytest/tinytest.h"
#include "novaphysics/novaphysics.h"

#include "random.h"

#include "units/refarray.h"
#include "units/array.h"
#include "units/hashmap.h"
#include "units/shape.h"


/**
 * @file tests/main.c
 * 
 * @brief Nova unit tests entry point.
 */


int main(int argc, char *argv[]) {
    unsigned int seed = nv_init_rand();

    ttUnitTestSuite test = {0};
    test.colored_output = true;

    run_nvRefArray_tests(&test);
    run_nvArray_tests(&test);
    run_nvHashMap_tests(&test);
    run_nvShape_tests(&test);

    tt_print_report(&test);

    nv_check_leaks();
    printf("Random was seeded with %u\n", seed);

    return test.fails != 0;
}