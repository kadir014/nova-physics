<p align="center"><img src="https://raw.githubusercontent.com/kadir014/kadir014.github.io/master/assets/novaphysics.png" width=340></p>
<p align="center">
  <img src="https://img.shields.io/badge/license-MIT-blue.svg">
  <img src="https://img.shields.io/badge/version-0.4.0-yellow">
  <a href="https://www.codacy.com/gh/kadir014/nova-physics/dashboard?utm_source=github.com&amp;utm_medium=referral&amp;utm_content=kadir014/nova-physics&amp;utm_campaign=Badge_Grade"><img src="https://app.codacy.com/project/badge/Grade/9556f3db17d54b288557d3b2e9dbf366"></a>
</p>
<p align="center">
Nova Physics is a lightweight and easy to use 2D physics engine.
</p>



# Features
- Simple and user-friendly interface
- Rigid body dynamics
- Iterative sequential impulse solver
- Semi-implicit (symplectic) Euler integrator
- Convex polygon, circle and AABB collisions
- Material properties (friction, restitution and density)
- Collision event callbacks
- Sleeping
- Attractive forces
- Portable codebase with no dependencies



# Future
Nova Physics is still in its early stages as in 0.x.x versions. There is a large room of improvement and optimization of the API and engine's itself. Here are some important points that needs a touch before reaching 1.x.x milestone:

#### Faster broad-phase with Spatial Hashing or Bounding Volume Hierarchy
Current broad-phase strategy is to check AABB collisions between every bodies to narrow down expensive polygon checks. But this aproach is still **far away** from efficient, we are checking every body with each one. That's why the current algorithm needs to be changed in order to have a reasonable performance with _looots_ of bodies simulating.

#### Stability & Stacking
Other than a faster broad-phase, I will be working on simulation stability and general optimization. Methods like accumulating impulses accross frames and warm-starting cached impulses are one of the methods. ([Erin Catto has amazing slides about these topics from GDC](https://box2d.org/files/ErinCatto_SequentialImpulses_GDC2006.pdf))

#### Codebase & Structure
Improve the codebase. Doing so with better docs and all will help me and contributors on the long run.

#### Python Binding
Nova Physics's Python module ([here](https://github.com/kadir014/nova-physics/blob/main/python-binding/)) is still WIP. I plan it to have an easy-to-use Pythonic interface. I might create a new repository for it instead.



# Installing & Building
Nova Phyiscs is lightweight and portable, using only C standard library.
<br><br>
Development libraries are always shipped with the most recent release under the name of `nova-physics-X.X.X-devel.zip` or `.tar.gz`. You can download the archive [here](https://github.com/kadir014/nova-physics/releases) and link `libnova.a` to use Nova Physics in your project.
<br><br>
But if you want (*or need*) to build Nova Physics from scratch on your own, use [the building guide](https://github.com/kadir014/nova-physics/blob/main/BUILDING.md#building-nova-physics-library).



# Examples

<img src="https://raw.githubusercontent.com/kadir014/nova-physics/main/examples/assets/examplegif.gif" width=400>

Example demos are in [examples](https://github.com/kadir014/nova-physics/blob/main/examples/) directory, use [the example building guide](https://github.com/kadir014/nova-physics/blob/main/BUILDING.md#running-examples-on-windows) to run examples.



# Documentation
You can access the documentations [here](https://nova-physics.rtfd.io).



# Resources & References
- **Erin Catto**, [GDC Presentations](https://box2d.org/publications/)
- **Chris Hecker**, [Rigid Body Dynamics](https://chrishecker.com/Rigid_Body_Dynamics)
- **Randy Gaul**, [Game Physics Articles](https://tutsplus.com/authors/randy-gaul)
- **Allen Chou**, [Physics Blogs](https://allenchou.net/category/physics/)
- **Marjin Tamis** & **Giuseppe Maggiore**, [Constraint Based Physics Solver](http://mft-spirit.nl/files/MTamis_ConstraintBasedPhysicsSolver.pdf)
- **Micheal Manzke**, [Multiple Contact Resolution](https://www.scss.tcd.ie/~manzkem/CS7057/cs7057-1516-10-MultipleContacts-mm.pdf)
- **Dirk Gregorius**, [Robust Contact Creationg for Physics Simulations](http://media.steampowered.com/apps/valve/2015/DirkGregorius_Contacts.pdf)
- **Andrew Sevenson**, [Separating Axis Theorem Explanation](https://www.sevenson.com.au/programming/sat/)



# License
[MIT](LICENSE) © Kadir Aksoy