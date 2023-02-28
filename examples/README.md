# Welcome to Nova Physics Example Demos 👋
To build & run examples use Nova Physics's build system `nova_builder.py` Python script.

## Windows
Dependencies will be installed by the script.

Set working directory as the Nova Physics directory.
```
cd "C:\...\nova-physics"
```

Then run the build system, it will detect the missing dependencies and install them on the fly.
```
python nova_builder.py example <example_name>
```

## Linux
You have to install SDL2 and TTF extension via your package manager or build it yourself.

Here are two sources explaining how to install SDL2 on Linux
- https://wiki.libsdl.org/SDL2/Installation
- https://lazyfoo.net/tutorials/SDL/01_hello_SDL/linux/index.php

After installing the dependencies, set working directory as Nova Physics directory.
```
cd ./.../nova-physics
```
Then run the build system. (It can be `py` or `python3` depending on the platform.)
```
python nova_builder.py example <example_name>
```


## Examples in alphabetical order:
- [Arch](#arch)
- [Bridge](#bridge)
- [Orbit](#orbit)
- [Pool](#pool)
- [Pyramid](#pyramid)
- [Spring Car](#spring-car)
- [Springs](#springs)


## Arch
Showcasing a semi-circle arch structure holding together with friction. Source file is `arch.c`

<img src="https://raw.githubusercontent.com/kadir014/nova-physics/main/examples/assets/example_arch.png" width=620>


## Bridge
Showcasing bodies attached together with multiple spring constraints. Source file is `bridge.c`

<img src="https://raw.githubusercontent.com/kadir014/nova-physics/main/examples/assets/example_bridge.png" width=620>


## Orbit
Showcasing attractive forces with an orbital system similar to our solar system. Source file is `orbit.c`

<img src="https://raw.githubusercontent.com/kadir014/nova-physics/main/examples/assets/example_orbit.png" width=620>


## Pool
Showcasing lots of circle bodies in a pool. Source file is `pool.c`

<img src="https://raw.githubusercontent.com/kadir014/nova-physics/main/examples/assets/example_pool.png" width=620>


## Pyramid
Showcasing stacked boxes in pyramid form. Source file is `pyramid.c`

<img src="https://raw.githubusercontent.com/kadir014/nova-physics/main/examples/assets/example_pyramid.png" width=620>


## Spring Car
Showcasing a car with suspension wheels which can be controlled with <kbd>←</kbd> and <kbd>→</kbd>. Source file is `spring_car.c`

<img src="https://raw.githubusercontent.com/kadir014/nova-physics/main/examples/assets/example_spring_car.png" width=620>


## Springs
Showcasing spring constraints. Source file is `springs.c`

<img src="https://raw.githubusercontent.com/kadir014/nova-physics/main/examples/assets/example_springs.png" width=620>
