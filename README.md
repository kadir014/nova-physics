<p align="center"><img src="https://raw.githubusercontent.com/kadir014/kadir014.github.io/master/assets/novaphysics.png" width=340></p>
<p align="center">
  <img src="https://img.shields.io/badge/license-MIT-blue.svg">
  <img src="https://img.shields.io/badge/version-1.0.0-yellow">
  <a href="https://app.codacy.com/gh/kadir014/nova-physics/dashboard?utm_source=gh&utm_medium=referral&utm_content=&utm_campaign=Badge_grade"><img src="https://app.codacy.com/project/badge/Grade/9556f3db17d54b288557d3b2e9dbf366"></a>
</p>
<p align="center">
Nova Physics is a lightweight and easy to use 2D physics engine designed with game development in mind.
<br>
<img src="https://raw.githubusercontent.com/kadir014/nova-physics/main/examples/assets/examplegif.gif" width=400>
</p>
<p align="center">
<sup><i>You can also read this page in</i></sub>
<br>
<a href="#"><img src="https://raw.githubusercontent.com/kadir014/nova-physics/main/docs/_static/flag_uk.png" width=30 alt="EN"></a>
<a href="./docs/translations/README_tr.md"><img src="https://raw.githubusercontent.com/kadir014/nova-physics/main/docs/_static/flag_tr.png" width=30></a>
</p>



# Features
- Simple and user-friendly interface
- Portable codebase with no dependencies
- Rigid body dynamics
- Discrete collision detection
  - Circle shape
  - Convex polygon shape
  - Testing shapes against shapes
  - Testing shapes against point
- One-shot contact manifold generation between shapes
- Multiple shapes per body
- Broadphase strategies
  - Bruteforce
  - Spatial hash grid
  - Bounding volume hierarchy tree
- Material properties (friction, restitution and density)
- Constraints between bodies
  - Distance constraint - can also behave like spring
  - Hinge constraint
- [Erin Catto's](https://box2d.org/files/ErinCatto_SequentialImpulses_GDC2006.pdf) sequential impulse solver algorithm
- Great stacking stability
- Semi-implicit (symplectic) Euler integrator
- Collision filtering with masks and grouping
- Built-in profiler
- Optional double-precision mode
- Various interactive demos using SDL2 & OpenGL


# Building
The library uses C99 standard and depends only on the C standard library, which you can change in with your own definitions.

For further instructions see [here](BUILDING.md).



# Examples

<img src="https://raw.githubusercontent.com/kadir014/nova-physics/main/examples/assets/examplegif.gif" width=400>

Example demos are in [examples](https://github.com/kadir014/nova-physics/blob/main/examples/) directory, enable building demos option in the build system (if not already enabled).



# Documentation
You can access the documentation [here](https://nova-physics.rtfd.io).



# Resources & References
Following are some of the many great resources that helped me to build Nova Physics to this state.
- **Erin Catto**, [GDC Presentations](https://box2d.org/publications/) and [Box2D](https://github.com/erincatto/box2c)
- **Chris Hecker**, [Rigid Body Dynamics](https://chrishecker.com/Rigid_Body_Dynamics)
- **Ian Millington**, [Game Physics Engine Development](https://www.r-5.org/files/books/computers/algo-list/realtime-3d/Ian_Millington-Game_Physics_Engine_Development-EN.pdf)
- **Christer Ericson**, [Real-Time Collision Detection](https://www.r-5.org/files/books/computers/algo-list/realtime-3d/Christer_Ericson-Real-Time_Collision_Detection-EN.pdf)
- **Dirk Gregorius**, [Robust Contact Creation for Physics Simulations](http://media.steampowered.com/apps/valve/2015/DirkGregorius_Contacts.pdf)
- **Randy Gaul**, [Game Physics Articles](https://tutsplus.com/authors/randy-gaul)
- **Allen Chou**, [Physics Blogs](https://allenchou.net/category/physics/)
- **Marjin Tamis** & **Giuseppe Maggiore**, [Constraint Based Physics Solver](http://mft-spirit.nl/files/MTamis_ConstraintBasedPhysicsSolver.pdf)
- **Micheal Manzke**, [Multiple Contact Resolution](https://www.scss.tcd.ie/~manzkem/CS7057/cs7057-1516-10-MultipleContacts-mm.pdf)



# License
[MIT](LICENSE) © Kadir Aksoy

Nova Physics is, and always will be, free and open-source. Although we would greatly appreciate [donations!](https://www.buymeacoffee.com/kadir014)