#include <algorithm>
#include <string>
#include <string_view>
#include <variant>
#include <vs-templ.hpp>
#include <stack-lang.hpp>
#include <format>

#include "logging.hpp"
#include "utils.hpp"

namespace vs{
namespace templ{

const float EPS = 10e-5;

void preprocessor::init(const pugi::xml_node& root_data, const pugi::xml_node& root_template,const char* prefix, logfn_t _logfn, loadfn_t _loadfn, uint64_t seed){
    if(_logfn!=nullptr)logfn=_logfn;
    if(_loadfn!=nullptr)loadfn=_loadfn;

    stack_template.emplace(root_template.begin(),root_template.end());
    stack_compiled.emplace(compiled);
    this->root_data=root_data;
    this->seed=seed;
    symbols.set("$",root_data);
    ns(prefix);
}

void preprocessor::reset(){
    symbols.reset();
    stack_template=decltype(stack_template)();
    stack_data=decltype(stack_data)();
    stack_compiled=decltype(stack_compiled)();
}

void preprocessor::log(log_t::values type, const std::string& str) const{
    //TODO: Add prefix & stuff in here like for vs.fltk
    logctx_t ctx;
    logfn(type,str.data(),ctx);
}

//TODO: The next release of pugixml will have support for string_views directly. 
//      As such, the double buffer to add \0 around will no longer be needed.
//      Once that is done, please revise this function to make it much faster.
std::optional<concrete_symbol> preprocessor::resolve_expr(const std::string_view& _str, const pugi::xml_node* base) const{
    int str_len = _str.size(); 
    char str[str_len+1];
    memcpy(str,_str.data(),str_len+1);
    str[str_len]=0;

    pugi::xml_node ref;
    int idx = 0;
    if(str[0]=='.' || str[0]=='+' || str[0]=='-' || (str[0]>'0' && str[0]<'9')){
        if(_str[_str.length()-1]=='f')return (float)atof(str);
        else return atoi(str);
    }
    else if(str[0]==':') {
        repl r(*this);
        return r.eval(str+1);
    }
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

    return {};
}

preprocessor::order_method_t::values preprocessor::order_method_t::from_string(std::string_view str){
    bool dot_eval=false;
    if(str[0]=='.')dot_eval=true;
    if((std::string_view(str.begin()+dot_eval, str.end()) == std::string_view("asc")))return (values)((dot_eval?USE_DOT_EVAL:UNKNOWN)|ASC);
    else if((std::string_view(str.begin()+dot_eval, str.end()) == std::string_view("desc")))return (values)((dot_eval?USE_DOT_EVAL:UNKNOWN)|DESC);
    else if((std::string_view(str.begin()+dot_eval, str.end()) == std::string_view("random")))return (values)((dot_eval?USE_DOT_EVAL:UNKNOWN)|RANDOM);
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
        STRLEN("test")+STRLEN("case")+
        STRLEN("value")+
        STRLEN("eval")+
        STRLEN("element")+STRLEN("type")+
        STRLEN("log")+
        STRLEN("include")+

        STRLEN("for.src")+STRLEN("for.filter")+STRLEN("for.sort-by")+STRLEN("for.order-by")+STRLEN("for.offset")+STRLEN("for.limit")+
        STRLEN("for-props.src")+STRLEN("for-props.filter")+STRLEN("for.order-by")+STRLEN("for-props.offset")+STRLEN("for-props.limit")+
        
        STRLEN("value")+
        STRLEN("prop")+
        STRLEN("when")
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

    WRITE(TEST_TAG,"test");
        WRITE(CASE_TAG,"case");

    WRITE(VALUE_TAG,"value");
    WRITE(ELEMENT_TAG,"element");
        WRITE(TYPE_ATTR, "type");

    WRITE(LOG_TAG,"log");
    WRITE(INCLUDE_TAG,"include");

    WRITE(FOR_SRC_PROP,"for.src");
    WRITE(FOR_FILTER_PROP,"for.filter");
    WRITE(FOR_SORT_BY_PROP,"for.sort-by");
    WRITE(FOR_ORDER_BY_PROP,"for.order-by");
    WRITE(FOR_OFFSET_PROP,"for.offset")
    WRITE(FOR_LIMIT_PROP,"for.limit");


    WRITE(FOR_PROPS_SRC_PROP,"for.src");
    WRITE(FOR_PROPS_FILTER_PROP,"for.filter");
    WRITE(FOR_PROPS_ORDER_BY_PROP,"for.order-by");
    WRITE(FOR_PROPS_OFFSET_PROP,"for.offset");
    WRITE(FOR_PROPS_LIMIT_PROP,"for.limit");
        
    WRITE(VALUE_PROP,"value"); 

    WRITE(PROP_PROP,"prop");

    WRITE(PROP_PROP,"when");


#   undef WRITE
#   undef STRLEN
}

std::vector<pugi::xml_attribute> preprocessor::prepare_props_data(const pugi::xml_node& base, int limit, int offset, const char *filter, order_method_t::values criterion){
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
    {
        symbols.guard();
        for(auto& child: base.attributes()){
            if(filter!=nullptr){
                repl testexpr(*this);
                auto retexpr = testexpr.eval(filter+1);
                if(std::holds_alternative<int>(retexpr.value_or(true))==false)continue; //Skip logic.
                if(std::get<int>(retexpr.value_or(true))==false)continue; //Skip logic.
            }

            dataset.push_back(child);
        }
    }

    std::sort(dataset.begin(),dataset.end(),cmp_fn);

    //TODO: Check if these boudary condition are sound.
    if(offset<0)offset=0;
    else if(offset>=(int)dataset.size())return {};
    if(limit>0 && offset+limit>(int)dataset.size())limit=dataset.size()-offset;
    else if(limit<0 && (int)dataset.size()+limit<=offset)return {};

    else if(limit<=0)return std::vector(dataset.begin()+offset, dataset.end()-limit);
    else return std::vector(dataset.begin()+offset, dataset.begin()+offset+limit);

    return {};
}

std::vector<pugi::xml_node> preprocessor::prepare_children_data(const pugi::xml_node& base, int limit, int offset, const char* filter, const std::vector<std::pair<std::string,order_method_t::values>>& criteria){
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
            else if(criterion.second==(order_method_t::ASC | order_method_t::USE_DOT_EVAL)){
                if(valA.has_value() && std::holds_alternative<std::string>(valA.value()) && valB.has_value() && std::holds_alternative<std::string>(valB.value())){
                    const std::string& strA = std::get<std::string>(valA.value());
                    const std::string& strB = std::get<std::string>(valB.value());

                    auto i = cmp_dot_str(strA.c_str(), strB.c_str());
                    if(i<0)return true;
                    else if(i>0)return false;
                }
                else return false;
            }
            else if(criterion.second==(order_method_t::DESC | order_method_t::USE_DOT_EVAL)){
                if(valA.has_value() && std::holds_alternative<std::string>(valA.value()) && valB.has_value() && std::holds_alternative<std::string>(valB.value())){
                    const std::string& strA = std::get<std::string>(valA.value());
                    const std::string& strB = std::get<std::string>(valB.value());

                    auto i = cmp_dot_str(strA.c_str(), strB.c_str());
                    if(i>0)return true;
                    else if(i<0)return false;
                }
                else return false;
            }
            else{
                //TODO: methods not implemented. The dot variants are only valid for strings or string-like content. They uses `.` to nest the search in blocks, like for prop names.
                //Random is based on the hash of the value. It requires to be stable: as such, a fast hashing function is needed (externally supplied, C++ has none).

            }
        }
        return false;
    };
    
