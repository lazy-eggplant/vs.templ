metainfo_file = 'metainfo.xml'
ascli_exe = find_program('appstreamcli', required: false)
if ascli_exe.found()
  test(
    'validate metainfo file',
    ascli_exe,
    args: ['validate', '--no-net', '--pedantic', metainfo_file],
  )
endif

ascli_exe = find_program('appstreamcli')
custom_target(
  'gen-desktop-entry',
  input: [metainfo_file],
  output: ['com.karurochari.vs.templ.desktop'],
  command: [ascli_exe, 'make-desktop-file', '@INPUT@', '@OUTPUT@'],
  install: true,
  install_dir: join_paths(get_option('datadir'), 'applications'),
)

install_data(
  sources: ['vs.templ.light.svg'],
  rename: ['vs.templ.svg'],
  install_dir: join_paths(get_option('datadir'), 'icons', 'hicolor', 'scalable', 'apps'),
)

#i18n = import('i18n')

# NOTE: Remember to add the XML file to POTFILES.in!
#i18n.merge_file(
#  input: metainfo_file,
#  output: 'com.karurochari.vs.templ.metainfo.xml',
#  type: 'xml',
#  po_dir: join_paths(meson.project_source_root(), 'po'),
#    install: true,
#    install_dir: join_paths (get_option ('datadir'), 'metainfo')
#)
