vs_templ_test = executable(
    'vs.templ-test',
    ['./main.cpp'],
    dependencies: [pugixml_dep, vs_templ_dep],
    install: false,
)

cases = ['id', 'for-range', 'for.tag', 'for-props.tag', 'calc', 'when.prop', 'complex-paths', 'tree-rewrite']

foreach case : cases

    test(
        case,
        vs_templ_test,
        args: [
            meson.current_source_dir() + './cases/' + case + '.xml',
        ],
    )

endforeach

subdir('marginal')