    std::vector<pugi::xml_node> dataset;
    {
        symbols.guard();
        for(auto& child: base.children()){
            symbols.set("$",child);
            if(filter!=nullptr){
                repl testexpr(*this);
                auto retexpr = testexpr.eval(filter+1);
                if(std::holds_alternative<int>(retexpr.value_or(true))==false)continue; //Skip logic.
                if(std::get<int>(retexpr.value_or(true))==false)continue; //Skip logic.
            }
            dataset.push_back(child);
        }
    }
    std::sort(dataset.begin(),dataset.end(),cmp_fn);

    //TODO: Check if these boudary condition are sound.
    if(offset<0)offset=0;
    else if(offset>=(int)dataset.size())return {};
    if(limit>0 && offset+limit>(int)dataset.size())limit=dataset.size()-offset;
    else if(limit<0 && (int)dataset.size()+limit<=offset)return {};

    else if(limit<=0)return std::vector(dataset.begin()+offset, dataset.end()-limit);
    else return std::vector(dataset.begin()+offset, dataset.begin()+offset+limit);

    return {};
}

void preprocessor::_parse(std::optional<pugi::xml_node_iterator> stop_at){ 
    
    while(!stack_template.empty()){

        auto& current_template = stack_template.top();
        auto& current_compiled = stack_compiled.top();

        if(stop_at.has_value() && current_template.first==stop_at)break;

        if(current_template.first!=current_template.second){

            //Special handling of static element
            if(strncmp(current_template.first->name(),ns_prefix.c_str(),ns_prefix.length())==0) {
                if(strcmp(current_template.first->name(),strings.FOR_RANGE_TAG)==0){
                    const char* tag = current_template.first->attribute("tag").as_string();
                    int from = get_or<int>(resolve_expr(current_template.first->attribute("from").as_string("0")).value_or(0),0);
                    int to = get_or<int>(resolve_expr(current_template.first->attribute("to").as_string("0")).value_or(0),0);
                    int step = get_or<int>(resolve_expr(current_template.first->attribute("step").as_string("1")).value_or(1),1);
                    if(step>0 && to<from){/* Skip infinite loop*/}
                    else if(step<0 && to>from){/* Skip infinite loop*/}
                    else if(step==0){/* Skip potentially infinite loop*/}
                    else for(int i=from; i<to; i+=step){
                        auto frame_guard = symbols.guard();
                        if(tag!=nullptr)symbols.set(tag,i);
                        symbols.set("$",i);
                        stack_template.emplace(current_template.first->begin(),current_template.first->end());
                        _parse(current_template.first);
                        //When exiting one too many is removed. restore it.
                        stack_compiled.emplace(current_compiled);
                    }
                }
                else if(strcmp(current_template.first->name(),strings.FOR_TAG)==0){
                    const char* tag = current_template.first->attribute("tag").as_string();
                    const char* in = current_template.first->attribute("in").as_string(current_template.first->attribute("src").as_string());

                    //TODO: filter has not defined syntax yet.
                    const char* filter = current_template.first->attribute("filter").as_string(nullptr);
                    const char* _sort_by = current_template.first->attribute("sort-by").as_string();
                    const char* _order_by = current_template.first->attribute("order-by").as_string("asc");

                    int limit = get_or<int>(resolve_expr(current_template.first->attribute("limit").as_string("0")).value_or(0),0);
                    int offset = get_or<int>(resolve_expr(current_template.first->attribute("offset").as_string("0")).value_or(0),0);

                    auto expr = resolve_expr(in);

                    //Only a node is acceptable in this context, otherwise show the error
                    if(!expr.has_value() || !std::holds_alternative<const pugi::xml_node>(expr.value())){ 
                        for(const auto& el: current_template.first->children(strings.ERROR_TAG)){
                            stack_template.emplace(el.begin(),el.end());
                            _parse(current_template.first);
                            stack_compiled.emplace(current_compiled);
                        }
                    }
                    else{
                        std::vector<std::pair<std::string,order_method_t::values>> criteria;
                        //Build criteria
                        {
                            auto orders = split_string(_order_by,'|');
                            int c = 0;
                            //Apply order directive with wrapping in case not enough cases are specified.
                            for(auto& i:split_string(_sort_by,'|')){
                                criteria.emplace_back(i,order_method_t::from_string(orders[c%orders.size()]));
                                c++;
                            }
                        }
                        auto good_data = prepare_children_data(std::get<const pugi::xml_node>(expr.value()), limit, offset, filter, criteria);

                        if(good_data.size()==0){
                            for(const auto& el: current_template.first->children(strings.EMPTY_TAG)){
                                stack_template.emplace(el.begin(),el.end());
                                _parse(current_template.first);
                                stack_compiled.emplace(current_compiled);
                            }
                        }
                        else{
                            //Header (once)
                            {
                                for(const auto& el: current_template.first->children(strings.HEADER_TAG)){
                                    stack_template.emplace(el.begin(),el.end());
                                    _parse(current_template.first);
                                    stack_compiled.emplace(current_compiled);
                                }
                            }
                        
                            //Items (iterate)
                            int counter = 0;
                            for(auto& i : good_data){
                                auto frame_guard = symbols.guard();
    
                                if(tag!=nullptr)symbols.set(tag,i);
                                symbols.set("$",i);
                                symbols.set("$$",counter);

                                for(const auto& el: current_template.first->children(strings.ITEM_TAG)){
                                    stack_template.emplace(el.begin(),el.end());
                                    _parse(current_template.first);
                                    stack_compiled.emplace(current_compiled);
                                }
                                counter++;
                            }

                            //Footer (once)
                            {
                                for(const auto& el: current_template.first->children(strings.FOOTER_TAG)){
                                    stack_template.emplace(el.begin(),el.end());
                                    _parse(current_template.first);
                                    stack_compiled.emplace(current_compiled);
                                }
                            }
                        }
                    }
                }
                else if(strcmp(current_template.first->name(),strings.FOR_PROPS_TAG)==0){
                    const char* tag = current_template.first->attribute("tag").as_string();
                    const char* in = current_template.first->attribute("in").as_string(current_template.first->attribute("src").as_string());

                    //TODO: filter has not defined syntax yet.
                    const char* filter = current_template.first->attribute("filter").as_string(nullptr);
                    const char* _order_by = current_template.first->attribute("order-by").as_string("asc");

                    int limit = get_or<int>(resolve_expr(current_template.first->attribute("limit").as_string("0")).value_or(0),0);
                    int offset = get_or<int>(resolve_expr(current_template.first->attribute("offset").as_string("0")).value_or(0),0);
                    
                    auto expr = resolve_expr(in);

                    //Only a node is acceptable in this context, otherwise show the error
                    if(!expr.has_value() || !std::holds_alternative<const pugi::xml_node>(expr.value())){ 
                        for(const auto& el: current_template.first->children(strings.ERROR_TAG)){
                            stack_template.emplace(el.begin(),el.end());
                            _parse(current_template.first);
                            stack_compiled.emplace(current_compiled);
                        }
                    }
                    else{
                        auto good_data = prepare_props_data(std::get<const pugi::xml_node>(expr.value()), limit, offset, filter,order_method_t::from_string(_order_by));

                        if(good_data.size()==0){
                            for(const auto& el: current_template.first->children(strings.EMPTY_TAG)){
                                stack_template.emplace(el.begin(),el.end());
                                _parse(current_template.first);
                                stack_compiled.emplace(current_compiled);
                            }
                        }
                        else{
                            //Header (once)
                            {
                                for(const auto& el: current_template.first->children(strings.HEADER_TAG)){
                                    stack_template.emplace(el.begin(),el.end());
                                    _parse(current_template.first);
                                    stack_compiled.emplace(current_compiled);
                                }
                            }
                        
                            //Items (iterate)
                            int counter = 0;
                            for(auto& i : good_data){
                                auto frame_guard = symbols.guard();

                                if(tag!=nullptr)symbols.set(tag,i);
                                symbols.set("$",i);
                                symbols.set("$$",counter);

                                for(const auto& el: current_template.first->children(strings.ITEM_TAG)){
                                    stack_template.emplace(el.begin(),el.end());
                                    _parse(current_template.first);
                                    stack_compiled.emplace(current_compiled);
                                }
                                counter++;
                            }

                            //Footer (once)
                            {
                                for(const auto& el: current_template.first->children(strings.FOOTER_TAG)){
                                    stack_template.emplace(el.begin(),el.end());
                                    _parse(current_template.first);
                                    stack_compiled.emplace(current_compiled);
                                }
                            }
                        }
                    }
                }
                else if(strcmp(current_template.first->name(), strings.ELEMENT_TAG)==0){
                    //It is possible for it to generate strange results as strings are not validated by pugi
                    auto symbol = resolve_expr(current_template.first->attribute(strings.TYPE_ATTR).as_string("$"));
                    if(!symbol.has_value()){
                    }
                    else if(std::holds_alternative<std::string>(symbol.value())){
                        auto child = current_compiled.append_child(std::get<std::string>(symbol.value()).c_str());
                        for(auto& attr : current_template.first->attributes()){
                            if(strcmp(attr.name(),strings.TYPE_ATTR)!=0)child.append_attribute(attr.name()).set_value(attr.value());
                        }
                        stack_compiled.emplace(child);

                        stack_template.emplace(current_template.first->begin(),current_template.first->end());
                        _parse(current_template.first);
                        stack_compiled.emplace(current_compiled);
                    }
                    else if(std::holds_alternative<const pugi::xml_node>(symbol.value())){
                        auto child = current_compiled.append_child(std::get<const pugi::xml_node>(symbol.value()).text().as_string());
                        for(auto& attr : current_template.first->attributes()){
                            if(strcmp(attr.name(),strings.TYPE_ATTR)!=0)child.append_attribute(attr.name()).set_value(attr.value());
                        }
                        stack_compiled.emplace(child);

                        stack_template.emplace(current_template.first->begin(),current_template.first->end());
                        _parse(current_template.first);
                        stack_compiled.emplace(current_compiled);
                    }
                    else{}
                }
                else if(strcmp(current_template.first->name(), strings.VALUE_TAG)==0){
                    auto symbol = resolve_expr(current_template.first->attribute("src").as_string("$"));
                    if(!symbol.has_value()){
                        /*Show default content if search fails*/
                        stack_template.emplace(current_template.first->begin(),current_template.first->end());
                        _parse(current_template.first);
                        stack_compiled.emplace(current_compiled);
                    }
                    else{
                        if(std::holds_alternative<int>(symbol.value())){
                            current_compiled.append_child(pugi::node_pcdata).set_value(std::to_string(std::get<int>(symbol.value())).c_str());
                        }
                        else if(std::holds_alternative<float>(symbol.value())){
                            current_compiled.append_child(pugi::node_pcdata).set_value(std::to_string(std::get<float>(symbol.value())).c_str());
                        }
                        else if(std::holds_alternative<const pugi::xml_attribute>(symbol.value())) {
                            current_compiled.append_child(pugi::node_pcdata).set_value(std::get<const pugi::xml_attribute>(symbol.value()).as_string());
                        }
                        else if(std::holds_alternative<std::string>(symbol.value())) {
                            current_compiled.append_child(pugi::node_pcdata).set_value(std::get<std::string>(symbol.value()).c_str());
                        }
                        else if(std::holds_alternative<const pugi::xml_node>(symbol.value())) {
                            auto tmp = std::get<const pugi::xml_node>(symbol.value());
                            current_compiled.append_copy(tmp);
                        }
                    }
                }
                else if(strcmp(current_template.first->name(),strings.TEST_TAG)==0){
                    for(const auto& entry: current_template.first->children(strings.CASE_TAG)){
                        bool _continue =  entry.attribute("continue").as_bool(false);
                        auto test = resolve_expr(entry.attribute("value").as_string("$"));

                        auto when =  get_or<int>(resolve_expr(current_template.first->attribute("when").as_string(": false")).value_or(false),false);
                
                        if(when){
                            stack_template.emplace(entry.begin(),entry.end());
                            _parse(current_template.first);
                            stack_compiled.emplace(current_compiled);

                            if(_continue==false)break;
                        }
                    }
                }
                else if(strcmp(current_template.first->name(),strings.LOG_TAG)==0){
                    auto _type = current_template.first->attribute("type").as_string("info");
                    auto _msg = resolve_expr(current_template.first->attribute("type").as_string(""));
                    log_t::values type = log_t::PANIC;
                    //        ERROR, WARNING, PANIC, INFO
                    if(strcmp(_type,"info")==0)type=log_t::INFO;
                    else if(strcmp(_type,"panic")==0)type=log_t::PANIC;
                    else if(strcmp(_type,"error")==0)type=log_t::ERROR;
                    else if(strcmp(_type,"warning")==0)type=log_t::WARNING;
                    else {/*....*/}
                    if(_msg.has_value()){
                        if(std::holds_alternative<std::string>(_msg.value()))log(type,std::get<std::string>(_msg.value()));
                        else if(std::holds_alternative<int>(_msg.value()))log(type,std::to_string(std::get<int>(_msg.value())));
                        else if(std::holds_alternative<float>(_msg.value()))log(type,std::to_string(std::get<float>(_msg.value())));
                        else{/*...*/}
                    }
                }
                else if(strcmp(current_template.first->name(),strings.INCLUDE_TAG)==0){
                    //This is intentionally not an expression. Declarations of files to include should always be fully static to ensure they can be statically traced.
                    auto src = current_template.first->attribute("src").as_string("");
                    if(src[0]!=0){
                        pugi::xml_document localdoc;
                        if(loadfn(src,localdoc)){
                            current_template.first->attribute("src").set_value("");
                            current_template.first->remove_children();
                            for(auto& child: localdoc.root().first_child().children()){
                                current_template.first->append_copy(child);
                            }
                        }
                        else log(log_t::WARNING, std::format("Unable to use file `{}`, the content of the `include` will be used instead",src));
                    }
                    
                    stack_template.emplace(current_template.first->begin(),current_template.first->end());
                    _parse(current_template.first);
                    stack_compiled.emplace(current_compiled);
                    
                }
                else {
                    log(log_t::ERROR, std::format("unrecognized static operation `{}`",current_template.first->name()));
                }
                
                current_template.first++;
                continue;
            }
        
            

            auto last = current_compiled.append_child(current_template.first->type());
            last.set_name(current_template.first->name());
            last.set_value(current_template.first->value());
            for(const auto& attr :current_template.first->attributes()){
                //Special handling of static attribute rewrite rules
                if(strncmp(attr.name(), ns_prefix.c_str(), ns_prefix.length())==0){
                    if(false){}
                    //Matches for.src.key.* even without named suffix
                    else if(cexpr_strneqv(attr.name()+ns_prefix.length(),"for.src.key")){
                        int subgroup_length = 0;
                        if(attr.name()[ns_prefix.length()+sizeof("for.src.key")-1]=='\0'){}
                        else if(attr.name()[ns_prefix.length()+sizeof("for.src.key")-1]=='.'){subgroup_length=strlen(attr.name())-ns_prefix.length()+sizeof("for.src.key")-1+1;}
                        else {continue;}

#                       define WRITE(NAME,VALUE)    char NAME [ns_prefix.length()+sizeof(VALUE)-1+subgroup_length+1];\
                                                    memcpy(NAME,ns_prefix.data(),ns_prefix.length());\
                                                    memcpy(NAME+ns_prefix.length(),VALUE,std::char_traits<char>::length(VALUE));\
                                                    if(subgroup_length!=0){\
                                                        NAME [ns_prefix.length()+std::char_traits<char>::length(VALUE)]='.';\
                                                        memcpy(NAME+ns_prefix.length()+std::char_traits<char>::length(VALUE)+1,attr.name()+ns_prefix.length()+sizeof("for.src.key")-1+1,subgroup_length);\
                                                    }\
                                                    NAME [sizeof(NAME)-1]=0;

                        //Compute all the other tags dynamically
                        WRITE(FOR_SRC_KEY_PROP,"for.src.key");
                        WRITE(FOR_SRC_VALUE_PROP,"for.src.value");
                        WRITE(FOR_SORT_BY_PROP,"for.sort-by");
                        WRITE(FOR_ORDER_BY_PROP,"for.order-by");
                        WRITE(FOR_LIMIT_PROP,"for.limit");
                        WRITE(FOR_OFFSET_PROP,"for.offset");
                        WRITE(FOR_FORMAT_KEY_PROP,"for.format.key");
                        WRITE(FOR_FORMAT_VALUE_PROP,"for.format.value");
#                       undef WRITE

                        //Collect values
                        const char* in = current_template.first->attribute("in").as_string(current_template.first->attribute("src").as_string());

                        //TODO: filter has not defined syntax yet.
                        //const char* _filter = current_template.first->attribute("filter").as_string();
                        const char* _sort_by = current_template.first->attribute("sort-by").as_string();
                        const char* _order_by = current_template.first->attribute("order-by").as_string("asc");

                        int limit = get_or<int>(resolve_expr(current_template.first->attribute("limit").as_string("0")).value_or(0),0);
                        int offset = get_or<int>(resolve_expr(current_template.first->attribute("offset").as_string("0")).value_or(0),0);
                        
                        auto expr = resolve_expr(in);
                        log(log_t::ERROR, std::format("static operation `{}` not yet implemented",attr.name()));
                    }
                    else if(cexpr_strneqv(attr.name()+ns_prefix.length(),"for-props.src.")){
                        log(log_t::ERROR, std::format("static operation `{}` not yet implemented",attr.name()));
                    }
                    else if(cexpr_strneqv(attr.name()+ns_prefix.length(),"prop.")){
                        log(log_t::ERROR, std::format("static operation `{}` not yet implemented",attr.name()));
                    }
                    else if(cexpr_strneqv(attr.name()+ns_prefix.length(),"value.")){
                        log(log_t::ERROR, std::format("static operation `{}` not yet implemented",attr.name()));
                    }
                    else if(cexpr_strneqv(attr.name()+ns_prefix.length(),"when")){
                        auto test  = get_or<int>(resolve_expr(attr.value()).value_or(false),false);
                        if(!test)last.parent().remove_child(last);
                        break;
                    }
                    else {log(log_t::ERROR, std::format("unrecognized static operation `{}`",attr.name()));}
                }
                else last.append_attribute(attr.name()).set_value(attr.value());
            }
            if(!current_template.first->children().empty()){
        
                stack_template.emplace(current_template.first->children().begin(),current_template.first->children().end());
                stack_compiled.emplace(last);
            }
            current_template.first++;
        }
        else{
            stack_template.pop();
            stack_compiled.pop();
        }
    }

    return;
}


}
}