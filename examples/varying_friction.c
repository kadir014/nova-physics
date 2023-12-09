/*

  This file is a part of the Nova Physics Engine
  project and distributed under the MIT license.

  Copyright © Kadir Aksoy
  https://github.com/kadir014/nova-physics

*/

#include "example_base.h"


void setup(Example *example) {
    nvMaterial platform_mat = {
        .density = 1.0,
        .restitution = 0.0,
        .friction = 0.5
    };

    nv_float platform_angle = 0.6;
    nv_float box_angle = 0.0;

    // Create platforms
    nvBody *platform0 = nv_Rect_new(
        nvBodyType_STATIC,
        NV_VEC2(64.0, 18.0 + 15.0),
        platform_angle,
        platform_mat,
        100.0, 2.0
    );

    nvSpace_add(example->space, platform0);

    nvBody *platform1 = nv_Rect_new(
        nvBodyType_STATIC,
        NV_VEC2(64.0, 36.0 + 15.0),
        platform_angle,
        platform_mat,
        100.0, 2.0
    );

    nvSpace_add(example->space, platform1);

    nvBody *platform2 = nv_Rect_new(
        nvBodyType_STATIC,
        NV_VEC2(64.0, 54.0 + 15.0),
        platform_angle,
        platform_mat,
        100.0, 2.0
    );

    nvSpace_add(example->space, platform2);

    // Create boxes
    nvBody *box0 = nv_Rect_new(
        nvBodyType_DYNAMIC,
        NV_VEC2(50.0, 18.0),
        box_angle,
        (nvMaterial){
            .density = 1.0,
            .restitution = 0.0,
            .friction = 0.0
        },
        3.0, 3.0
    );
    
    nvSpace_add(example->space, box0);

    nvBody *box1 = nv_Rect_new(
        nvBodyType_DYNAMIC,
        NV_VEC2(50.0, 36.0),
        box_angle,
        (nvMaterial){
            .density = 1.0,
            .restitution = 0.0,
            .friction = 0.35
        },
        3.0, 3.0
    );
    
    nvSpace_add(example->space, box1);

    nvBody *box2 = nv_Rect_new(
        nvBodyType_DYNAMIC,
        NV_VEC2(50.0, 54.0),
        box_angle,
        (nvMaterial){
            .density = 1.0,
            .restitution = 0.0,
            .friction = 0.8
        },
        3.0, 3.0
    );
    
    nvSpace_add(example->space, box2);
}


int main(int argc, char *argv[]) {
    // Create example
    Example *example = Example_new(
        1280, 720,
        "Nova Physics  -  Varying Friction Example",
        165.0,
        1.0/60.0,
        ExampleTheme_DARK
    );

    // Set callbacks
    example->setup_callback = setup;

    // Run the example
    Example_run(example);

    // Free the space allocated by example
    Example_free(example);

    return 0;
}