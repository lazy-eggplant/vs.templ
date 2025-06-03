version_components = meson.project_version().split('.')
major_version = version_components[0]
minor_version = version_components[1]
rev_version = version_components[2]

conf = configuration_data()
conf.set('VS_TEMPL_VERSION_MAJOR', major_version)
conf.set('VS_TEMPL_VERSION_MINOR', minor_version)
conf.set('VS_TEMPL_VERSION_REV', rev_version)
conf.set_quoted('VS_TEMPL_VERSION', major_version + '.' + minor_version + '.' + rev_version)

cfgfile = configure_file(output : 'config.hpp',
  configuration : conf,
)

install_headers(
  [
    'vs-templ.hpp',
    'logging.hpp',
    'symbols.hpp',
    'utils.hpp',
    'module.modulemap',
    cfgfile
  ],
  subdir: 'vs-templ',
)
#install_subdir('.', install_dir : 'include/vs-templ', strip_directory: false, follow_symlinks: true, exclude_files: ['meson.build'] )
