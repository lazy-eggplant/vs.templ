#pragma once

#include <cstdlib>
#include <cstring>
#include <algorithm>
#include <map>
#include <optional>
#include <stack>
#include <string>
#include <variant>
#include <vector>

#include <pugixml.hpp>

#include "utils.hpp"
#include "symbols.hpp"

namespace vs{
namespace templ{


struct preprocessor{
    private:
        std::string ns_prefix = "ns:";

        //Final document to be shared
        pugi::xml_document compiled;

        //To track operations within the parser
        std::stack<pugi::xml_node> stack_compiled;
        std::stack<pugi::xml_node_iterator> stack_data;
        std::stack<std::pair<pugi::xml_node_iterator,pugi::xml_node_iterator>> stack_template;

        //Stack-like table of symbols
        symbol_map symbols;

        //Entry point in the root document.
        pugi::xml_node root_data;

    public:
        inline preprocessor(const pugi::xml_document& root_data, const pugi::xml_document& root_template){
            init(root_data,root_template);
        }

        void init(const pugi::xml_document& root_data, const pugi::xml_document& root_template){
            stack_template.push({root_template.root().begin(),root_template.root().end()});
            stack_compiled.push(compiled.root());
            this->root_data=root_data.root();
        }
        inline void reset(){symbols.reset();stack_template=decltype(stack_template)();stack_data=decltype(stack_data)();stack_compiled=decltype(stack_compiled)();}
        inline pugi::xml_document& parse (){_parse({});return compiled;}

        inline void ns(const char* str){ns_prefix = str;/*TODO: Update strings on change*/}

    private:
        struct order_method_t{
            enum values{
                UNKNOWN =  0, 
                ASC, 
                DESC, 
                RANDOM,

                USE_DOT_EVAL = 16 //For strings, split evaluation based on their dot groups. Valid for all methods.
            };

            static values from_string(const char* str);
        };

        //Precomputed string to avoid spawning an absurd number of small objects in heap at each cycle.
        struct node_strings{
            private:
                char* data = nullptr;
            public:

            //S:TAGS
            const char *FOR_RANGE_TAG;

            const char *FOR_TAG;
            const char *FOR_PROPS_TAG;
                const char *EMPTY_TAG;
                const char *HEADER_TAG;
                const char *FOOTER_TAG;
                const char *ITEM_TAG;
                const char *ERROR_TAG;

            const char *WHEN_TAG;
                const char *IS_TAG;

            const char *VALUE_TAG;
            const char *ELEMENT_TAG;
                const char *TYPE_ATTR;

            //S:PROPS
            const char *FOR_IN_PROP;
            const char *FOR_FILTER_PROP;
            const char *FOR_SORT_BY_PROP;
            const char *FOR_ORDER_BY_PROP;
            const char *FOR_OFFSET_PROP;
            const char *FOR_LIMIT_PROP;

            const char *FOR_PROPS_IN_PROP;
            const char *FOR_PROPS_FILTER_PROP;
            const char *FOR_PROPS_ORDER_BY_PROP;
            const char *FOR_PROPS_OFFSET_PROP;
            const char *FOR_PROPS_LIMIT_PROP;

            const char *VALUE_SRC_PROP;
            const char *VALUE_FORMAT_PROP;

            const char *USE_SRC_PROP;

    #       define WRITE(name,value) name=data+count;memcpy(data+count,ns_prefix,ns_prefix_len);memcpy(data+count+ns_prefix_len,value,std::char_traits<char>::length(value));data[count+ns_prefix_len+std::char_traits<char>::length(value)]=0;count+=ns_prefix_len+std::char_traits<char>::length(value)+1;
    #       define STRLEN(str) ns_prefix_len+std::char_traits<char>::length(str)+1

            void prepare(const char * ns_prefix){
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
            }

            ~node_strings(){delete[] data;}

            node_strings(){prepare(ns_prefix);}

    #       undef STRLEN
    #       undef WRITE
        }strings;



        //Transforming a string into a parsed symbol, setting an optional base root or leaving it to a default evaluation.
        std::optional<concrete_symbol> resolve_expr(const char* _str, const pugi::xml_node* base=nullptr);

