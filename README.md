<p align="center"><img src="https://raw.githubusercontent.com/kadir014/kadir014.github.io/master/assets/novaphysics.png" width=340></p>
<p align="center">
  <img src="https://img.shields.io/badge/license-MIT-blue.svg">
  <img src="https://img.shields.io/badge/version-0.1.2-yellow">
</p>
<p align="center">
Nova physics is a lightweight and easy to use 2d physics engine.
</p>

## Features
- Simple and user-friendly interface
- Rigid body dynamics
- Convex polygon and circle collisions
- Friction and restitution
- Collision callbacks
- Sleeping
- Collision callbacks
- Portable codebase with no dependencies
- `TODO` Ray casting
- `TODO` Attractive forces

## Future
#### Faster broad-phase with Bounding Volume Hierarchy
Current broad-phase strategy is to check AABB collisions between bodies to narrow down expensive polygon checks. But this aproach is still far away from efficient. That's why I'll be working on a BVH based tree system for broad-phase.

#### Stability & Stacking
Other than a faster broad-phase, I will be working on simulation stability and general optimization. Methods like accumulating impulses accross frames, warm-starting, coherence... ([Erin Catto has amazing slides about these topics from GDC](https://box2d.org/files/ErinCatto_SequentialImpulses_GDC2006.pdf))

#### Python Binding
Nova Physics's Python module ([here](https://github.com/kadir014/nova-physics/blob/main/python-binding/)) is still WIP. I plan it to have an easy-to-use Pythonic interface. 

## Examples
Example demos are in [examples](https://github.com/kadir014/nova-physics/blob/main/examples/) directory, just run `run_example.py` with any example as argument (eg. `python run_example.py pool`).

The script will also download any dependency like `SDL2` if needed.

## Testing
Unit tests are in [tests](https://github.com/kadir014/nova-physics/blob/main/tests/) directory, just run `run_tests.py` script to run test suite.

# Resources & References
- **Erin Catto**, GDC Slides (https://box2d.org/publications/)
- **Chris Hecker**, Rigid Body Dynamics (https://chrishecker.com/Rigid_Body_Dynamics)
- **Randy Gaul**, Game Physics Articles (https://randygaul.github.io/)

## License
[MIT](LICENSE) © Kadir Aksoy