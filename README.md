<p align="center"><img src="https://raw.githubusercontent.com/kadir014/kadir014.github.io/master/assets/novaphysics.png" width=340></p>
<p align="center">
  <img src="https://img.shields.io/badge/license-MIT-blue.svg">
  <img src="https://img.shields.io/badge/version-0.7.0-yellow">
  <a href="https://www.codacy.com/gh/kadir014/nova-physics/dashboard?utm_source=github.com&amp;utm_medium=referral&amp;utm_content=kadir014/nova-physics&amp;utm_campaign=Badge_Grade"><img src="https://app.codacy.com/project/badge/Grade/9556f3db17d54b288557d3b2e9dbf366"></a>
</p>
<p align="center">
Nova Physics is a lightweight and easy to use 2D physics engine.
</p>



# Features
- Simple and user-friendly interface
- Rigid body dynamics
- Primitive shape collisions (circle, rect, polygon, AABB)
- Physical material properties (friction, restitution and density)
- Joint constraints (spring, distance, hinge ..)
- Great stacking stability and collision persistence
- Iterative sequential impulse solver
- Semi-implicit (symplectic) Euler integrator
- Collision event callbacks
- Sleeping bodies to reduce CPU load
- Attractive forces
- Portable codebase with no dependencies
- Various interactive example demos using SDL2



# Roadmap & Future
Nova Physics is still in its early stages as in `0.x.x` versions. There is a large room of improvement and optimization of the API and engine's itself. Here are some of the important points that needs a touch before reaching the `1.x.x` milestone:

- ### Better & faster broad-phase
  Current broad-phase strategies available in Nova are a spatial hash grid and a BVH (bounding volume hierarchy) tree. Both are fast but there is still room for improvement, especially for BVH-tree construction and multi-threaded SHG tasks.

- ### Python binding
  Nova Physics's Python module ([here](https://github.com/kadir014/nova-physics-python)) is still WIP. I plan it to have an easy-to-use Pythonic interface. Other language binding contributions are also always welcome!



# Installing & Building
Development libraries are always shipped with the most recent release under the name of `nova-physics-X.X.X-devel.zip` (or `.tar.gz`). You can download the archive [here](https://github.com/kadir014/nova-physics/releases) and link `libnova.a` (or `libnova.lib`) with your favorite compiler to use Nova Physics in your project.
<br><br>
But if you want (*or need*) to build Nova Physics from scratch on your own, use [the building guide](https://github.com/kadir014/nova-physics/blob/main/BUILDING.md#building-nova-physics-static-libraries).



# Examples

<img src="https://raw.githubusercontent.com/kadir014/nova-physics/main/examples/assets/examplegif.gif" width=400>

Example demos are in [examples](https://github.com/kadir014/nova-physics/blob/main/examples/) directory, use [the example building guide](https://github.com/kadir014/nova-physics/blob/main/BUILDING.md#running-nova-physics-example-demos) to run examples.



# Documentation
You can access the documentations [here](https://nova-physics.rtfd.io).



# Resources & References
Following are some of the many great resources that helped me to build Nova Physics to this state.
- **Erin Catto**, [GDC Presentations](https://box2d.org/publications/)
- **Chris Hecker**, [Rigid Body Dynamics](https://chrishecker.com/Rigid_Body_Dynamics)
- **Randy Gaul**, [Game Physics Articles](https://tutsplus.com/authors/randy-gaul)
- **Allen Chou**, [Physics Blogs](https://allenchou.net/category/physics/)
- **Marjin Tamis** & **Giuseppe Maggiore**, [Constraint Based Physics Solver](http://mft-spirit.nl/files/MTamis_ConstraintBasedPhysicsSolver.pdf)
- **Micheal Manzke**, [Multiple Contact Resolution](https://www.scss.tcd.ie/~manzkem/CS7057/cs7057-1516-10-MultipleContacts-mm.pdf)
- **Dirk Gregorius**, [Robust Contact Creation for Physics Simulations](http://media.steampowered.com/apps/valve/2015/DirkGregorius_Contacts.pdf)
- **Andrew Sevenson**, [Separating Axis Theorem Explanation](https://www.sevenson.com.au/programming/sat/)



# License
[MIT](LICENSE) © Kadir Aksoy

Nova Physics is, and always will be, free and open-source. Although we would greatly appreciate [donations!](https://www.buymeacoffee.com/kadir014)