        std::vector<pugi::xml_attribute> prepare_props_data(const pugi::xml_node& base, int limit, int offset, bool(*filter)(const pugi::xml_attribute&), order_method_t::values criterion){
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

        std::vector<pugi::xml_node> prepare_children_data(const pugi::xml_node& base, int limit, int offset, bool(*filter)(const pugi::xml_node&), const std::vector<std::pair<std::string,order_method_t::values>>& criteria){
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

        void _parse(std::optional<pugi::xml_node_iterator> stop_at){ 
            
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
                                stack_template.push({current_template.first->begin(),current_template.first->end()});
                                _parse(current_template.first);
                                //When exiting one too many is removed. restore it.
                                stack_compiled.push(current_compiled);
                            }
                        }
                        else if(strcmp(current_template.first->name(),strings.FOR_TAG)==0){
                            const char* tag = current_template.first->attribute("tag").as_string();
                            const char* in = current_template.first->attribute("in").as_string();
                            //TODO: filter has not defined syntax yet.
                            const char* _filter = current_template.first->attribute("filter").as_string();

                            const char* _sort_by = current_template.first->attribute("sort-by").as_string();
                            const char* _order_by = current_template.first->attribute("order-by").as_string("asc");

                            int limit = get_or<int>(resolve_expr(current_template.first->attribute("limit").as_string("0")).value_or(0),0);
                            int offset = get_or<int>(resolve_expr(current_template.first->attribute("offset").as_string("0")).value_or(0),0);
                            
                            auto expr = resolve_expr(in);


                            //Only a node is acceptable in this context, otherwise show the error
                            if(!expr.has_value() || !std::holds_alternative<const pugi::xml_node>(expr.value())){ 
                                for(const auto& el: current_template.first->children(strings.ERROR_TAG)){
                                    stack_template.push({el.begin(),el.end()});
                                    _parse(current_template.first);
                                    stack_compiled.push(current_compiled);
                                }
                            }
                            else{
                                std::vector<std::pair<std::string,order_method_t>> criteria;
                                //Build criteria
                                {
                                    auto orders = split(_order_by,',');
                                    int c = 0;
                                    //Apply order directive with wrapping in case not enough cases are specified.
                                    for(auto& i:split(_sort_by,',')){criteria.push_back({i,order_form_str(orders[c%orders.size()])});c++;}
                                }

                                auto good_data = prepare_children_data(std::get<const pugi::xml_node>(expr.value()), limit, offset, nullptr, criteria);

                                if(good_data.size()==0){
                                    for(const auto& el: current_template.first->children(strings.EMPTY_TAG)){
                                        stack_template.push({el.begin(),el.end()});
                                        _parse(current_template.first);
                                        stack_compiled.push(current_compiled);
                                    }
                                }
                                else{
                                    //Header (once)
                                    {
                                        for(const auto& el: current_template.first->children(strings.HEADER_TAG)){
                                            stack_template.push({el.begin(),el.end()});
                                            _parse(current_template.first);
                                            stack_compiled.push(current_compiled);
                                        }
                                    }
                                
                                    //Items (iterate)
                                    for(auto& i : good_data){
                                        auto frame_guard = symbols.guard();
                                        if(tag!=nullptr)symbols.set(tag,i);
                                        symbols.set("$",i);
                                        for(const auto& el: current_template.first->children(strings.ITEM_TAG)){
                                            stack_template.push({el.begin(),el.end()});
                                            _parse(current_template.first);
                                            stack_compiled.push(current_compiled);
                                        }

                                    }

                                    //Footer (once)
                                    {
                                        for(const auto& el: current_template.first->children(strings.FOOTER_TAG)){
                                            stack_template.push({el.begin(),el.end()});
                                            _parse(current_template.first);
                                            stack_compiled.push(current_compiled);
                                        }
                                    }
                                }
                            }
                        }
                        else if(strcmp(current_template.first->name(),strings.FOR_PROPS_TAG)==0){
                            const char* tag = current_template.first->attribute("tag").as_string();
                            const char* in = current_template.first->attribute("in").as_string();
                            //TODO: filter has not defined syntax yet.
                            const char* _filter = current_template.first->attribute("filter").as_string();

                            const char* _order_by = current_template.first->attribute("order-by").as_string("asc");

                            int limit = get_or<int>(resolve_expr(current_template.first->attribute("limit").as_string("0")).value_or(0),0);
                            int offset = get_or<int>(resolve_expr(current_template.first->attribute("offset").as_string("0")).value_or(0),0);
                            
                            auto expr = resolve_expr(in);


                            //Only a node is acceptable in this context, otherwise show the error
                            if(!expr.has_value() || !std::holds_alternative<const pugi::xml_node>(expr.value())){ 
                                for(const auto& el: current_template.first->children(strings.ERROR_TAG)){
                                    stack_template.push({el.begin(),el.end()});
                                    _parse(current_template.first);
                                    stack_compiled.push(current_compiled);
                                }
                            }
                            else{
                                auto good_data = prepare_props_data(std::get<const pugi::xml_node>(expr.value()), limit, offset, nullptr,order_form_str(_order_by));

                                if(good_data.size()==0){
                                    for(const auto& el: current_template.first->children(strings.EMPTY_TAG)){
                                        stack_template.push({el.begin(),el.end()});
                                        _parse(current_template.first);
                                        stack_compiled.push(current_compiled);
                                    }
                                }
                                else{
                                    //Header (once)
                                    {
                                        for(const auto& el: current_template.first->children(strings.HEADER_TAG)){
                                            stack_template.push({el.begin(),el.end()});
                                            _parse(current_template.first);
                                            stack_compiled.push(current_compiled);
                                        }
                                    }
                                
                                    //Items (iterate)
                                    for(auto& i : good_data){
                                        auto frame_guard = symbols.guard();
                                        if(tag!=nullptr)symbols.set(tag,i);
                                        symbols.set("$",i);
                                        for(const auto& el: current_template.first->children(strings.ITEM_TAG)){
                                            stack_template.push({el.begin(),el.end()});
                                            _parse(current_template.first);
                                            stack_compiled.push(current_compiled);
                                        }

                                    }

                                    //Footer (once)
                                    {
                                        for(const auto& el: current_template.first->children(strings.FOOTER_TAG)){
                                            stack_template.push({el.begin(),el.end()});
                                            _parse(current_template.first);
                                            stack_compiled.push(current_compiled);
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
                                stack_compiled.push(child);

                                stack_template.push({current_template.first->begin(),current_template.first->end()});
                                _parse(current_template.first);
                                stack_compiled.push(current_compiled);
                            }
                            else if(std::holds_alternative<const pugi::xml_node>(symbol.value())){
                                auto child = current_compiled.append_child(std::get<const pugi::xml_node>(symbol.value()).text().as_string());
                                for(auto& attr : current_template.first->attributes()){
                                    if(strcmp(attr.name(),strings.TYPE_ATTR)!=0)child.append_attribute(attr.name()).set_value(attr.value());
                                }
                                stack_compiled.push(child);

                                stack_template.push({current_template.first->begin(),current_template.first->end()});
                                _parse(current_template.first);
                                stack_compiled.push(current_compiled);
                            }
                            else{}
                        }
                        else if(strcmp(current_template.first->name(), strings.VALUE_TAG)==0){
                            auto symbol = resolve_expr(current_template.first->attribute("src").as_string("$"));
                            if(!symbol.has_value()){
                                /*Show default content if search fails*/
                                stack_template.push({current_template.first->begin(),current_template.first->end()});
                                _parse(current_template.first);
                                stack_compiled.push(current_compiled);
                            }
                            else{
                                if(std::holds_alternative<int>(symbol.value())){
                                    current_compiled.append_child(pugi::node_pcdata).set_value(std::to_string(std::get<int>(symbol.value())).c_str());
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
                        else if(strcmp(current_template.first->name(),strings.WHEN_TAG)==0){
                            auto subject = resolve_expr(current_template.first->attribute("subject").as_string("$"));
                            for(const auto& entry: current_template.first->children(strings.IS_TAG)){
                                bool _continue =  entry.attribute("continue").as_bool(false);
                                auto test = resolve_expr(entry.attribute("value").as_string("$"));

                                bool result = false;
                                //TODO: Perform comparison.

                                if(!subject.has_value() && !test.has_value()){result = true;}
                                else if (!subject.has_value() || !test.has_value()){result = false;}
                                else if(std::holds_alternative<int>(subject.value()) && std::holds_alternative<int>(test.value())){
                                    result = std::get<int>(subject.value())==std::get<int>(test.value());
                                }
                                else{

                                    //Move everything to string
                                    const char* op1,* op2;
                                    if(std::holds_alternative<std::string>(subject.value()))op1=std::get<std::string>(subject.value()).c_str();
                                    else if(std::holds_alternative<const pugi::xml_attribute>(subject.value()))op1=std::get<const pugi::xml_attribute>(subject.value()).as_string();
                                    else if(std::holds_alternative<const pugi::xml_node>(subject.value()))op1=std::get<const pugi::xml_node>(subject.value()).text().as_string();

                                    if(std::holds_alternative<std::string>(test.value()))op2=std::get<std::string>(test.value()).c_str();
                                    else if(std::holds_alternative<const pugi::xml_attribute>(test.value()))op2=std::get<const pugi::xml_attribute>(test.value()).as_string();
                                    else if(std::holds_alternative<const pugi::xml_node>(test.value()))op2=std::get<const pugi::xml_node>(test.value()).text().as_string();

                                    result = strcmp(op1,op2)==0;
                                }
                        
                                if(result){
                                    stack_template.push({entry.begin(),entry.end()});
                                    _parse(current_template.first);
                                    stack_compiled.push(current_compiled);

                                    if(_continue==false)break;
                                }
                            }
                        }
                        else {std::cerr<<"unrecognized static operation\n";}
                        
                        current_template.first++;
                        continue;
                    }
                
                    

                    auto last = current_compiled.append_child(current_template.first->type());
                    last.set_name(current_template.first->name());
                    last.set_value(current_template.first->value());
                    for(const auto& attr :current_template.first->attributes()){
                        //Special handling of static attribute rewrite rules
                        if(strncmp(attr.name(), ns_prefix, ns_prefix_len)==0){
                            if(cx_strneqv(attr.name()+ns_prefix_len,"for.src.")){}
                            else if(cx_strneqv(attr.name()+ns_prefix_len,"for-prop.src.")){}
                            else if(cx_strneqv(attr.name()+ns_prefix_len,"use.src.")){}
                            else if(cx_strneqv(attr.name()+ns_prefix_len,"eval.")){}
                            else {std::cerr<<"unrecognized static operation\n";}
                        }
                        else last.append_attribute(attr.name()).set_value(attr.value());
                    }
                    if(!current_template.first->children().empty()){
                
                        stack_template.push({current_template.first->children().begin(),current_template.first->children().end()});
                        stack_compiled.push(last);
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

};


}
}