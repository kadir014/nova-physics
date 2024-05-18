# Benchmarks
All benchmarks were run on:
- Windows 10
- Intel i5 6402P 4-core @ 2.8GHz
- 16GB DDR4 RAM

Chipmunk2D and Box2D tests are used as a baseline to see how Nova Physics has progressed so far performance-wise. All benchmarks uses the same configuration among all engines:
- 1/60 seconds of timestep
- 10 velocity solving iterations
- 10 position/relax solving iterations
- Continous collision detection is disabled
- Sleeping is disabled
- Warm-starting is enabled

Data points gathered for each benchmark includes thousands of simulation steps for each engine to converge to an accurate result.


## Boxes
3500 bodies with minimal movement
| Physics Engine       | Average physics time (ms) |
|----------------------|---------------------------|
| Nova `0.7.0`         |           32.21           |
| Box2D `2.4.0`        |           46.43           |
| Box2D `3.0.0 alpha`  |               -           |
| Chipmunk2D `7.0.3`   |           17.41           |


## Pyramid
5000 bodies with minimal movement
| Physics Engine       | Average physics time (ms) |
|----------------------|---------------------------|
| Nova `0.7.0`         |           51.08           |
| Box2D `2.4.0`        |           52.00           |
| Box2D `3.0.0 alpha`  |           17.67           |
| Chipmunk2D `7.0.3`   |               -           |


## Ball Pool
9000 bodies with average movement
| Physics Engine       | Average physics time (ms) |
|----------------------|---------------------------|
| Nova `0.7.0`         |            62.09          |
| Box2D `2.4.0`        |            71.38          |
| Box2D `3.0.0 alpha`  |               -           |
| Chipmunk2D `7.0.3`   |            41.69          |