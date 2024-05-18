#ifndef NOVAPHYSICS_EXAMPLE_COMMON_H
#define NOVAPHYSICS_EXAMPLE_COMMON_H

#include <stdint.h>
#include <stdlib.h>


typedef uint32_t nv_uint32;


float frand(float lower, float higher) {
    float normal = rand() / (float)RAND_MAX;
    return lower + normal * (higher - lower);
}


#endif