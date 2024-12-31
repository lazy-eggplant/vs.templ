vg = find_program('valgrind', required: false)
if vg.found()
    add_test_setup(
        'valgrind',
        exe_wrapper: [
            join_paths(meson.current_source_dir(), '../scripts/valgrind_wrap.sh'),
        ],
    )
endif

to_pass = [
    'eval-expr',
]
to_fail = []

foreach case : to_pass
    exe = executable(
        'test_marginal_' + case,
        './' + case + '.cpp',
        dependencies: [vs_templ_dep, pugixml_dep],
        cpp_args: '',
        install: false
    )
    test(
        'marginal-' + case,
        exe,
        env: [ ],
    )
endforeach

foreach case : to_fail
    exe = executable(
        case,
        './' + case + '.cpp',
        dependencies: [vs_templ_dep, pugixml_dep],
        cpp_args: '',
        install: false
    )
    test(
        'marginal-' + case,
        exe,
        should_fail: true,
        env: [ ],
    )
endforeach