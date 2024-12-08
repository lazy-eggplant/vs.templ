vs_templ_test = executable(
    'vs.templ-test',
    ['./main.cpp'],
    dependencies: [pugixml_dep, vs_templ_dep],
    install: false,
)

test('for-range', vs_templ_test, args: ['./cases/for-range.xml'])