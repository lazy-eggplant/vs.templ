#pragma once

#include <map>
#include <optional>
#include <variant>
#include <vector>
#include <pugixml.hpp>

namespace vs{
namespace templ{

//Symbol which can be saved in the table
typedef std::variant<int,const pugi::xml_node, const pugi::xml_attribute> symbol;

//Extended symbol which is the result of computations. String is introduced as they cannot be set, but they can be computed.
typedef std::variant<int,const pugi::xml_node, const pugi::xml_attribute, std::string> concrete_symbol;

//Utility class to implement a list of symbols. Use for `for` like structures in pattern matching.
struct symbol_map{
    private:
        std::vector<std::map<std::string,symbol>> symbols;
    public:
        symbol_map(){
            new_frame();
        }

        void new_frame(){
            symbols.push_back({});
        };

        void remove_frame(){
            symbols.pop_back();
        };

        void reset(){symbols=decltype(symbols)();}

        std::optional<symbol> resolve(std::string_view name){
            for(auto it = symbols.rbegin();it!=symbols.rend();it++){
                auto found = it->find(std::string(name));
                if(found!=it->end()){
                    return found->second;
                }
            }
            return {};
        }

        void set(const std::string& name, auto value){
            symbols.back().emplace(name,value);
        }

        struct guard_t{
            private:
                symbol_map& ref;
            public:
                inline guard_t(symbol_map& r):ref(r){ref.new_frame();}
                inline ~guard_t(){ref.remove_frame();}
        };

        inline guard_t guard(){return guard_t(*this);}
};

    
}
}