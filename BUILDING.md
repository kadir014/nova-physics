# Building Nova Physics
Nova Physics uses [Meson](https://mesonbuild.com/) as its build system.

Install meson. (if you have any issues refer to [meson's installation guide](https://mesonbuild.com/Getting-meson.html))
```sh
$ pip install meson
```

Change dir to repository root.
```sh
$ cd .../nova-physics
```

Create a `build` directory for meson.
```sh
$ mkdir build
```

Setup meson configuration. For the other type of build types (optimization levels) see [here.](https://mesonbuild.com/Builtin-options.html#details-for-buildtype)
```sh
$ meson setup build --buildtype=debug
```

We can finally start compiling. If successful you should see the built targets in `build` directory.
```sh
$ meson compile -C build
```

# Building Options
While in the `build` dir (not root dir) you can use `$ meson configure` to see all building options and change them with `$ meson configure -Doption=nv_bool`

Here's a list of our custom build options:
- `build_examples`: Build example demos, installs SDL2. On by default.
- `enable_profiler`: Enable built-in profiler. (Defines NV_ENABLE_PROFILER, on by default)
- `enable_simd`: Enable usage of any SIMD extension. (Defines NV_ENABLE_SIMD, on by default)
- `use_doubles`: Use double-precision floats. (Defines NV_USE_DOUBLE_PRECISION, off by default)

For instance, if you didn't want to build example demos and just the static library, you could do `$ meson configure -Dbuild_examples=false`.
