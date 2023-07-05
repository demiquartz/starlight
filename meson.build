project(
    'starlight', 'c', 'cpp',
    default_options: [
        'buildtype=release',
        'warning_level=3',
        'c_std=c17',
        'cpp_std=c++20'
    ],
    license: 'AGPL-3.0',
    license_files: 'LICENSE',
    version: '0.0.0'
)

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
    'src/core/device.cpp',
    'src/core/window.cpp',
    'src/main.cpp',
    dependencies: [
        dependency('glfw3'),
        dependency('vulkan')
    ],
    gui_app: true
)
