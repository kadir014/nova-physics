# Building Nova Physics
Nova Physics is mainly a C99 library. However, it also offers benchmarks, unit tests and example demos using SDL2 & OpenGL if opted in while building.

Instead of CMake or other legacy workflows, Nova Physics uses [Meson](https://mesonbuild.com/) as its build system. So you can configure what you exactly want to build easily.

This document explains the building process thoroughly, step by step. However, you can also jump to specific sections to use it as a reference.

- [Prerequisites](#prerequisites)
- [Building](#building)
  - [Building Only Static Library](#building-only-static-library)
  - [Building For Web](#building-for-web)
- [Build Options](#build-options)
- [Troubleshooting](#troubleshooting)



# Prerequisites
- [Meson build system](https://mesonbuild.com/Getting-meson.html)
- [Python 3.12+](https://www.python.org/downloads/)
- A compiler that supports C99 standard

If you're on macOS, you can use HomeBrew for developer dependencies.
```shell
$ brew install glfw mesa sdl2 meson
```

You can also use pip to install Meson quickly. Or just head to [Meson's own installation guide](https://mesonbuild.com/Getting-meson.html).
```shell
$ pip install meson
```



# Building
**1.** Clone the repository in your working space (or download the latest release from [Nova Physics Releases](https://github.com/kadir014/nova-physics/releases)).
```shell
$ git clone https://github.com/kadir014/nova-physics.git
$ cd nova-physics
```

**2.** Setup meson configuration. For the other type of build types (optimization levels) see [here.](https://mesonbuild.com/Builtin-options.html#details-for-buildtype)
```shell
$ meson setup build --buildtype=debug
```

There are four build targets. Static library, example demos, benchmarks and unit tests. Each one of them can be disabled or enabled with build options. See [building only static library](#building-only-static-library) or [build options](#build-options) for more information.

**3.** We can finally start compiling. If successful you should see the built targets in `build` directory.
```shell
$ cd build
$ meson compile
```

**4.** Run the example demos and play around!
```shell
$ ./examples
```

## Building Only Static Library
The default configuration builds every target. That includes the example binary that requires SDL2. If you want to use Nova Physics in your application and only need the static library you can configure your meson setup like so:
```shell
$ cd build
$ meson configure -Dbuild_examples=false -Dbuild_benchmarks=false -Dbuild_tests=false
```

Once built, you will only end up with `libnova.a`.

## Building For Web
Assuming you have the [Emscripten SDK](https://emscripten.org/docs/compiling/Building-Projects.html) installed, You can build the static library for web with invoking `emconfigure` first and using the crossfile.
```shell
$ emconfigure meson setup build_web --cross-file wasm.ini -Dbuild_examples=false -Dbuild_benchmarks=false -Dbuild_tests=false
$ meson compile -C build_web
```

> [!IMPORTANT]
> Notice the disabled build targets. You should not build any executables (such as example demos and benchmarks) when targeting web. Only build the static library.



# Build Options
Nova Physics has four build targets and more build options to configure how the library is setup. For example disabling profiler and using double-precision floats.

While inside your build directory (not root directory), you can use `$ meson configure` to see all building options and change them with `$ meson configure -Doption=bool`.

For a full list of options, check out the [meson.options](meson.options) file.

For instance, if you didn't want to build example demos and enable double-precision floats, you could do `$ meson configure -Dbuild_examples=false -Duse_doubles=true`.



# Troubleshooting
- On Windows, you may need to run `meson setup ...` twice for SDL2 (and other dependencies) to download & install properly if you saw something along the lines of `ERROR: Dependency lookup for ... with method 'pkgconfig' failed`. I have no idea why this happens.
- On Linux, the display protocol might break key inputs (for the SDL2 example demos). Try forcing either `wayland` or `x11` while launching the binary:
  ```shell
  $ SDL_VIDEODRIVER=x11 ./examples
  ```