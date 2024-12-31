project(
  'vs-templ',
  ['cpp'],
  version: '0.3.6',
  meson_version: '>= 1.1',
  default_options: ['cpp_std=c++20'],
)

pugixml_dep = dependency('pugixml', version: '>=1.14', required: false)
if pugixml_dep.found() == false
  pugixml_proj = subproject('pugixml')
  pugixml_dep = pugixml_proj.get_variable('pugixml_dep')
endif

vs_templ_lib = library(
  'vs-templ-lib',
  [
    'src/vs-templ.cpp',
    'src/utils.cpp',
    'src/symbols.cpp',
    'src/logging.cpp',
    'src/stack-lang.cpp',
  ],
  dependencies: [pugixml_dep],
  include_directories: ['include'],
  install: not meson.is_subproject(),
)

vs_templ_cli = executable(
  'vs.templ',
  ['src/app/main.cpp'],
  dependencies: [pugixml_dep],
  link_with: [vs_templ_lib],
  include_directories: ['include'],
  install: not meson.is_subproject(),
)

pandoc = find_program('pandoc', required: false)
if pandoc.found()
  custom_target(
    'vs-templ-man',
    output: 'vs-templ.1',
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

subdir(['./metadata/'])

pconf = import('pkgconfig')
pconf.generate(
  vs_templ_lib,
  description: 'Simple static template builder for XML files, inspired by XSLT',
  url: 'https://github.com/KaruroChori/vs-templ',
  version: meson.project_version(),
)