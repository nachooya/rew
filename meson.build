# Project definition
project('rew', 'c',
    version: '0.5.0',
    default_options: ['c_std=c99'],
    meson_version: '>=0.46.0')

# Set compiler warning flags
compiler = meson.get_compiler('c')
compiler_args = compiler.get_supported_arguments([
    '-Wall',
    '-Wmissing-declarations',
    '-Wmissing-prototypes',
    '-Wstrict-prototypes',
    '-Wbad-function-cast',
    '-Wsign-compare',
    '-Wnested-externs',
    '-Wshadow',
    '-Waggregate-return',
    '-Wcast-align',
    '-Wextra',
    '-Wold-style-definition',
    '-Wdeclaration-after-statement',
    '-Wuninitialized',  # TODO: Only when optimising? But why?
    '-Wshorten-64-to-32',  # Apple-specific
    '-pedantic',
])
add_project_arguments(compiler_args, language: 'c')

# Configuration
# TODO: Generate a configuration file instead of defining? Would simplify the
#       configuration.
#       This would allow projects depending on this able to import those
#       defines, too.
configuration = configuration_data()

# Dependency: math
math_dep = compiler.find_library('m', required: false)

# Dependency: re
# TODO: Arbitrary version, ask maintainer
re_dep = dependency('re',
    version: '>=0.5.9',
    fallback: ['re', 're_dep'],
    required: true)

# Dependencies list
dependencies = [
    math_dep,
    re_dep,
]

# Features: IPv6
configuration.set('HAVE_INET6', 1)
add_project_arguments('-DHAVE_INET6', language: 'c')

# Includes
include_dir = include_directories('include')
subdir('include')

# Sources & Modules
# TODO: Make which to build configurable
subdir('src')

# Build library
rew = library('rew', sources,
    dependencies: dependencies,
    include_directories: include_dir,
    install: true,
    version: meson.project_version())

# Install headers
install_headers(includes, subdir: 'rew')

# Generate pkg-config file
pkg = import('pkgconfig')
pkg.generate(rew,
    name: 'librew',
    description: 'A sister library to libre with experimental code',
    url: 'https://github.com/alfredh/rew',
    subdirs: 'rew')

# TODO: Ensure 'install' has the same result as when invoking 'make'
