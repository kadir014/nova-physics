# Overview
Nova Physics is a lightweight rigid body physics engine. It is created with game development in mind but you can use it anywhere you need to simulate rigid body dynamics. It is written in portable C with an addition of Python binding on its way.

Nova Physics tries to give satisfying results with the developer having control over it, creating a realistic and accurate results in bond with real-time performance.

# Usage & Installing
With every release, an archive is shipped with it that you can use to include and link to your program. Or you can use `build.py` script to build the library yourself.

Everything is prefixed with `nv` so you can use the library without mixing up your namespaces.

# Important
To keep the overall simulations stable and realistic-enough looking, there are few aspects you have to keep attention to while using Nova Physics:
- Use a fixed time step for `nv_Space_step()` method, don't use the delta time from your FPS counter as it will not help the stability and make the simulation non deterministic.
- Keep the bodies in a standard size, don't create microscopic or gigantic bodies.
- While creating polygon bodies, use its vertices' center point as body's center point (position attr.) You can use `nv_polygon_centroid()` method from `math.h` to find the center of given vertices.

# Feedback & Issues
You can check out [contributing guidelines](CONTRIBUTING.md) for any of your related questions.
