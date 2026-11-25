/*

  This file is a part of the Nova Physics Engine
  project and distributed under the MIT license.

  Copyright © Kadir Aksoy
  https://github.com/kadir014/nova-physics

*/

#ifndef NOVAPHYSICS_TESTS_RANDOM_H
#define NOVAPHYSICS_TESTS_RANDOM_H

#include <stdlib.h>
#include <time.h>


/**
 * @brief Initialize PRNG with a random seed.
 * 
 * @return Chosen seed.
 */
unsigned int nv_init_rand() {
    srand((unsigned int)time(NULL));

    // Iterate the seed to shuffle more
    unsigned int seed;
    for (size_t i = 0; i < 50; i++)
        seed = rand();

    srand(seed);
    return seed;
}

/**
 * @brief Return random nv_float in given range.
 * 
 * @param lower Min range.
 * @param higher Max range.
 * @return nv_float 
 */
static inline nv_float nv_frand(nv_float lower, nv_float higher) {
    nv_float normal = rand() / (nv_float)RAND_MAX;
    return lower + normal * (higher - lower);
}


#endif // NOVAPHYSICS_TESTS_RANDOM_H