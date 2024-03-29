# Benchmarks
All benchmarks were run on:
- Windows 10
- Intel i5 6402P 4-core @ 2.8GHz
- 16GB DDR4 RAM

Chipmunk2D and Box2D tests are used as a baseline to see how Nova Physics has progressed so far performance-wise. All benchmark scenes use the same configuration among all engines; 10 velocity and position iterations, 60Hz simulation.

Data points gathered for each benchmark includes thousands of simulation steps for each engine to converge to an accurate result.


## Mixer (`mixer.c`)
6000 steps, 1500 objects constantly moving
| Physics Engine       | Average physics time (ms) |
|----------------------|---------------------------|
| Nova `0.5.1`         |           23.65           |
| Nova `0.7.0`         |           10.44           |
| Box2D `2.3.1`        |           10.90           |
| Box2D `3.0.0 alpha`  |               -           |
| Chipmunk2D `7.0.3`   |            4.33           |


## Boxes (`boxes.c`)
5000 steps, 3500 objects minimal movement
| Physics Engine       | Average physics time (ms) |
|----------------------|---------------------------|
| Nova `0.5.1`         |           73.70           |
| Nova `0.7.0`         |           32.21           |
| Box2D `2.3.1`        |           46.43           |
| Box2D `3.0.0 alpha`  |               -           |
| Chipmunk2D `7.0.3`   |           17.41           |


## Ball Pool (`ball_pool.c`)
4000 steps, 9000 objects average movement
| Physics Engine       | Average physics time (ms) |
|----------------------|---------------------------|
| Nova `0.5.1`         |           126.76          |
| Nova `0.7.0`         |            62.09          |
| Box2D `2.3.1`        |            71.38          |
| Box2D `3.0.0 alpha`  |               -           |
| Chipmunk2D `7.0.3`   |            41.69          |