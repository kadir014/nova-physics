# Benchmarks
All benchmarks were run on a Windows 10, Intel i5 6th gen machine.

Chipmunk2D and Box2D tests are used as a baseline to see how Nova Physics has progress so far performance-wise. All benchmark scenes use the same configuration among all engines; 10 velocity and position iterations, 60Hz simulation.

Data points gathered for each benchmark includes 30,000 frames and shows how long it takes to simulate one step in milliseconds.



## Mixer (`mixer.c`)
| Physics Engine       | Average physics time (ms) |
|----------------------|---------------------------|
| Nova Physics `0.5.1` |           23.65           |
| Box2D `2.3.1`        |           10.90           |
| Chipmunk2D `7.0.3`   |            4.33           |