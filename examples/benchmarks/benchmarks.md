# Big ball pool
This benchmark aims to test lots of circle vs. circle collisions.

**Benchmark details**:
- Nova Physics: `0.3.1`
- Machine: Windows 10, Intel i5 6402P 2.8GHz (4 cores)
- Compiler: GCC (MinGW 64) 11.2.0
- Impulse iterations: `8`
- Substeps: `1`

**Result**: ~25ms / step

```
┌┐         ┌┐ Big Pool benchmark
││ooooooooo││ 1500 (50x30) balls
││ooooooooo││
││ooooooooo││
│└─────────┘│
└───────────┘

1000 steps, ran in 23.7 seconds         10,000 steps, ran in 4.1 minutes
 
       μs       ms       s                     μs       ms       s
       -------- -------- --------              -------- -------- --------
min:   18952.2  18.952   0.018952       min:   18812.0  18.812   0.018812
max:   31605.6  31.606   0.031606       max:   64091.0  64.091   0.064091
avg:   23701.5  23.702   0.023702       avg:   24750.1  24.750   0.024750
stdev: 1144.2   1.144    0.001144       stdev: 1228.6   1.229    0.001229
```


# Rotated boxes pool
This benchmark aims to test lots of polygon vs. polygon collisions

**Benchmark details**:
- Nova Physics: `0.3.1`
- Machine: Windows 10, Intel i5 6402P 2.8GHz (4 cores)
- Compiler: GCC (MinGW 64) 11.2.0
- Impulse iterations: `10`
- Substeps: `3`

**Result**: ~44ms / step

```
┌┐         ┌┐ Boxes benchmark
││□□□□□□□□□││ 660 (20x33) rotated boxes
││□□□□□□□□□││
││□□□□□□□□□││
│└─────────┘│
└───────────┘

1000 steps, ran in 44 seconds           10,000 steps, ran in 7.4 minutes

       μs       ms       s                     μs       ms       s
       -------- -------- --------              -------- -------- --------
min:   35493.9  35.494   0.035494       min:   35351.1  35.351   0.035351
max:   47556.7  47.557   0.047557       max:   57837.1  57.837   0.057837
avg:   43408.7  43.409   0.043409       avg:   44407.2  44.407   0.044407
stdev: 2397.6   2.398    0.002398       stdev: 980.4    0.980    0.000980
```