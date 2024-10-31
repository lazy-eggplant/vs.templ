project(
    'vs-templ',
    ['cpp'],
    default_options: ['cpp_std=c++17'],
)

pugixml_proj = subproject('pugixml')
pugixml_dep = pugixml_proj.get_variable('pugixml_dep')


vs_templ_lib= library('vs-templ', ['src/vs-templ.cpp'], dependencies: [pugixml_dep], include_directories: ['include'])


vs_templ_dep = declare_dependency(
  link_with: vs_templ_lib,
  include_directories: ['include'],
)

install_headers(
  'include/vs-templ.hpp',
)

pconf = import('pkgconfig')
pconf.generate(
  vs_templ_lib,
  description: 'Simple static template builder for XML files inspired by XSLT',
  url: 'https://pugixml.org/',
)


#if get_option('tests')
#  subdir('test')
#  vs_templ_check = executable('vs-templ-check', test_sources, dependencies: pugixml_dep)
#  test('vs-templ-check', pugixml_check, workdir: meson.current_source_dir())
#endif