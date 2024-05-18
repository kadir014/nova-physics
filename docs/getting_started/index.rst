===============
Getting Started
===============

Nova Physics is a lightweight 2D rigid body physics engine. It is designed with game development in mind, however you can utilize it anywhere you need to simulate rigid body dynamics. It is written in portable C with no dependencies other than the standard library, meaning anyone can easily write a binding for their favored programming language.

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

        // Create a ground body with a rectangle shape
        // Making it static means the body will never move no matter what.
        nvRigidBody *ground = nvRigidBody_new(
            nvRigidBodyType_STATIC,
            nvRectShape_new(10.0, 1.0), // A rectangle shape.
            NV_VEC2(0.0, 30.0), // NV_VEC2 is a utility macro to quickly creating vectors.
            0.0,
            nvMaterial_CONCRETE // You can specify a custom material as well.
        );

        // Add the body to the space.
        nvSpace_add(space, ground);

        // Now create a ball that is going to fall to the ground.
        nvRigidBody *ball = nvRigidBody_new(
            nvRigidBodyType_DYNAMIC, // Notice the dynamic type. The ball will move in space.
            nvCircleShape_new(1.5), // Circle shape with radius of 1.5
            NV_VEC2(0.0, 0.0),
            0.0,
            nvMaterial_RUBBER // Giving the ball a rubber material, so it bounces
        );

        nvSpace_add(space, ball);

        // The scene is set up. Now we only have to simulate it!

        // This is the time step length the engine going to simulate the space in.
        nv_float dt = 1.0 / 60.0;

        // Let's simulate for 5 seconds.
        nv_float duration = 5.0;

        for (nv_float t = 0.0; t < duration; t += dt) {
            printf(
                "Ball is at (%.2f, %.2f) with velocity (%.2f, %.2f) at time %.2f.\n",
                ball->position.x, ball->position.y,
                ball->linear_velocity.x, ball->linear_velocity.y,
                t
            );

            // Simulate the space
            nvSpace_step(space, dt, 10, 10, 5, 1);
        }

        // Free the space and all resources it used.
        // Space also manages the bodies and constraints we add to it.
        // Unless you removed them manually, in that case you have to free your bodies.
        nvSpace_free(space);
    }

When the code is run, we can see that the ball bounces on the ground few times and then rests.