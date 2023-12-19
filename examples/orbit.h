/*

  This file is a part of the Nova Physics Engine
  project and distributed under the MIT license.

  Copyright © Kadir Aksoy
  https://github.com/kadir014/nova-physics

*/

#include "example.h"


void OrbitExample_setup(Example *example) {
    nvSpace *space = example->space;
    
    // Set space gravity to 0
    space->gravity = nvVector2_zero;
    

    // Create planets & stars
    // also give planets some inital force

    nvMaterial star_material = (nvMaterial){
        .density = 15.0,
        .restitution = 0.5,
        .friction = 0.0
    };

    nvMaterial planet_material = (nvMaterial){
        .density = 2.0,
        .restitution = 0.5,
        .friction = 0.0
    };
    
    nvBody *star = nvBody_new(
        nvBodyType_DYNAMIC,
        nvCircleShape_new(3.0),
        NV_VEC2(64.0, 36.0),
        0.0,
        star_material
    );
    
    nvSpace_add(space, star);

    nvBody_set_is_attractor(star, true);


    nvBody *planet1 = nvBody_new(
        nvBodyType_DYNAMIC,
        nvCircleShape_new(1.5),
        NV_VEC2(85.0, 28.0),
        0.0,
        planet_material
    );

    nvSpace_add(space, planet1);

    nvBody_apply_force(planet1, NV_VEC2(0.0, 8.0e3));


    nvBody *planet2 = nvBody_new(
        nvBodyType_DYNAMIC,
        nvCircleShape_new(1.5),
        NV_VEC2(30.0, 35.0),
        0.0,
        planet_material
    );

    nvSpace_add(space, planet2);

    nvBody_apply_force(planet2, NV_VEC2(0.0, 8.0e3));


    nvBody *planet3 = nvBody_new(
        nvBodyType_DYNAMIC,
        nvRectShape_new(3.1, 2.1),
        NV_VEC2(30.0, 55.0),
        0.0,
        planet_material
    );

    nvSpace_add(space, planet3);

    nvBody_apply_force(planet3, NV_VEC2(3.6e3, 5.0e3));
}