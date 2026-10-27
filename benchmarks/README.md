# Benchmarks
All benchmark data were collected on the same machine and built with the same compiler.

Results from other engines are included solely as reference points to provide context for Nova Physics's performance and track its progress over time. They are not intended as competitive comparisons or rankings.

Data points gathered from each benchmark includes at least hundreds of simulation steps to converge to an accurate result.

### Platform:
- Windows 10 Pro
- AMD Ryzen 5 7500f 6-core @ 3.7GHz
- 32GB DDR5 RAM
- Compiled with x64 MSVC 19.42 (Visual Studio 17 2022)

### Configuration
All benchmarks try to match these settings as similar as possible for consistency.
- `1/60` seconds of timestep
- `8` iterations for velocity constraints
- `4` iterations for position constraints (NGS)
- `32-bit` floating-point mode
- ❌ Any sort of continous collision detection is disabled
- ❌ Sleeping is disabled
- ✔️ Warm-starting is enabled

### Benchmark Scenes
- [Pyramid](#pyramid-scene)


# Pyramid Scene
Pyramid scene consists of over 5000 bricks laid over each other.
| Engine             | Physics Frame (avg ms) | Workers | Notes |
|--------------------|------------------------|---------|-------|
| Nova `1.2.0`       | 14.1361                | -       |       |
| Box2D `2.4.2`      | 11.2717                | -       |       |
| Box2D `3.2.0`      | 1.82547                | 1       |       |
| Box2D `3.2.0`      | 0.46813                | 6+1     |       |
| Chipmunk2D `7.0.3` | 8.35005                | -       |       |