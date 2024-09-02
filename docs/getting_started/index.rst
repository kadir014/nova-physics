===============
Getting Started
===============

Nova Physics is a lightweight 2D rigid body physics engine. It is designed with game development in mind. It is written in portable C with no dependencies other than the standard library, meaning anyone can easily write a binding for their favored programming language.

Nova Physics is, and always will be, free and open-source.

Hello World
===========

After installing (you can follow :doc:`installing`), you are ready for your first Nova Physics program, where a ball free falls onto a ground and we print its coordinates every step.

.. code-block:: c
    :linenos:

    #include <stdio.h>
    #include "novaphysics/novaphysics.h"


    int main() {
        // Create an empty simulation space
        nvSpace *space = nvSpace_new();


        /* Create a ground body with a rectangle shape. */

        // The initializer struct is used to set basic properties of a body before creation.
        // Making the motion type static means the body will never move under simulation.
        nvRigidBodyInitializer body_init = nvRigidBodyInitializer_default;
        body_init.type = nvRigidBodyType_STATIC;
        body_init.position = NV_VECTOR2(0.0, 20.0); // NV_VECTOR2 is a utility macro to quickly initialize vectors.
        body_init.material = nvMaterial_CONCRETE; // You can initialize a custom material as well.
        nvRigidBody *ground = nvRigidBody_new(body_init);

        // After creating the rigid body, we have to assign a shape.
        nvShape *ground_shape = nvBoxShape_new(10.0, 1.0, nvVector2_zero);
        nvRigidBody_add_shape(ground, ground_shape);

        // We can finally add ground body to space.
        nvSpace_add_rigidbody(space, ground);


        /* Now create a ball that is going to fall and bounce off the ground. */

        // We can use the same initializer struct by changing the necessary fields.
        // This time the motion type have to by dynamic so the ball can be simulated under physical forces.
        // We also initialize a custom material with restitution 0.85, so it can bounce
        body_init.type = nvRigidBodyType_DYNAMIC;
        body_init.position = NV_VECTOR2(0.0, 0.0);
        body_init.material = (nvMaterial){.density=1.0, .restitution=0.85, .friction=0.1};
        nvRigidBody *ball = nvRigidBody_new(body_init);

        // Now we assign a circle shape to our body with radius 1.0
        nvShape *ball_shape = nvCircleShape(nvVector2_zero, 1.0);
        nvRigidBody_add_shape(ball, ball_shape);

        nvSpace_add_rigidbody(space, ground);


        /* The scene is set up. Now we only have to simulate it! */

        // This is the time step length the engine going to simulate the space in.
        nv_float dt = 1.0 / 60.0;

        // Let's simulate for 5 seconds.
        nv_float duration = 5.0;

        for (nv_float t = 0.0; t < duration; t += dt) {
            nvVector2 position = nvRigidBody_get_position(ball);
            nvVector2 velocity = nvRigidBody_get_linear_velocity(ball);

            printf(
                "Ball is at (%.2f, %.2f) with velocity (%.2f, %.2f) at time %.2f.\n",
                position.x, position.y,
                velocity.x, velocity.y,
                t
            );

            // Advance the simulation.
            nvSpace_step(space, dt);
        }

        // Free the space and all resources it used.
        // Space also manages the bodies, shapes and constraints we add to it.
        // Unless you removed them manually, in that case you have to free your bodies.
        nvSpace_free(space);
    }

When the code is run, we can see that the ball bounces on the ground few times and then rests.