# Benchmarks
All benchmarks were run on:
- Windows 10
- Intel i5 6402P 4-core @ 2.8GHz
- 16GB DDR4 RAM

Chipmunk2D and Box2D tests are used as references to see how Nova Physics has progressed so far performance-wise. All benchmarks uses the same configuration among all engines:
- 1/60 seconds of timestep
- 10 velocity solving iterations
- 10 position/relax iterations
- Continous collision detection is disabled
- Sleeping is disabled
- Warm-starting is enabled

Data points gathered for each benchmark includes thousands of simulation steps for each engine to converge to an accurate result.


# TODO: Update benchmarks with 1.0.0