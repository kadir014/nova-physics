project(
    'nova-physics',
    'c',
    license: 'MIT',
    license_files: 'LICENSE',
    default_options: [
        'c_std=c99',
        'warning_level=1',
        'default_library=static'
    ]
)


compiler = meson.get_compiler('c')

if compiler.get_id() == 'msvc'
    c_args = ['/arch:AVX2', '/D_CRT_SECURE_NO_WARNINGS']

    #c_args += ['/wd4305', '/wd4244']

    if get_option('enable_profiler')
        c_args += '/DNV_ENABLE_PROFILER'
    endif

    if get_option('use_doubles')
        c_args += '/DNV_USE_DOUBLE_PRECISION'
    endif

    link_args = []
else
    # -march=mavx2 ?
    c_args = ['-march=native']

    # When you target C99, you also have to specify POSIX clock
    # https://raspberrypi.stackexchange.com/a/95480
    c_args += '-D_POSIX_C_SOURCE=200809L'

    if get_option('enable_profiler')
        c_args += '-DNV_ENABLE_PROFILER'
    endif

    if get_option('use_doubles')
        c_args += '-DNV_USE_DOUBLE_PRECISION'
    endif

    link_args = ['-lm']
endif


nova_src = [
    'src/core/array.c',
    'src/core/error.c',
    'src/core/hashmap.c',
    'src/constraints/constraint.c',
    'src/constraints/contact_constraint.c',
    'src/constraints/distance_constraint.c',
    'src/constraints/hinge_constraint.c',
    'src/constraints/spline_constraint.c',
    'src/body.c',
    'src/broadphase.c',
    'src/collision.c',
    'src/contact.c',
    'src/narrowphase.c',
    'src/space.c',
    'src/shape.c'
]

libnova = library(
    'nova',
    sources: nova_src,
    include_directories: 'include',
    c_args: c_args,
    link_args: link_args,
    version: '1.0.0',
)


if get_option('build_examples')

    r = run_command('python', 'scripts/install_wraps.py')
    if r.returncode() != 0
        error('install_wraps.py script failed.\n', r.stderr())
    endif

    examples_src = [
        'examples/main.c',
        'external/glad/glad.c'
    ]

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
        include_directories: ['external', 'include'],
        c_args: c_args,
        link_args: link_args,
        dependencies: examples_deps,
        link_with: libnova
    )

    r = run_command('python', 'scripts/copy_assets.py')
    if r.returncode() != 0
        error('copy_assets.py script failed.\n', r.stderr())
    endif
    
endif