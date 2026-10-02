/*

  This file is a part of the Nova Physics Engine
  project and distributed under the MIT license.

  Copyright © Kadir Aksoy
  https://github.com/kadir014/nova-physics

*/

#include "../common.h"


#define CREATEDESTROY_ROWS 100
#define CREATEDESTROY_COLS 100
#define CREATEDESTROY_SIZE 1.0f

void CreateDestroy_setup(ExampleContext *example) {}

void CreateDestroy_update(ExampleContext *example) {
    ExampleContext_reset(example);

    nvRigidBody *ground;
    nvRigidBodyInitializer ground_init = nvRigidBodyInitializer_default;
    ground_init.position = NV_VECTOR2(64.0f, 72.0f - 2.5f);
    ground = nvRigidBody_new(ground_init);

    nvShape *ground_shape = nvBoxShape_new(128.0f, 5.0f, nvVector2_zero);
    nvRigidBody_add_shape(ground, ground_shape);

    nvSpace_add_rigidbody(example->space, ground);


    nv_float start_y = 72.0f - 2.5f - 2.5f - CREATEDESTROY_SIZE / 2.0f;

    for (size_t y = 0; y < CREATEDESTROY_ROWS; y++) {
        // Random horizontal offset for each row
        float offset = frand(-0.1f, 0.1f);

        for (size_t x = 0; x < CREATEDESTROY_COLS; x++) {

            nvRigidBody *box;
            nvRigidBodyInitializer box_init = nvRigidBodyInitializer_default;
            box_init.type = nvRigidBodyType_DYNAMIC;
            box_init.position = NV_VECTOR2(
                64.0f - CREATEDESTROY_SIZE * ((nv_float)CREATEDESTROY_COLS * 0.5f) + x * CREATEDESTROY_SIZE + offset,
                start_y - y * (CREATEDESTROY_SIZE - 0.01f) // Sink a little so collisions happen in first frame
            );
            box_init.material = (nvMaterial){.density=1.0f, .restitution=0.1f, .friction=0.6f};
            box = nvRigidBody_new(box_init);

            nvShape *box_shape = nvBoxShape_new(CREATEDESTROY_SIZE, CREATEDESTROY_SIZE, nvVector2_zero);
            nvRigidBody_add_shape(box, box_shape);

            nvSpace_add_rigidbody(example->space, box);
        }
    }
}