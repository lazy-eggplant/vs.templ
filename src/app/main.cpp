/*
    CLI Usage:
    vs.tmpl TEMPLATE DATA [NAMESPACE='s']

    Unlike its usage in vs.fltk, template must be specified on its own.

    Alternative

    vs.tmpl [NAMESPACE='s']
    
    with both files added via pipes, like `vs.tmpl <(cat template.xml) <(cat data.xml)
*/

#include <pugixml.hpp>
#include <vs-templ.hpp>

int main(int argc, const char* argv[]){
    return 0;
}