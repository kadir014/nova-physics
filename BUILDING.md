# Nova Physics Build System Guidance
Welcome to the Nova Physics building guide.

Nova Physics uses a dependency-free Python script to simplify and organize building.

You can use `python nova_builder.py --help` to see the usage manual. Only requirement is to have Python 3.9 or higher installed on the system.


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

Download or clone the repository.
```
git clone https://github.com/kadir014/nova-physics.git
```

Set working directory as Nova Physics directory.
```
cd "C:\...\nova-physics"
```

Run Nova Physics's build system with `build` command to start building. To see options and arguments you can utilize for building use `--help`.
```
python nova_builder.py build
```

If successful, you should see the generated static libraries in the `nova-physics\build` sub-directory.


## <img src="https://i.imgur.com/J3TG2gm.png" width=20> Linux

❗ **Prerequisite**: Make sure you have GCC installed

<br>

Download or clone the repository.
```
$ git clone https://github.com/kadir014/nova-physics.git
```

Set working directory as Nova Physics directory.
```
$ cd ./.../nova-physics
```

Run Nova Physics's build system with `build` command to start building. (Can also be `py` or `python3` depending on the platform) To see options and arguments you can utilize for building use `--help`.
```
$ python nova_builder.py build
```

If successful, you should see the generated static libraries in `nova-physics/build` sub-directory.


<br><br><br>


# Running Nova Physics Example Demos
This section covers building & running examples.

## <img src="https://i.imgur.com/lyTfWAN.png" width=20> Windows
❗ **Prerequisite**: Make sure you have MinGW (GCC) or Visual Studio build tools installed. Dependencies like SDL2 will be installed automatically  by the script.

Download or clone the repository.
```
git clone https://github.com/kadir014/nova-physics.git
```

Set working directory as the Nova Physics directory.
```
cd "C:\...\nova-physics"
```

Run Nova Physics's build system with `examples` command, it will detect the missing dependencies and download them on the fly. To see options and arguments you can utilize for building use `--help`.
```
python nova_builder.py examples
```

If successful, the example demos will be ran and you will see an SDL2 window open.

## <img src="https://i.imgur.com/J3TG2gm.png" width=20> Linux
❗ **Prerequisite**: Make sure you have GCC, SDL2, SDL2-ttf and SDL2-image installed.
Here are two sources explaining how to install SDL2 and the extensions on Linux:
- https://wiki.libsdl.org/SDL2/Installation
- https://lazyfoo.net/tutorials/SDL/01_hello_SDL/linux/index.php

<br>

After installing the dependencies, download or clone the repository.
```
$ git clone https://github.com/kadir014/nova-physics.git
```

Set working directory as Nova Physics directory
```
$ cd ./.../nova-physics
```

Then run the build system with the `examples` command. (Can also be `py` or `python3` depending on the platform) To see options and arguments you can utilize for building use `--help`.
```
$ python nova_builder.py examples
```

If successful, the example demos will be ran and you will see an SDL2 window open.


<br><br><br>


# Running Nova Physics Benchmarks
This section covers building & running benchmarks.

## <img src="https://i.imgur.com/lyTfWAN.png" width=20> Windows
❗ **Prerequisite**: Make sure you have either MinGW (GCC) or Visual Studio build tools installed.

Download or clone the repository.
```
git clone https://github.com/kadir014/nova-physics.git
```

Set working directory as Nova Physics directory.
```
cd "C:\...\nova-physics"
```

Run Nova Physics's build system with `bench` command with a benchmark argument (e.g `boxes.c`) to start benchmarking. To see options and arguments you can utilize for building use `--help`.
```
python nova_builder.py bench <benchmark_source>
```

If successful, the benchmark will start running, you can see the progress and remaining time on terminal.


## <img src="https://i.imgur.com/J3TG2gm.png" width=20> Linux

❗ **Prerequisite**: Make sure you have GCC installed

<br>

Download or clone the repository.
```
$ git clone https://github.com/kadir014/nova-physics.git
```

Set working directory as Nova Physics directory.
```
$ cd ./.../nova-physics
```

Run Nova Physics's build system with `bench` command with a benchmark argument (e.g `boxes.c`) to start benchmarking. (Can also be `py` or `python3` depending on the platform) To see options and arguments you can utilize for building use `--help`.
```
$ python nova_builder.py bench <benchmark_source>
```

If successful, the benchmark will start running, you can see the progress and remaining time on terminal.


<br><br><br>


# Running Nova Physics Test Suite
This section covers building & running unit tests.

## <img src="https://i.imgur.com/lyTfWAN.png" width=20> Windows
❗ **Prerequisite**: Make sure you have either MinGW (GCC) or Visual Studio build tools installed.

Download or clone the repository.
```
git clone https://github.com/kadir014/nova-physics.git
```

Set working directory as Nova Physics directory.
```
cd "C:\...\nova-physics"
```

Run Nova Physics's build system with `tests` command to start running the test suite. To see options and arguments you can utilize for building use `--help`.
```
python nova_builder.py tests
```

If successful, the unit tests will be run and results will be shown on the terminal.


## <img src="https://i.imgur.com/J3TG2gm.png" width=20> Linux

❗ **Prerequisite**: Make sure you have GCC installed

<br>

Download or clone the repository.
```
$ git clone https://github.com/kadir014/nova-physics.git
```

Set working directory as Nova Physics directory.
```
$ cd ./.../nova-physics
```

Run Nova Physics's build system with `tests` command to start running the test suite. (Can also be `py` or `python3` depending on the platform) To see options and arguments you can utilize for building use `--help`.
```
$ python nova_builder.py tests
```

If successful, the unit tests will be run and results will be shown on the terminal.