vs_templ_lib = library(
  'vs-templ',
  [
    'lib/vs-templ.cpp',
    'lib/utils.cpp',
    'lib/stack-lang.cpp',
  ],
  dependencies: [pugixml_dep, frozen_dep, ff_dep, natsort_dep],
  include_directories: ['../include'],
  install: not meson.is_subproject(),
)

vs_templ_cli = executable(
  'vs.templ',
  ['cli/main.cpp'],
  dependencies: [pugixml_dep],
  link_with: [vs_templ_lib],
  include_directories: ['../include'],
  install: not meson.is_subproject(),
)