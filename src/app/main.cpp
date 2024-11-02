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

#include <iostream>

using namespace vs::templ;

int main(int argc, const char* argv[]){
    const char* ns_prefix="s:";
    if(argc<3){
        std::cerr<<"Wrong usage:\n\t"<<argv[0]<<" <template-file> <data-file> [prefix=`s:`]\nOR\t "<<argv[0]<<"[prefix=`s:`] and two input streams\n";
        exit(1);
    }

    pugi::xml_document data, tmpl;
    data.load_file(argv[2]);
    tmpl.load_file(argv[1]);

    if(argc==4){ns_prefix=argv[3];}
    
    preprocessor doc(data,tmpl,ns_prefix);
    doc.parse().save(std::cout);

    /*
    for(auto& log : doc.logs){
        if(log.type==log_t::types::ERROR){
            std::cerr<<log;
        }
    }
    */
    
    return 0;
}