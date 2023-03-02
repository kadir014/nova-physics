# Overview
Nova Physics is a lightweight rigid body physics engine. It is created with game development in mind but you can use it anywhere you need to simulate rigid body dynamics. It is written in portable C with an addition of Python binding on its way.

Nova Physics tries to give satisfying results with the developer having control over the simulation, creating realistic and accurate results within real-time performance.



# Usage & Installing
Development libraries are always shipped with the most recent release under the name of `nova-physics-X.X.X-devel.zip` or `.tar.gz`. You can download the archive [here](https://github.com/kadir014/nova-physics/releases) and link `libnova.a` to use Nova Physics in your project.

But if you want (*or need*) to build Nova Physics from scratch on your own, use [the building guide](https://github.com/kadir014/nova-physics/blob/main/BUILDING.md#building-nova-physics-library).



# Important
To keep the overall simulation stable and accurate, there are few aspects you have to keep an eye on while using Nova Physics:
- Use a fixed time step for `nv_Space_step()` method, don't use the delta time from your FPS counter as it will not certainly help the stability and make the simulation non-deterministic.
- Keep the bodies in a standard size, don't create microscopic or gigantic bodies.
- While creating polygon bodies, make sure the shape's centroid is on the body's center of mass. You can use `nv_polygon_centroid()` method for that.



# Feedback & Issues
You can check out [contributing guidelines](https://github.com/kadir014/nova-physics/blob/main/CONTRIBUTING.md) for any of your related questions.



# Quick Starter Example
Here is a quick start point before diving into reference documentation
```c
// This header includes all the API you'll need
#include "novaphysics/novaphysics.h"

int main() {
    // Space is the container of everything
    nv_Space *space = nv_Space_new();
    
    // Let's create a basic circle object
    // nv_Circle_new is a helper function that creates a body for us
    nv_Body *ball = nv_Circle_new(
        nv_BodyType_DYNAMIC, // Type of the body
        NV_VEC2(0.0, 10.0),  // Position (center of mass)
        0.0                  // Angle
        nv_Material_WOOD,    // Material
        5.0                  // Radius
    );

    // Add our ball the space, so it can be simulated
    nv_Space_add(space, body);

    // Simulate 1 second
    int ticks = 60;
    nv_float dt = 1.0 / 60.0;

    for (size_t i = 0; i < ticks; i++) {
        // Advance the simulation
        // (You can learn about the other parameters in the reference docs)
        nv_Space_step(space, dt, 1, 1, 1);

        // We will see that our ball is falling :)
        printf("ball y: %f\n", ball->position.y);
    }

    // We only have to call space's free method, it will free all the other
    // things it contains for us
    nv_Space_free(space);

    return 0;
}
```