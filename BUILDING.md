# Nova Physics Build System Guidance
Welcome to the Nova Physics building guide.

Nova Physics uses a dependency-free Python script to simplify and organize building.

You can use `python nova_builder.py` without any arguments and commands to see usage manual.

<img src="https://raw.githubusercontent.com/kadir014/nova-physics/main/examples/assets/example_cli.png" width=450>

**What you can do with the build system:**
- [Build Nova Physics library](#building-nova-physics-library)
- [Build & run example demos](#running-examples-on-windows)
- [Build & run benchmarks](#running-nova-physics-benchmarks)
- [Build & run the test suite](#running-nova-physics-test-suite)



# Building Nova Physics Library
This section covers building the library yourself (in case you can't use the already-built archives from the latest release).

### Building Nova Physics on Windows
**Prerequisite**: Make sure you have MinGW (`gcc.exe`) or MSVC (`cl.exe`) on PATH.

Download the repository archive or clone it.
```
git clone https://github.com/kadir014/nova-physics.git
```

Set working directory as Nova Physics directory.
```
cd "C:\...\nova-physics"
```

Run Nova Physics's build system with `build` command to start building.
```
python nova_builder.py build
```

If successful, you should see results under `nova-physics\build` sub-directory.


### Building Nova Physics on Linux

**Prerequisite**: Make sure you have GCC installed

Ubuntu:
```
$ sudo apt-get install build-essential
```
Manjaro / Arch:
```
$ sudo pacman -S gcc
```

<br>

Clone the repository.
```
$ git clone https://github.com/kadir014/nova-physics.git
```

Set working directory as Nova Physics directory.
```
$ cd ./.../nova-physics
```

Run Nova Physics's build system with `build` command to start building. (Can also be `py` or `python3` depending on the platform.)
```
$ python nova_builder.py build
```

If successful, you should see results under `nova-physics/build` sub-directory.



# Running Nova Physics Example Demos
This section covers building & running examples.

### Running examples on Windows
Nova Physics, as a library, doesn't have any dependencies but the examples uses SDL2 for visuals. But the dependencies will be installed by the script automatically.

Set working directory as the Nova Physics directory.
```
cd "C:\...\nova-physics"
```

Run Nova Physics's build system with `example` command, it will detect the missing dependencies and download them on the fly.
```
python nova_builder.py example <example_name>
```

If successful, the example will be ran and you will see an SDL2 window open.

### Running examples on Linux
In Linux, you have to install the dependencies below either via your package manager or build it yourself. Nova Physics, as a library, doesn't have any dependencies but the examples uses SDL2 for visuals.
- [SDL2](https://github.com/libsdl-org/SDL/releases)
- [SDL2_ttf](https://github.com/libsdl-org/SDL_ttf/releases)
- [SDL2_image](https://github.com/libsdl-org/SDL_image)

Here are two sources explaining how to install SDL2 and the extensions on Linux:
- https://wiki.libsdl.org/SDL2/Installation
- https://lazyfoo.net/tutorials/SDL/01_hello_SDL/linux/index.php

<br>

After installing the dependencies, set working directory as Nova Physics directory.
```
$ cd ./.../nova-physics
```
Then run the build system with the `example` command. (Can also be `py` or `python3` depending on the platform.)
```
$ python nova_builder.py example <example_name>
```

If successful, the example will be ran and you will see an SDL2 window open.



# Running Nova Physics Benchmarks
TODO



# Running Nova Physics Test Suite
TODO