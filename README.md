<p align="center"><img src="https://raw.githubusercontent.com/kadir014/kadir014.github.io/master/assets/novaphysics.png" width=340></p>
<p align="center">
  <img src="https://img.shields.io/badge/license-MIT-blue.svg">
  <img src="https://img.shields.io/badge/version-0.3.1-yellow">
  <a href="https://www.codacy.com/gh/kadir014/nova-physics/dashboard?utm_source=github.com&amp;utm_medium=referral&amp;utm_content=kadir014/nova-physics&amp;utm_campaign=Badge_Grade"><img src="https://app.codacy.com/project/badge/Grade/9556f3db17d54b288557d3b2e9dbf366"></a>
</p>
<p align="center">
Nova Physics is a lightweight and easy to use 2D physics engine.
</p>

# Features
- Simple and user-friendly interface
- Rigid body dynamics
- Convex polygon, circle and AABB collisions
- Material properties (Friction, restitution and density)
- Collision callbacks
- Sleeping
- Attractive forces
- Portable codebase with no dependencies
- `TODO` Ray casting

# Future
#### Faster broad-phase with Bounding Volume Hierarchy
Current broad-phase strategy is to check AABB collisions between bodies to narrow down expensive polygon checks. But this aproach is still far away from efficient. That's why I'll be working on a BVH based tree system for broad-phase.

#### Stability & Stacking
Other than a faster broad-phase, I will be working on simulation stability and general optimization. Methods like accumulating impulses accross frames and warm-starting ([Erin Catto has amazing slides about these topics from GDC](https://box2d.org/files/ErinCatto_SequentialImpulses_GDC2006.pdf))

#### Codebase & Structure
Improve the codebase, there are a few duplicates of array structures that can be merged into one generic array type. Improving the codebase with better docs will help contributors as well.

#### Python Binding
Nova Physics's Python module ([here](https://github.com/kadir014/nova-physics/blob/main/python-binding/)) is still WIP. I plan it to have an easy-to-use Pythonic interface. 

# Examples

<img src="https://raw.githubusercontent.com/kadir014/nova-physics/main/examples/assets/examplegif.gif" width=400>

Example demos are in [examples](https://github.com/kadir014/nova-physics/blob/main/examples/) directory, use [the guidance](https://github.com/kadir014/nova-physics/blob/main/examples/README.md) to build and run examples.

# Testing
Unit tests are in [tests](https://github.com/kadir014/nova-physics/blob/main/tests/) directory, just run `run_tests.py` script to run test suite.

# Resources & References
- **Erin Catto**, GDC Presentations (https://box2d.org/publications/)
- **Chris Hecker**, Rigid Body Dynamics (https://chrishecker.com/Rigid_Body_Dynamics)
- **Randy Gaul**, Game Physics Articles (https://randygaul.github.io/)
- **Marjin Tamis** & **Giuseppe Maggiore**, Constraint Based Physics Solver (http://mft-spirit.nl/files/MTamis_ConstraintBasedPhysicsSolver.pdf)

## License
[MIT](LICENSE) © Kadir Aksoy
