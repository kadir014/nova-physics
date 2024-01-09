/*

  This file is a part of the Nova Physics Engine
  project and distributed under the MIT license.

  Copyright © Kadir Aksoy
  https://github.com/kadir014/nova-physics

*/

#include "example.h"


void OrbitExample_setup(Example *example) {
    nvSpace *space = example->space;
    
    space->gravity = nvVector2_zero;

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

    for (nv_float angle = 0.0; angle < 2.0 * NV_PI; angle += 0.1) {
        nv_float dist = frand(25.0, 40.0);
        nvVector2 delta = nvVector2_rotate(NV_VEC2(dist, 0.0), angle);
        nvVector2 pos = nvVector2_add(star->position, delta);
        
        nvBody *planet = nvBody_new(
            nvBodyType_DYNAMIC,
            nvCircleShape_new(0.7),
            pos,
            0.0,
            planet_material
        );

        nvSpace_add(space, planet);

        nv_float strength = 1.5e2 / ((dist - 25.0) / 5.0 + 1.0); // / ((dist - 20.0) * 0.5);
        nvBody_apply_force(planet, nvVector2_mul(nvVector2_perp(delta), strength));
    }
}