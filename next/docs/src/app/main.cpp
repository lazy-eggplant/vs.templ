/*
    CLI Usage:
    vs.tmpl <template-file> <data-file> [namespace=`s:`]

    Unlike its usage in vs.fltk, template must be specified on its own.

    Alternative

    vs.tmpl [namespace=`s:`]
    
    with both files added via pipes, like `vs.tmpl <(cat template.xml) <(cat data.xml)
*/

#include <pugixml.hpp>
#include <vs-templ.hpp>

#include <iostream>

using namespace vs::templ;

//TODO: Support error logging on std::cerr. Maybe use VS_VERBOSE env variable to determine what is shown and if.
int main(int argc, const char* argv[]){
    const char* ns_prefix="s:";
    if(argc==0 || argc > 4){
        std::cerr<<"Wrong usage:\n\t"<<argv[0]<<" <template-file> <data-file> [namespace=`s:`]\nOR\t "<<argv[0]<<"[namespace=`s:`] and two input streams\n";
        exit(1);
    }

    pugi::xml_document data, tmpl;

    if(argc>=2){
        {auto t = tmpl.load_file(argv[1]); if(!t){std::cerr<<t.description()<<" @ `template file`\n";exit(2);}}
        {auto t = data.load_file(argv[2]); if(!t){std::cerr<<t.description()<<" @ `data file`\n";exit(3);}}

        if(argc>=4){ns_prefix=argv[3];}
        if(argc>=5){/*TODO: process random seed*/}
    }
    else{
        if(argc==2){ns_prefix=argv[1];}

        {auto t = tmpl.load(std::cin); if(!t){std::cerr<<t.description()<<" @ `template file`\n";exit(2);}}
        {auto t = data.load(std::cin); if(!t){std::cerr<<t.description()<<" @ `data file`\n";exit(3);}}

    }

    preprocessor doc(data,tmpl,ns_prefix);
    auto& result = doc.parse();

    
    for(auto& log : doc.logs()){
        if(log.type()==log_t::values::ERROR){
            std::cerr<<log.description()<<"\n";
        }
    }
    

    result.save(std::cout);

    return 0;
}