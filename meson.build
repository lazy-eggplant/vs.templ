project(
  'vs-templ',
  ['cpp'],
  version: '0.4.8',
  meson_version: '>= 1.1',
  default_options: ['cpp_std=c++20'],
)

version_components = meson.project_version().split('.')
major_version = version_components[0]
minor_version = version_components[1]
rev_version = version_components[2]

conf = configuration_data()
conf.set('VS_TEMPL_VERSION_MAJOR', major_version)
conf.set('VS_TEMPL_VERSION_MINOR', minor_version)
conf.set('VS_TEMPL_VERSION_REV', rev_version)
conf.set_quoted('VS_TEMPL_VERSION', major_version + '.' + minor_version + '.' + rev_version)

configure_file(output : 'config.hpp',
  configuration : conf,
)

cfgfile = custom_target('make-config',
  input : './build/config.hpp',
  output : ['config.hpp.fake-meson'],
  command : ['cp', '@INPUT0@', '../include/config.hpp'],
  capture : true,
  install : false,
  build_always_stale: true
)

add_project_arguments(['-Wno-vla-cxx-extension'], language: 'cpp')

cmake = import('cmake')

pugixml_dep = dependency('pugixml', version: '>=1.14', required: false)
if pugixml_dep.found() == false or host_machine.cpu() == 'wasm32'
  pugixml_opts = []
  if host_machine.cpu() == 'wasm32'
    pugixml_opts += ['no_exceptions=true']
  endif
  pugixml_proj = subproject('pugixml', default_options: pugixml_opts)
  pugixml_dep = pugixml_proj.get_variable('pugixml_dep')
endif

frozen_dep = dependency('frozen')
ff_dep = dependency('fast_float')
natsort_dep = dependency('natsort')

subdir(['./src/'])

pandoc = find_program('pandoc', required: false)
if pandoc.found()
  custom_target(
    'vs-templ-man',
    output: 'vs.templ.1',
    input: './docs/manual.md',
    command: ['pandoc', '@INPUT@', '-s', '-t', 'man', '-o', '@OUTPUT@'],
    install: not meson.is_subproject(),
    install_dir: join_paths(get_option('mandir'), 'man1'),
  )
endif

vs_templ_dep = declare_dependency(
  link_with: vs_templ_lib,
  include_directories: ['include'],
)

install_headers(
  [
    'include/config.hpp',
    'include/vs-templ.hpp',
    'include/logging.hpp',
    'include/symbols.hpp',
    'include/utils.hpp',
    'include/module.modulemap',
  ],
  subdir: 'vs-templ',
)

if get_option('tests')
  subdir(['./test/'])
endif

if get_option('benchmarks')
  subdir(['./benchmark/'])
endif

subdir(['./metadata/'])

pconf = import('pkgconfig')
pconf.generate(
  vs_templ_lib,
  description: 'Simple static template builder for XML files, inspired by XSLT',
  url: 'https://github.com/KaruroChori/vs-templ',
  version: meson.project_version(),
)