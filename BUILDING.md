# Nova Physics Build System Guidance
Welcome to the Nova Physics building guide.

Nova Physics uses a dependency-free Python script to simplify and organize building.

You can use `python nova_builder.py` without any arguments and commands to see usage manual. Only requirement is to have Python 3.9 or higher installed on the system.

<img src="https://raw.githubusercontent.com/kadir014/nova-physics/main/examples/assets/example_cli.png" width=450>

**What you can do with the build system:**
- [Build Nova Physics library](#building-nova-physics-library)
- [Build & run example demos](#running-examples-on-windows)
- [Build & run benchmarks](#running-nova-physics-benchmarks)
- [Build & run the test suite](#running-nova-physics-test-suite)


<br><br><br>


# Building Nova Physics Static Libraries
This section covers building and generating static libraries yourself (in case you don't use the already-built archives from the [latest release.](https://github.com/kadir014/nova-physics/releases/latest))

## <img src="https://i.imgur.com/lyTfWAN.png" width=20> Windows
❗ **Prerequisite**: Make sure you have either MinGW (GCC) or Visual Studio build tools installed.

Download the repository or clone it.
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

If successful, you should see the generated static libraries in the `nova-physics\build` sub-directory.


## <img src="https://i.imgur.com/J3TG2gm.png" width=20> Linux

❗ **Prerequisite**: Make sure you have GCC installed

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

If successful, you should see the generated static libraries in `nova-physics/build` sub-directory.


<br><br><br>


# Running Nova Physics Example Demos
This section covers building & running examples.

## <img src="https://i.imgur.com/lyTfWAN.png" width=20> Windows
❗ **Prerequisite**: Make sure you have MinGW (GCC) or Visual Studio build tools installed. Dependencies like SDL2 will be installed automatically  by the script.

Set working directory as the Nova Physics directory.
```
cd "C:\...\nova-physics"
```

Run Nova Physics's build system with `example` command, it will detect the missing dependencies and download them on the fly.
```
python nova_builder.py example <example_name>
```

If successful, the example will be ran and you will see an SDL2 window open.

## <img src="https://i.imgur.com/J3TG2gm.png" width=20> Linux
❗ **Prerequisite**: Make sure you have GCC, SDL2, SDL2-ttf and SDL2-image installed.
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


<br><br><br>


# Running Nova Physics Benchmarks
TODO


<br><br><br>


# Running Nova Physics Test Suite
TODO