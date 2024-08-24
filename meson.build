project(
    'nova-physics',
    ['c', 'cpp'],
    license: 'MIT',
    license_files: 'LICENSE',
    default_options: [
        'c_std=c99',
        'cpp_std=c++11',
        'warning_level=1',
        'default_library=static'
    ]
)


compiler = meson.get_compiler('c')

cpp_args = []

if compiler.get_id() == 'msvc'
    c_args = ['/arch:AVX2', '/D_CRT_SECURE_NO_WARNINGS']
    link_args = []

    #c_args += ['/wd4305', '/wd4244']
    c_args += ['/wd4305']

    if get_option('enable_profiler')
        c_args += '/DNV_ENABLE_PROFILER'
    endif

    if get_option('use_doubles')
        c_args += '/DNV_USE_DOUBLE_PRECISION'
    endif

    if get_option('use_tracy')
        c_args += '/DTRACY_ENABLE'
        cpp_args += '/DTRACY_ENABLE'
        link_args += ['ws2_32.lib', 'wsock32.lib', 'dbghelp.lib']
    endif
else
    # -march=mavx2 ?
    c_args = ['-march=native']
    link_args = ['-lm']

    # When you target C99, you also have to specify POSIX clock
    # https://raspberrypi.stackexchange.com/a/95480
    c_args += '-D_POSIX_C_SOURCE=200809L'

    if get_option('enable_profiler')
        c_args += '-DNV_ENABLE_PROFILER'
    endif

    if get_option('use_doubles')
        c_args += '-DNV_USE_DOUBLE_PRECISION'
    endif

    if get_option('use_tracy')
        c_args += '-DTRACY_ENABLE'
        cpp_args += '-DTRACY_ENABLE'
        link_args += ['-lws2_32', '-lwsock32', '-ldbghelp']
    endif
endif


nova_src = [
    'src/core/array.c',
    'src/core/error.c',
    'src/core/hashmap.c',
    'src/core/pool.c',
    'src/constraints/constraint.c',
    'src/constraints/contact_constraint.c',
    'src/constraints/distance_constraint.c',
    'src/constraints/hinge_constraint.c',
    'src/constraints/spline_constraint.c',
    'src/body.c',
    'src/broadphase.c',
    'src/bvh.c',
    'src/collision.c',
    'src/contact.c',
    'src/narrowphase.c',
    'src/space.c',
    'src/shape.c'
]

nova_includes = ['include']

if get_option('use_tracy')
    nova_src += 'src/tracy/TracyClient.cpp'
    nova_includes += 'src/tracy'
endif

libnova = library(
    'nova',
    sources: nova_src,
    include_directories: nova_includes,
    c_args: c_args,
    cpp_args: cpp_args,
    link_args: link_args,
    version: '1.0.0',
)

python = find_program('python3', 'python')

if get_option('build_examples')

    r = run_command(python, 'scripts/install_wraps.py')
    if r.returncode() != 0
        error('install_wraps.py script failed.\n', r.stderr())
    endif

    examples_src = [
        'examples/main.c',
        'external/glad/glad.c'
    ]
    examples_includes = ['external', 'include']

    if get_option('use_tracy')
        examples_includes += 'src/tracy'
    endif

    sdl2_dep = dependency('sdl2')
    opengl_dep = dependency('gl')

    if compiler.get_id() == 'msvc'
        sdl2_main_dep = dependency('sdl2main')
        examples_deps = [sdl2_dep, sdl2_main_dep, opengl_dep]
    else
        examples_deps = [sdl2_dep, opengl_dep]
    endif

    executable(
        'examples',
        sources: examples_src,
        include_directories: examples_includes,
        c_args: c_args,
        link_args: link_args,
        dependencies: examples_deps,
        link_with: libnova
    )

    r = run_command(python, 'scripts/copy_assets.py')
    if r.returncode() != 0
        error('copy_assets.py script failed.\n', r.stderr())
    endif
    
endif


if get_option('build_benchmarks')

    benchmarks_src = ['benchmarks/main.c']
    benchmarks_includes = ['include']

    if get_option('use_tracy')
        benchmarks_includes += 'src/tracy'
    endif

    executable(
        'benchmarks',
        sources: benchmarks_src,
        include_directories: benchmarks_includes,
        c_args: c_args,
        link_args: link_args,
        link_with: libnova
    )

endif


if get_option('build_tests')

    tests_src = ['tests/main.c']
    tests_includes = ['include']

    if get_option('use_tracy')
        tests_includes += 'src/tracy'
    endif

    executable(
        'tests',
        sources: tests_src,
        include_directories: tests_includes,
        c_args: c_args,
        link_args: link_args,
        link_with: libnova
    )

endif