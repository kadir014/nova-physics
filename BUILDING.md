# Building Nova Physics
Nova Physics uses [Meson](https://mesonbuild.com/) as its build system.

If you're on macOS, install HomeBrew and Nova Physics's developer dependencies.
```shell
$ brew install glfw mesa sdl2 meson
```

Install meson. (if you have any issues refer to [meson's installation guide](https://mesonbuild.com/Getting-meson.html))
```shell
$ pip install meson
```

Change working dir to repository root.
```shell
$ cd path/to/nova-physics
```

Create a `build` directory for meson.
```shell
$ mkdir build
```

Setup meson configuration. For the other type of build types (optimization levels) see [here.](https://mesonbuild.com/Builtin-options.html#details-for-buildtype)
```shell
$ meson setup build --buildtype=debug
```

We can finally start compiling. If successful you should see the built targets in `build` directory.
```shell
$ meson compile -C build
```

# Building Options
While in the `build` dir (not root dir) you can use `$ meson configure` to see all building options and change them with `$ meson configure -Doption=bool`

Here's a quick summary of our custom build options (You can also check `meson.options` file for more details):
- `build_examples`: Build example demos, installs SDL2.
- `build_benchmarks`: Build bechmarks.
- `build_tests`: Build unit tests.
- `enable_profiler`: Enable built-in profiler. If enabled, `<windows.h>` can be included. (Defines `NV_ENABLE_PROFILER`.)
- `use_doubles`: Use double-precision floats. (Defines `NV_USE_DOUBLE_PRECISION`.)

For instance, if you didn't want to build example demos and just the static library, you could do `$ meson configure -Dbuild_examples=false`.


# Building For Web
You can build the static library for web with invoking `emconfigure` first and using the crossfile.
```shell
$ emconfigure meson setup build_web --cross-file wasm.ini
$ meson compile -C build_web
```

> [!IMPORTANT]
> You should not build any executables (such as example demos and benchmarks) when targeting web. Only build the static library. Refer to [Building Options](#building-options) to see how to configure.