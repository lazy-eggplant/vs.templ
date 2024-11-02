#include <vs-templ.hpp>

namespace vs{
namespace templ{

std::optional<concrete_symbol> preprocessor::resolve_expr(const char* _str, const pugi::xml_node* base){
    int str_len = strlen(_str);
    char str[str_len+1];
    memcpy(str,_str,str_len+1);

    pugi::xml_node ref;
    int idx = 0;
    if(str[0]=='.' || str[0]=='+' || str[0]=='-' || (str[0]>'0' && str[0]<'9')) return atoi(_str);
    else if(str[0]=='#') return std::string(str+1);  //Consider what follows as a string
    else if(str[0]=='{'){
        int close = 0;
        for(;close<str_len && str[close]!='}';close++);
        str[close]=0;
        auto tmp = symbols.resolve(std::string_view(str+1,str+close));
        if(!tmp.has_value())return {};
        else if(std::holds_alternative<const pugi::xml_node>(tmp.value())){
            ref=std::get<const pugi::xml_node>(tmp.value());
        }
        else if(std::holds_alternative<int>(tmp.value())){
            return std::get<int>(tmp.value());
        }
        else if(std::holds_alternative<const pugi::xml_attribute>(tmp.value())){
            return std::get<const pugi::xml_attribute>(tmp.value());
        }
        if(close==str_len)return ref;        //End of string was met earlier
        idx=close+1;
    }
    else if(str[0]=='$'){

        if(base==nullptr){
            auto tmp = symbols.resolve("$");
            if(!tmp.has_value() || std::holds_alternative<const pugi::xml_node>(tmp.value())==false)return {};
            else{
                ref=std::get<const pugi::xml_node>(tmp.value());
            }
        }
        else ref=*base;

        if(str[1]==0)return ref;            //End of string was met earlier
        idx++;
    }
    else if(str[0]=='/'){
        ref=root_data;
        idx++;
    }

    //Recurse over **/ blocks
    for(;;){
        int close = idx;
        for(;close<str_len && str[close]!='/' && str[close]!='~';close++);
        char oldc=str[close];
        str[close]=0;
        if(idx!=close)ref = ref.child(str+idx);         //Avoid the prefix /
        if(close==str_len )return ref;                        //If the end of string was met earlier
        else if(oldc=='~'){str[close]=oldc;idx=close;break;}
        else{idx=close+1;}
    }
    //Process the terminal attributes and special properties name & text
    if(str[idx]=='~'){
        if(strcmp(str+idx+1,"!txt")==0) return ref.text().as_string();
        else if(strcmp(str+idx+1,"!tag")==0) return ref.name();
        else return ref.attribute(str+idx+1).as_string();
    }

    //TODO: Add command type to return the text() of a node as string.

    return {};
}

preprocessor::order_method_t::values preprocessor::order_method_t::from_string(const char* str){
    bool dot_eval=false;
    if(str[0]=='.')dot_eval=true;
    if(strcmp(str+dot_eval,"asc"))return (values)((dot_eval?USE_DOT_EVAL:UNKNOWN)|ASC);
    else if(strcmp(str+dot_eval,"desc"))return (values)((dot_eval?USE_DOT_EVAL:UNKNOWN)|DESC);
    else if(strcmp(str+dot_eval,"random"))return (values)((dot_eval?USE_DOT_EVAL:UNKNOWN)|RANDOM);
    else return order_method_t::UNKNOWN;
}

}
}