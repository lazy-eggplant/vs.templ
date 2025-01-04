/*
    CLI Usage:
    vs.tmpl <template-file> <data-file> [namespace=`s:`]

    Unlike its usage in vs.fltk, template must be specified on its own.

    Alternative

    vs.tmpl [namespace=`s:`]
    
    with both files added via pipes, like `vs.tmpl <(cat template.xml) <(cat data.xml)
*/

#include "logging.hpp"
#include <pugixml.hpp>
#include <vs-templ.hpp>

#include <iostream>

using namespace vs::templ;

void logfn(log_t::values, const char* msg, const logctx_t&){
    std::cerr<<msg<<"\n";
    //TODO: Add contextual info
}

//TODO: Support error logging on std::cerr. Maybe use VS_VERBOSE env variable to determine what is shown and if.
int main(int argc, const char* argv[]){
    const char* ns_prefix="s:";
    if(argc==0 || argc > 4){
        std::cerr<<"Wrong usage:\n\t"<<argv[0]<<" <template-file> <data-file> [namespace=`s:`]\nOR\t "<<argv[0]<<"[namespace=`s:`] and two input streams\n";
        exit(1);
    }

    pugi::xml_document data, tmpl;
    int seed = 0 ;

    if(argc>=2){
        {auto t = tmpl.load_file(argv[1]); if(!t){std::cerr<<t.description()<<" @ `template file`\n";exit(2);}}
        {auto t = data.load_file(argv[2]); if(!t){std::cerr<<t.description()<<" @ `data file`\n";exit(3);}}

        if(argc>=4){ns_prefix=argv[3];}
        if(argc>=5){seed=atoi(argv[4]);}
    }
    else{
        if(argc==2){ns_prefix=argv[1];}

        {auto t = tmpl.load(std::cin); if(!t){std::cerr<<t.description()<<" @ `template file`\n";exit(2);}}
        {auto t = data.load(std::cin); if(!t){std::cerr<<t.description()<<" @ `data file`\n";exit(3);}}

    }

    preprocessor doc(data,tmpl,ns_prefix, logfn, +[](const char* path, pugi::xml_document& ref){auto result = ref.load_file(path);if(result.status==pugi::status_ok)return true;return false;}, seed);
    auto& result = doc.parse();
    

    result.save(std::cout);

    return 0;
}