/*
    CLI Usage:
    vs.tmpl <template-file> <data-file> [namespace=`s:`]

    Unlike its usage in vs.fltk, template must be specified on its own.

    Alternative

    vs.tmpl [namespace=`s:`]
    
    with both files added via pipes, like `vs.tmpl <(cat template.xml) <(cat data.xml)
*/

#include "logging.hpp"
#include <format>
#include <pugixml.hpp>

#include <vs-templ.hpp>

#include <iostream>

using namespace vs::templ;

void logfn(log_t::values type, const char* msg, const log_t::ctx&ctx){
    static const char* severity_table[] = {
        "\033[31;1m[ERROR]\033[0m  : ",
        "\033[33;1m[WARNING]\033[0m: ",
        "\033[41;30;1m[PANIC]\033[0m  : ",
        "\033[34;1m[INFO]\033[0m   : ",
        "\033[37;1m[LOG]\033[0m    : ",
        "\032[34;1m[OK]\033[0m     : ",
    };

    fprintf(type<=vs::templ::log_t::INFO?stderr:stdout,"%s",std::format("\033\n[33;1m╭\033[0m {}{} \033\n[33;1m├\033[0m src:/{}\n\033[33;1m├\033[0m tmpl:/{}\n\033[33;1m╰\033[0m dst:/{}\n",severity_table[type],msg,ctx.data_path,ctx.template_path,ctx.generated_path).c_str());
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
    uint64_t seed = 0 ;

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

    preprocessor doc({data,tmpl,ns_prefix, logfn, includefn, loadfn, seed});
    auto& result = doc.parse();
    
    result.save(std::cout);

    return 0;
}