#include <vs-templ.hpp>

namespace vs{
namespace templ{

void preprocessor::init(const pugi::xml_document& root_data, const pugi::xml_document& root_template,const char* prefix){
            stack_template.push({root_template.root().begin(),root_template.root().end()});
            stack_compiled.push(compiled.root());
            this->root_data=root_data.root();
            ns(prefix);
        }

void preprocessor::reset(){
    symbols.reset();
    stack_template=decltype(stack_template)();
    stack_data=decltype(stack_data)();
    stack_compiled=decltype(stack_compiled)();
    logs=decltype(logs)();
}

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


void preprocessor::ns_strings::prepare(const char * ns_prefix){
#   define WRITE(name,value) name=data+count;memcpy(data+count,ns_prefix,ns_prefix_len);memcpy(data+count+ns_prefix_len,value,std::char_traits<char>::length(value));data[count+ns_prefix_len+std::char_traits<char>::length(value)]=0;count+=ns_prefix_len+std::char_traits<char>::length(value)+1;
#   define STRLEN(str) ns_prefix_len+std::char_traits<char>::length(str)+1

    size_t ns_prefix_len=strlen(ns_prefix);

    if(data!=nullptr)delete []data;
    data = new char[
        STRLEN("for-range")+
        STRLEN("for")+STRLEN("for-props")+STRLEN("empty")+STRLEN("header")+STRLEN("footer")+STRLEN("item")+STRLEN("error")+
        STRLEN("when")+STRLEN("is")+
        STRLEN("value")+
        STRLEN("element")+STRLEN("type")+

        STRLEN("for.in")+STRLEN("for.filter")+STRLEN("for.sort-by")+STRLEN("for.order-by")+STRLEN("for.offset")+STRLEN("for.limit")+
        STRLEN("for-props.in")+STRLEN("for-props.filter")+STRLEN("for.order-by")+STRLEN("for-props.offset")+STRLEN("for-props.limit")+
        
        STRLEN("value.src")+STRLEN("value.format")+
        STRLEN("use.src")
        ];
    int count=0;
    
    WRITE(FOR_RANGE_TAG,"for-range");

    WRITE(FOR_TAG,"for");
    WRITE(FOR_PROPS_TAG,"for-props");
        WRITE(EMPTY_TAG,"empty");
        WRITE(HEADER_TAG,"header");
        WRITE(FOOTER_TAG,"footer");
        WRITE(ITEM_TAG,"item");
        WRITE(ERROR_TAG,"error");

    WRITE(WHEN_TAG,"when");
        WRITE(IS_TAG,"is");

    WRITE(VALUE_TAG,"value");
    WRITE(ELEMENT_TAG,"element");
        WRITE(TYPE_ATTR, "type");

    WRITE(FOR_IN_PROP,"for.in");
    WRITE(FOR_FILTER_PROP,"for.filter");
    WRITE(FOR_SORT_BY_PROP,"for.sort-by");
    WRITE(FOR_ORDER_BY_PROP,"for.order-by");
    WRITE(FOR_OFFSET_PROP,"for.offset")
    WRITE(FOR_LIMIT_PROP,"for.limit");


    WRITE(FOR_PROPS_IN_PROP,"for.in");
    WRITE(FOR_PROPS_FILTER_PROP,"for.filter");
    WRITE(FOR_PROPS_ORDER_BY_PROP,"for.order-by");
    WRITE(FOR_PROPS_OFFSET_PROP,"for.offset");
    WRITE(FOR_PROPS_LIMIT_PROP,"for.limit");
        
    WRITE(VALUE_SRC_PROP,"value.src");
    WRITE(VALUE_FORMAT_PROP,"value.format");
    
    WRITE(USE_SRC_PROP,"use.src");
#   undef WRITE
#   undef STRLEN
}



std::vector<pugi::xml_attribute> preprocessor::prepare_props_data(const pugi::xml_node& base, int limit, int offset, bool(*filter)(const pugi::xml_attribute&), order_method_t::values criterion){
    auto cmp_fn = [&](const pugi::xml_attribute& a, const pugi::xml_attribute& b)->int{
        if(criterion==order_method_t::ASC){
            int cmp =  strcmp(a.name(),b.name());
            if(cmp==-1)return true;
            else return false;
        }
        else if(criterion==order_method_t::DESC){
            int cmp =  strcmp(a.name(),b.name());
            if(cmp==1)return true;
            else return false;
        }
        else{
            //TODO: methods not implemented. The dot variants are only valid for strings or string-like content. They uses `.` to nest the search in blocks, like for prop names.
            //Random is based on the hash of the value. It requires to be stable: as such, a fast hashing function is needed (externally supplied, C++ has none).
        }
        
        return false;
    };
    
    std::vector<pugi::xml_attribute> dataset;
    for(auto& child: base.attributes()){
        if(filter==nullptr || filter(child))dataset.push_back(child);
    }

    std::sort(dataset.begin(),dataset.end(),cmp_fn);

    //TODO: Check if these boudary condition are sound.
    if(offset<0)offset=0;
    else if(offset>=dataset.size())return {};
    if(limit>0 && offset+limit>dataset.size())limit=dataset.size()-offset;
    else if(limit<0 && dataset.size()+limit<=offset)return {};

    else if(limit<=0)return std::vector(dataset.begin()+offset, dataset.end()-limit);
    else return std::vector(dataset.begin()+offset, dataset.begin()+offset+limit);

    return {};
}

std::vector<pugi::xml_node> preprocessor::prepare_children_data(const pugi::xml_node& base, int limit, int offset, bool(*filter)(const pugi::xml_node&), const std::vector<std::pair<std::string,order_method_t::values>>& criteria){
    auto cmp_fn = [&](const pugi::xml_node& a, const pugi::xml_node& b)->int{
        for(auto& criterion: criteria){
            auto valA = resolve_expr(criterion.first.c_str(),&a);
            auto valB = resolve_expr(criterion.first.c_str(),&b);

            if(criterion.second==order_method_t::ASC){
                if(valA<valB)return true;
                else if(valA>valB) return false;
            }
            else if(criterion.second==order_method_t::DESC){
                if(valA<valB)return false;
                else if(valA>valB) return true;
            }
            else{
                //TODO: methods not implemented. The dot variants are only valid for strings or string-like content. They uses `.` to nest the search in blocks, like for prop names.
                //Random is based on the hash of the value. It requires to be stable: as such, a fast hashing function is needed (externally supplied, C++ has none).

            }
        }
        return false;
    };
    
    std::vector<pugi::xml_node> dataset;
    for(auto& child: base.children()){
        if(filter==nullptr || filter(child))dataset.push_back(child);
    }

    std::sort(dataset.begin(),dataset.end(),cmp_fn);

    //TODO: Check if these boudary condition are sound.
    if(offset<0)offset=0;
    else if(offset>=dataset.size())return {};
    if(limit>0 && offset+limit>dataset.size())limit=dataset.size()-offset;
    else if(limit<0 && dataset.size()+limit<=offset)return {};

    else if(limit<=0)return std::vector(dataset.begin()+offset, dataset.end()-limit);
    else return std::vector(dataset.begin()+offset, dataset.begin()+offset+limit);

    return {};
}


}
}