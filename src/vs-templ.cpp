#include <algorithm>
#include <string>
#include <string_view>
#include <variant>
#include <format>
#include <charconv>

#include "vs-templ.hpp"
#include "stack-lang.hpp"
#include "logging.hpp"
#include "utils.hpp"

namespace vs{
namespace templ{


void preprocessor::init(const pugi::xml_node& root_data, const pugi::xml_node& root_template,const char* prefix, logfn_t _logfn, includefn_t _includefn, loadfn_t _loadfn, uint64_t seed){
    if(_logfn!=nullptr)logfn=_logfn;
    if(_includefn!=nullptr)includefn=_includefn;
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
    stack_compiled=decltype(stack_compiled)();
}

void preprocessor::log(log_t::values type, const std::string& str) const{
    //TODO: Add contextual information.
    logctx_t ctx;
    logfn(type,str.data(),ctx);
}


//TODO: Implement
preprocessor::compare_result preprocessor::compare_symbols(const symbol& a, const symbol& b, order_t method) const{
    //Apply defaults if not specified
    if(method.type==order_t::type_t::DEFAULT){
        if(std::holds_alternative<int>(a))method.type=order_t::type_t::INTEGER;
        else if(std::holds_alternative<float>(a))method.type=order_t::type_t::FLOAT;
        else if(std::holds_alternative<std::string>(a))method.type=order_t::type_t::STRING;//TODO: move to `order_t::type_t::NATURAL_STRING`
        else if(std::holds_alternative<const pugi::xml_node>(a))method.type=order_t::type_t::NODE;
    }

    if(method.method==order_t::method_t::DEFAULT){
        method.method=order_t::method_t::ASC;
    }

    //Simle cases to handle via std overloads
    if(
        (std::holds_alternative<int>(a) && std::holds_alternative<int>(b) && method.type==order_t::type_t::INTEGER)||
        (std::holds_alternative<int>(a) && std::holds_alternative<int>(b) && method.type==order_t::type_t::BOOLEAN)||
        (std::holds_alternative<float>(a) && std::holds_alternative<float>(b) && method.type==order_t::type_t::FLOAT)
    ){
        if(method.method==order_t::method_t::ASC) return (a<b)?compare_result::LESS: ( (a>b)? compare_result::BIGGER : compare_result::EQUAL);
        else if(method.method==order_t::method_t::DESC) return (a<b)?compare_result::BIGGER: ( (a>b)? compare_result::LESS : compare_result::EQUAL);
        else if(method.method==order_t::method_t::RANDOM){
            auto hash_a = preprocessor::hash(a);
            auto hash_b = preprocessor::hash(b);
            return (hash_a<hash_b)?compare_result::LESS: ( (hash_a>hash_b)? compare_result::BIGGER : compare_result::EQUAL);
        }
    }
    else if(std::holds_alternative<const pugi::xml_node>(a) && std::holds_alternative<const pugi::xml_node>(b) && method.type==order_t::type_t::NODE){
        
    }
    else if(std::holds_alternative<std::string>(a) && std::holds_alternative<std::string>(b) && method.type==order_t::type_t::STRING){
        const auto& _a = std::get<std::string>(a), _b = std::get<std::string>(b);
        if(method.modifiers.dot){
            auto i = cmp_dot_str(_a.c_str(), _b.c_str());
            if(method.method==order_t::method_t::ASC) return (i<0)?compare_result::LESS: ( (i>0)? compare_result::BIGGER : compare_result::EQUAL);
            else if(method.method==order_t::method_t::DESC) return (i<0)?compare_result::BIGGER: ( (i>0)? compare_result::LESS : compare_result::EQUAL);
            else if(method.method==order_t::method_t::RANDOM){
                auto va = split_string(std::get<std::string>(a).c_str(), '.');
                auto vb = split_string(std::get<std::string>(b).c_str(), '.');

                if(va.size()<vb.size())return compare_result::LESS;
                else if(va.size()>vb.size())return compare_result::BIGGER;

                for(size_t i =0;i<va.size();i++){
                    std::array<uint64_t,2> hash_a = {0,0};
                    std::array<uint64_t,2> hash_b = {0,0};

                    hash::MurmurHash3_x64_128(va.at(i).data(),va.at(i).size(),seed,&hash_a);
                    hash::MurmurHash3_x64_128(vb.at(i).data(),vb.at(i).size(),seed,&hash_b);

                    if(hash_a<hash_b)return compare_result::LESS;
                    else if(hash_a>hash_b)return compare_result::BIGGER;
                }
                return compare_result::EQUAL;
            }
        }
        else{
            if(method.method==order_t::method_t::ASC) return (_a<_b)?compare_result::LESS: ( (_a>_b)? compare_result::BIGGER : compare_result::EQUAL);
            else if(method.method==order_t::method_t::DESC) return (_a<_b)?compare_result::BIGGER: ( (_a>_b)? compare_result::LESS : compare_result::EQUAL);
            else if(method.method==order_t::method_t::RANDOM){
                auto hash_a = preprocessor::hash(a);
                auto hash_b = preprocessor::hash(b);
                return (hash_a<hash_b)?compare_result::LESS: ( (hash_a>hash_b)? compare_result::BIGGER : compare_result::EQUAL);
            }
        } 
    }
    else if(std::holds_alternative<std::string>(a) && std::holds_alternative<std::string>(b) && method.type==order_t::type_t::LEXI_STRING){
        
    }
    else if(std::holds_alternative<std::string>(a) && std::holds_alternative<std::string>(b) && method.type==order_t::type_t::NATURAL_STRING){
        //https://github.com/sourcefrog/natsort
    }
    //Cannot have pugi::xml_attribute since it has been resolved by the time it gets here.

    return compare_result::NOT_COMPARABLE;
}


//TODO: The next release of pugixml will have support for string_views directly. 
//      As such, the double buffer to add \0 around will no longer be needed.
//      Once that is done, please revise this function to make it much faster.
std::optional<symbol> preprocessor::resolve_expr(const std::string_view& _str, const pugi::xml_node* base) const{
    int str_len = _str.size(); 
    char str[str_len+1];
    memcpy(str,_str.data(),str_len+1);
    str[str_len]=0;

    pugi::xml_node ref;
    if(base!=nullptr) ref = *base;

    int idx = 0;
    if(str[0]=='.' || str[0]=='+' || str[0]=='-' || (str[0]>'0' && str[0]<'9')){
        if(_str[_str.length()-1]=='f'){str[_str.length()-1]=0;float result{};std::from_chars(str,str+_str.length()-1,result);return result;}
        else{int result{};std::from_chars(str,str+_str.length(),result);return result;}
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
        else if(std::holds_alternative<std::string>(tmp.value())){
            return std::get<std::string>(tmp.value());
        }
        else if(std::holds_alternative<float>(tmp.value())){
            return std::get<float>(tmp.value());
        }
        if(close==str_len)return ref;        //End of string was met earlier
        idx=close+1;
    }
    else if(str[0]=='$'){

        if(base==nullptr){
            auto tmp = symbols.resolve("$");
            if(tmp.has_value() && std::holds_alternative<const pugi::xml_node>(tmp.value())){
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
        //Special case to reduce complexity in expression.
        //If no prefix is used, $ is asssumed as reference
        if(idx==0 && base==nullptr){
            auto tmp = symbols.resolve("$");
            if(!tmp.has_value() || std::holds_alternative<const pugi::xml_node>(tmp.value())==false)return {};
            else{
                ref=std::get<const pugi::xml_node>(tmp.value());
            }
        }
        if(strcmp(str+idx+1,"!txt")==0) return ref.text().as_string();
        else if(strcmp(str+idx+1,"!tag")==0) return ref.name();
        else return ref.attribute(str+idx+1).as_string();
    }

    return {};
}

preprocessor::order_t preprocessor::order_from_string(std::string_view str){
    //TODO: extend to specify type. Syntax: `type:method`
    order_t tmp;
    if(str[0]=='.'){tmp.modifiers.dot=true;}
    if((std::string_view(str.begin()+tmp.modifiers.dot, str.end()) == std::string_view("asc"))){tmp.method=order_t::method_t::ASC;}
    else if((std::string_view(str.begin()+tmp.modifiers.dot, str.end()) == std::string_view("desc"))){tmp.method=order_t::method_t::DESC;}
    else if((std::string_view(str.begin()+tmp.modifiers.dot, str.end()) == std::string_view("random"))){tmp.method=order_t::method_t::RANDOM;}
    else{
        log(log_t::WARNING,std::format("`{}` is not a valid criterion for comparison",str));
    }

    return tmp;
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
        STRLEN("data")+

        STRLEN("for.src")+STRLEN("for.filter")+STRLEN("for.sort-by")+STRLEN("for.order-by")+STRLEN("for.offset")+STRLEN("for.limit")+
        STRLEN("for-props.src")+STRLEN("for-props.filter")+STRLEN("for.order-by")+STRLEN("for-props.offset")+STRLEN("for-props.limit")+
        
        STRLEN("value")+
        STRLEN("enable")+
        STRLEN("prop.name")+STRLEN("prop.value")+
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
    WRITE(DATA_TAG,"data");

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

    WRITE(ENABLE_PROP,"enable"); 

    WRITE(PROP_NAME_PROP,"prop.name");
    WRITE(PROP_VALUE_PROP,"prop.value");

    WRITE(WHEN_PROP,"when");


#   undef WRITE
#   undef STRLEN
}

std::vector<pugi::xml_attribute> preprocessor::prepare_props_data(const pugi::xml_node& base, int limit, int offset, const char *filter, order_t criterion){
    auto cmp_fn = [&](const pugi::xml_attribute& a, const pugi::xml_attribute& b)->int{
        //Not ideal due to its additional cast to string... but the code is so much easier... so for now it will stay.
        auto cmp_res = compare_symbols(a.name(), b.name(), criterion);
        if(cmp_res==compare_result::LESS)return true;
        else return false;
    };
    
    std::vector<pugi::xml_attribute> dataset;
    {
        symbols.guard();
        for(auto& child: base.attributes()){
            if(filter!=nullptr && filter[0]!=0){
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

std::vector<pugi::xml_node> preprocessor::prepare_children_data(const pugi::xml_node& base, int limit, int offset, const char* filter, const std::vector<std::pair<std::string,order_t>>& criteria){
    auto cmp_fn = [&](const pugi::xml_node& a, const pugi::xml_node& b)->int{
        for(auto& criterion: criteria){
            auto valA = resolve_expr(criterion.first.c_str(),&a);
            auto valB = resolve_expr(criterion.first.c_str(),&b);

            if(!valA.has_value() || !valB.has_value())continue;
            auto cmp_res = compare_symbols(*valA, *valB, criterion.second);
            if(cmp_res==compare_result::LESS)return true;
            else if(cmp_res==compare_result::BIGGER)return false;
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
            
            auto _visible =  current_template.first->attribute(strings.WHEN_PROP).as_string(nullptr);
            if(_visible!=nullptr){
                auto test  = get_or<int>(resolve_expr(_visible).value_or(false),false);
                if(!test){current_template.first++;continue;}
            }
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
                    const char* tag = current_template.first->attribute("tag").as_string("$");
                    const char* in = current_template.first->attribute("in").as_string();

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
                        std::vector<std::pair<std::string,order_t>> criteria;
                        //Build criteria
                        {
                            auto orders = split_string(_order_by,'|');
                            int c = 0;
                            //Apply order directive with wrapping in case not enough cases are specified.
                            for(auto& i:split_string(_sort_by,'|')){
                                criteria.emplace_back(i,order_from_string(orders[c%orders.size()]));
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
    
                                symbols.set(tag,i);
                                symbols.set(std::string(tag) + ".c",counter);    //TODO stack string

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
                    const char* tag = current_template.first->attribute("tag").as_string("$");
                    const char* in = current_template.first->attribute("in").as_string();

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
                        auto good_data = prepare_props_data(std::get<const pugi::xml_node>(expr.value()), limit, offset, filter,order_from_string(_order_by));

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

                                symbols.set(tag,i);
                                symbols.set(std::string(tag) + ".k",i.name());
                                symbols.set(std::string(tag) + ".v",i.value());
                                symbols.set(std::string(tag) + ".c",counter);    //TODO stack string

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
                        current_template.first->parent().remove_child(*current_template.first);
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
                    auto symbol = resolve_expr(current_template.first->attribute("src").as_string("{$}"));
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
                        auto test = resolve_expr(entry.attribute("value").as_string("{$}"));

                        auto when =  get_or<int>(resolve_expr(entry.attribute("when").as_string(": false")).value_or(false),false);

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
                        if(includefn(src,localdoc)){
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
                else if(strcmp(current_template.first->name(),strings.DATA_TAG)==0){
                    pugi::xml_document localdoc;
                    if(loadfn(*current_template.first,localdoc)){
                        symbols.guard();
                        symbols.set(current_template.first->attribute("tag").as_string("$"), localdoc.root());
                        stack_template.emplace(current_template.first->begin(),current_template.first->end());
                        _parse(current_template.first);
                        stack_compiled.emplace(current_compiled); 
                    }
                    else{
                        log(log_t::ERROR, std::format("The `data` command was unable to load its content."));
                        stack_template.emplace(current_template.first->begin(),current_template.first->end());
                        _parse(current_template.first);
                        stack_compiled.emplace(current_compiled); 
                    }
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
                    else if(cexpr_strneqv(attr.name()+ns_prefix.length(),"for.prop")){
                        int subgroup_length = 0;
                        if(attr.name()[ns_prefix.length()+sizeof("for.prop")-1]=='\0'){}
                        else if(attr.name()[ns_prefix.length()+sizeof("for.prop")-1]=='.'){subgroup_length=strlen(attr.name())-ns_prefix.length()+sizeof("for.prop")-1+1;}
                        else {continue;}

#                       define WRITE(NAME,VALUE)    char NAME [ns_prefix.length()+sizeof(VALUE)-1+subgroup_length+1];\
                                                    memcpy(NAME,ns_prefix.data(),ns_prefix.length());\
                                                    memcpy(NAME+ns_prefix.length(),VALUE,std::char_traits<char>::length(VALUE));\
                                                    if(subgroup_length!=0){\
                                                        NAME [ns_prefix.length()+std::char_traits<char>::length(VALUE)]='.';\
                                                        memcpy(NAME+ns_prefix.length()+std::char_traits<char>::length(VALUE)+1,attr.name()+ns_prefix.length()+sizeof("for.prop")-1+1,subgroup_length);\
                                                    }\
                                                    NAME [sizeof(NAME)-1]=0;

                        //Compute all the other tags dynamically
                        WRITE(FOR_TAG_PROP,"for.tag");
                        WRITE(FOR_IN_PROP,"for.in");
                        WRITE(FOR_FILTER_PROP,"for.filter");
                        WRITE(FOR_SORT_BY_PROP,"for.sort-by");
                        WRITE(FOR_ORDER_BY_PROP,"for.order-by");
                        WRITE(FOR_LIMIT_PROP,"for.limit");
                        WRITE(FOR_OFFSET_PROP,"for.offset");
                        WRITE(FOR_PROP_PROP,"for.prop");
#                       undef WRITE

                        const char* tag = current_template.first->attribute(FOR_TAG_PROP).as_string("$");
                        const char* in = current_template.first->attribute(FOR_IN_PROP).as_string();

                        const char* _prop = current_template.first->attribute(FOR_PROP_PROP).as_string();
                        const char* filter = current_template.first->attribute(FOR_FILTER_PROP).as_string(nullptr);
                        const char* _sort_by = current_template.first->attribute(FOR_SORT_BY_PROP).as_string();
                        const char* _order_by = current_template.first->attribute(FOR_ORDER_BY_PROP).as_string("asc");

                        int limit = get_or<int>(resolve_expr(current_template.first->attribute(FOR_LIMIT_PROP).as_string("0")).value_or(0),0);
                        int offset = get_or<int>(resolve_expr(current_template.first->attribute(FOR_OFFSET_PROP).as_string("0")).value_or(0),0);

                        auto expr = resolve_expr(in);


                        if(!expr.has_value() || !std::holds_alternative<const pugi::xml_node>(expr.value())){ 
                            //Do nothing; Maybe warning?
                        }
                        else{
                            std::vector<std::pair<std::string,order_t>> criteria;
                            //Build criteria
                            {
                                auto orders = split_string(_order_by,'|');
                                int c = 0;
                                //Apply order directive with wrapping in case not enough cases are specified.
                                for(auto& i:split_string(_sort_by,'|')){
                                    criteria.emplace_back(i,order_from_string(orders[c%orders.size()]));
                                    c++;
                                }
                            }
                            auto good_data = prepare_children_data(std::get<const pugi::xml_node>(expr.value()), limit, offset, filter, criteria);

                            //Items (iterate)
                            int counter = 0;
                            for(auto& i : good_data){
                                auto frame_guard = symbols.guard();
    
                                symbols.set(tag,i);
                                symbols.set(std::string(tag) + ".c",counter);    //TODO stack string

                                auto pair = split_string(_prop,'|');
                                if(pair.size()!=2){
                                    log(log_t::ERROR, std::format("unrecognized pair for prop command `for`"));
                                }
                                else{
                                    auto _name = resolve_expr(pair[0]);
                                    auto _value = resolve_expr(pair[1]);

                                    if(_name.has_value() && _value.has_value()){
                                        auto name = to_string(_name.value());
                                        auto value = to_string(_value.value());
                                        if(name.has_value() && value.has_value())last.append_attribute(name.value().c_str()).set_value(value.value().c_str());
                                        /*Error?*/
                                    }
                                }
                                
                                counter++;
                            }
                        }
                    }
                    else if(cexpr_strneqv(attr.name()+ns_prefix.length(),"for.")){
                        /*Skip, already considered in the earlier block*/
                    }
                    else if(cexpr_strneqv(attr.name()+ns_prefix.length(),"for-props.prop")){
                        int subgroup_length = 0;
                        if(attr.name()[ns_prefix.length()+sizeof("for-props.prop")-1]=='\0'){}
                        else if(attr.name()[ns_prefix.length()+sizeof("for-props.prop")-1]=='.'){subgroup_length=strlen(attr.name())-ns_prefix.length()+sizeof("for-props.prop")-1+1;}
                        else {continue;}

#                       define WRITE(NAME,VALUE)    char NAME [ns_prefix.length()+sizeof(VALUE)-1+subgroup_length+1];\
                                                    memcpy(NAME,ns_prefix.data(),ns_prefix.length());\
                                                    memcpy(NAME+ns_prefix.length(),VALUE,std::char_traits<char>::length(VALUE));\
                                                    if(subgroup_length!=0){\
                                                        NAME [ns_prefix.length()+std::char_traits<char>::length(VALUE)]='.';\
                                                        memcpy(NAME+ns_prefix.length()+std::char_traits<char>::length(VALUE)+1,attr.name()+ns_prefix.length()+sizeof("for-props.prop")-1+1,subgroup_length);\
                                                    }\
                                                    NAME [sizeof(NAME)-1]=0;

                        //Compute all the other tags dynamically
                        WRITE(FOR_TAG_PROP,"for-props.tag");
                        WRITE(FOR_IN_PROP,"for-props.in");
                        WRITE(FOR_FILTER_PROP,"for-props.filter");
                        WRITE(FOR_ORDER_BY_PROP,"for-props.order-by");
                        WRITE(FOR_LIMIT_PROP,"for-props.limit");
                        WRITE(FOR_OFFSET_PROP,"for-props.offset");
                        WRITE(FOR_PROP_PROP,"for-props.prop");
#                       undef WRITE

                        const char* tag = current_template.first->attribute(FOR_TAG_PROP).as_string("$");
                        const char* in = current_template.first->attribute(FOR_IN_PROP).as_string();

                        const char* _prop = current_template.first->attribute(FOR_PROP_PROP).as_string();
                        const char* filter = current_template.first->attribute(FOR_FILTER_PROP).as_string(nullptr);
                        const char* _order_by = current_template.first->attribute(FOR_ORDER_BY_PROP).as_string();

                        int limit = get_or<int>(resolve_expr(current_template.first->attribute(FOR_LIMIT_PROP).as_string("0")).value_or(0),0);
                        int offset = get_or<int>(resolve_expr(current_template.first->attribute(FOR_OFFSET_PROP).as_string("0")).value_or(0),0);

                        auto expr = resolve_expr(in);

                        if(!expr.has_value() || !std::holds_alternative<const pugi::xml_node>(expr.value())){ 
                            //Maybe error?
                        }
                        else{
                            auto good_data = prepare_props_data(std::get<const pugi::xml_node>(expr.value()), limit, offset, filter,order_from_string(_order_by));

                            if(good_data.size()==0){
                                //Do nothing; Maybe warning?
                            }
                            else{
                                //Items (iterate)
                                int counter = 0;
                                for(auto& i : good_data){
                                    auto frame_guard = symbols.guard();

                                    symbols.set(tag,i);
                                    symbols.set(std::string(tag) + ".k",i.name());
                                    symbols.set(std::string(tag) + ".v",i.value());
                                    symbols.set(std::string(tag) + ".c",counter);    //TODO stack string

                                    auto pair = split_string(_prop,'|');
                                    if(pair.size()!=2){
                                        log(log_t::ERROR, std::format("unrecognized pair for prop command `for-prop`"));
                                    }
                                    else{
                                        auto _name = resolve_expr(pair[0]);
                                        auto _value = resolve_expr(pair[1]);

                                        if(_name.has_value() && _value.has_value()){
                                            auto name = to_string(_name.value());
                                            auto value = to_string(_value.value());
                                            if(name.has_value() && value.has_value())last.append_attribute(name.value().c_str()).set_value(value.value().c_str());
                                            /*Error?*/
                                        }
                                    }
                                    counter++;
                                }
                            }
                        }
                    }
                    else if(cexpr_strneqv(attr.name()+ns_prefix.length(),"for-props.")){
                        /*Skip, already considered in the earlier block*/
                    }
                    else if(cexpr_strneqv(attr.name()+ns_prefix.length(),"prop.")){
                        auto pair = split_string(attr.value(),'|');
                        if(pair.size()!=2){
                            log(log_t::ERROR, std::format("unrecognized pair for prop command `prop`"));
                        }
                        else{
                            auto _name = resolve_expr(pair[0]);
                            auto _value = resolve_expr(pair[1]);

                            if(_name.has_value() && _value.has_value()){
                               auto name = to_string(_name.value());
                                auto value = to_string(_value.value());
                                if(name.has_value() && value.has_value())last.append_attribute(name.value().c_str()).set_value(value.value().c_str());
                                /*Error?*/
                            }
                        }
                    }
                    else if(cexpr_strneqv(attr.name()+ns_prefix.length(),"value.")){
                        auto val = resolve_expr(attr.value());
                        if(val.has_value()){
                            auto attribute = last.append_attribute(attr.name()+ns_prefix.length()+sizeof("value.")-1);
                            auto v = to_string(val.value());
                            if(v.has_value())attribute.set_value(v->c_str());
                            /* Error? */
                        }
                    }
                    else if(cexpr_strneqv(attr.name()+ns_prefix.length(),"enable.")){
                        auto val = resolve_expr(attr.value());
                        if(val.has_value() && std::holds_alternative<int>(*val)){
                            auto v = (val.value());
                            if(std::get<int>(*val)==true){
                                last.append_attribute(attr.name()+ns_prefix.length()+sizeof("enable.")-1);
                            }
                            /* Error? */
                        }
                    }
                    else if(cexpr_strneqv(attr.name()+ns_prefix.length(),"when")){
                        //Skip, already done
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

std::array<uint64_t,2> preprocessor::hash(const symbol& ref) const{
    uint64_t ret[2] = {0,0};

    if(std::holds_alternative<int>(ref)){
        hash::MurmurHash3_x64_128(&std::get<int>(ref),sizeof(int),seed,&ret);
    }
    else if(std::holds_alternative<float>(ref)){
        hash::MurmurHash3_x64_128(&std::get<float>(ref),sizeof(float),seed,&ret);
    }
    else if(std::holds_alternative<std::string>(ref)){
        auto str =std::get<std::string>(ref);
        hash::MurmurHash3_x64_128(str.c_str(),str.size(),seed,&ret);
    }
    else if(std::holds_alternative<const pugi::xml_node>(ref)){
        auto str = std::get<const pugi::xml_node>(ref).name();
        hash::MurmurHash3_x64_128(str,strlen(str),seed,&ret);
    }
    else if(std::holds_alternative<const pugi::xml_attribute>(ref)){
        auto str = std::get<const pugi::xml_attribute>(ref).name();
        hash::MurmurHash3_x64_128(str,strlen(str),seed,&ret);
    }
    return std::array<uint64_t,2>{ret[0], ret[1]};
}


}
}