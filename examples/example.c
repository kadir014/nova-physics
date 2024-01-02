/*

  This file is a part of the Nova Physics Engine
  project and distributed under the MIT license.

  Copyright © Kadir Aksoy
  https://github.com/kadir014/nova-physics

*/

#include <string.h>
#include "example.h"

#include "arch.h"
#include "bridge.h"
#include "chains.h"
#include "circle_stack.h"
#include "cloth.h"
#include "constraints.h"
#include "domino.h"
#include "fountain.h"
#include "hull.h"
#include "newtons_cradle.h"
#include "orbit.h"
#include "pool.h"
#include "pyramid.h"
#include "spring_car.h"
#include "stack.h"
#include "varying_bounce.h"
#include "varying_friction.h"


/**
 * @file example.h
 * 
 * @brief Entry point for example demos.
 */


ExampleEntry example_entries[100] = {NULL};
size_t example_count = 0;
size_t current_example = 0;

void Example_register(
    char *name,
    Example_callback setup_callback,
    Example_callback update_callback,
    void (* register_callback)(ExampleEntry *)
) {
    example_entries[example_count] = (ExampleEntry){
        .name=name,
        .slider_settings=nvArray_new(), //TODO: free
        .setup_callback=setup_callback,
        .update_callback=update_callback
    };
    if (register_callback) register_callback(&example_entries[example_count]);
    example_count++;
}

void Example_set_current(char *name) {
    for (size_t i = 0; i < example_count; i++) {
        if (!strcmp(name, example_entries[i].name)) {
            current_example = i;
            return;
        }
    }
}


int main(int argc, char *argv[]) {
    Example_register("Arch", ArchExample_setup, NULL, NULL);
    Example_register("Bridge", BridgeExample_setup, NULL, NULL);
    Example_register("Chains", ChainsExample_setup, NULL, NULL);
    Example_register("Circle Stack", CircleStackExample_setup, NULL, NULL);
    Example_register("Cloth", ClothExample_setup, NULL, ClothExample_init);
    Example_register("Constraints", ConstraintsExample_setup, NULL, NULL);
    Example_register("Domino", DominoExample_setup, NULL, NULL);
    Example_register("Fountain", FountainExample_setup, FountainExample_update, FountainExample_init);
    Example_register("Hull", HullExample_setup, NULL, NULL);
    Example_register("Newton's Cradle", NewtonsCradleExample_setup, NULL, NULL);
    Example_register("Orbit", OrbitExample_setup, NULL, NULL);
    Example_register("Pool", PoolExample_setup, NULL, NULL);
    Example_register("Pyramid", PyramidExample_setup, NULL, PyramidExample_init);
    Example_register("Spring Car", SpringCarExample_setup, SpringCarExample_update, NULL);
    Example_register("Stack", StackExample_setup, NULL, NULL);
    Example_register("Varying Bounce", VaryingBounceExample_setup, NULL, NULL);
    Example_register("Varying Friction", VaryingFrictionExample_setup, NULL, NULL);

    Example *example = Example_new(
        1280, 720,
        "Nova Physics Example Demos",
        165.0,
        1.0/60.0, // 60Hz = 1/60 dt
        ExampleTheme_DARK
    );

    Example_set_current("Pyramid");
    
    Example_run(example);

    Example_free(example);

    return 0;
}