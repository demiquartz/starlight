project(
    'starlight', 'c', 'cpp',
    default_options: [
        'buildtype=release',
        'warning_level=3',
        'c_std=c23',
        'cpp_std=c++23'
    ],
    license: 'AGPL-3.0',
    license_files: 'LICENSE',
    version: '0.0.0'
)

if get_option('buildtype').startswith('debug')
    add_project_arguments('-DDEBUG', language : ['c', 'cpp'])
    gui_app = false
else
    gui_app = true
endif

config = configuration_data()
config.set(
    'PROJECT_BUILD_ROOT',
    meson.global_build_root()
)
config.set(
    'PROJECT_SOURCE_ROOT',
    meson.global_source_root()
)
config.set(
    'PROJECT_NAME',
    meson.project_name()
)
config.set(
    'PROJECT_VERSION',
    meson.project_version()
)

configure_file(
    input: 'Doxyfile.in',
    output: '@BASENAME@',
    configuration: config
)

executable(
    'starlight',
    'src/core/config.cpp',
    'src/core/device.cpp',
    'src/core/window.cpp',
    'src/main.cpp',
    dependencies: [
        dependency('glfw3'),
        dependency('vulkan')
    ],
    gui_app: gui_app
)
