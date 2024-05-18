project(
    'nova-physics',
    'c',
    license: 'MIT',
    license_files: 'LICENSE',
    default_options: [
        'c_std=c99',
        'warning_level=2',
        'default_library=static'
    ]
)


compiler = meson.get_compiler('c')

if compiler.get_id() == 'msvc'
    c_args = ['/arch:AVX2', '/DNV_PROFILE']
else
    # -march=mavx2 ?
    c_args = ['-march=native', '-DNV_PROFILE']
endif


nova_src = [
    'src/array.c',
    'src/body.c',
    'src/broadphase.c',
    'src/bvh.c',
    'src/collision.c',
    'src/constraint.c',
    'src/contact_solver.c',
    'src/contact.c',
    'src/distance_joint.c',
    'src/hashmap.c',
    'src/hinge_joint.c',
    'src/narrowphase.c',
    'src/resolution.c',
    'src/shape.c',
    'src/shg.c',
    'src/space_step.c',
    'src/space.c',
    'src/spring.c',
    'src/threading.c'
]

libnova = library(
    'nova',
    sources: nova_src,
    include_directories: 'include',
    c_args: c_args,
    version: '1.0.0',
)


if get_option('build_examples')

    r = run_command('python', 'scripts/install_wraps.py').returncode()
    if r != 0
        error('install_wraps.py script failed.\n', r.stderr())
    endif

    examples_src = [
        'examples/main.c',
        'external/glad/glad.c'
    ]

    sdl2_dep = dependency('sdl2')
    sdl2_main_dep = dependency('sdl2main')
    opengl_dep = dependency('gl')

    if host_machine.system() == 'windows'
        examples_deps = [sdl2_dep, sdl2_main_dep, opengl_dep]
    else
        examples_deps = [sdl2_dep, opengl_dep]
    endif

    executable(
        'examples',
        sources: examples_src,
        include_directories: ['external', 'include'],
        c_args: c_args,
        win_subsystem: 'windows',
        dependencies: examples_deps,
        link_with: libnova
    )
    
endif