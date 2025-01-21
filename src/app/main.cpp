/*
    CLI Usage:
    vs.tmpl <template-file> <data-file> [namespace=`s:`]

    Unlike its usage in vs.fltk, template must be specified on its own.

    Alternative

    vs.tmpl [namespace=`s:`]
    
    with both files added via pipes, like `vs.tmpl <(cat template.xml) <(cat data.xml)
*/

#include <format>
#include <pugixml.hpp>

#include <vs-templ.hpp>

#include <iostream>

using namespace vs::templ;

void logfn(log_t::values type, const char* msg, const logctx_t&){
    static const char* severity_table[] = {
    "\033[31;1m[ERROR]\033[0m    : ",
    "\033[33;1m[WARNING]\033[0m  : ",
    "\033[41;30;1m[PANIC]\033[0m    : ",
    "\033[34;1m[INFO]\033[0m     : ",
    };
    //TODO show context information
    std::cerr<<std::format("{}{} \033[33;1m@\033[0m {}",severity_table[type],msg,"xxx")<<"\n";
}

bool loadfn (const pugi::xml_node ctx,pugi::xml_document& ref){
    auto path = ctx.attribute("src").as_string(nullptr);
    if(path==nullptr)return false;
    
    auto result = ref.load_file(path);
    if(result.status==pugi::status_ok)return true;
    return false;
}

bool includefn (const char* path, pugi::xml_document& ref){
    auto result = ref.load_file(path);
    if(result.status==pugi::status_ok)return true;
    return false;
}

int main(int argc, const char* argv[]){
    //Workaround to avoid problems with pugixml... but it is not perfect. 
    //The solution is to never use `as_float` and always handle them externally.
    std::setlocale(LC_ALL, "C");

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

    preprocessor doc(data,tmpl,ns_prefix, logfn, includefn, loadfn, seed);
    auto& result = doc.parse();
    

    result.save(std::cout);

    return 0;
}