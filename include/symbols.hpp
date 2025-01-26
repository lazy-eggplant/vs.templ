#pragma once

#include <cstdint>
#include <map>
#include <optional>
#include <variant>
#include <vector>
#include <pugixml.hpp>

namespace vs{
namespace templ{

/**
 * @brief Interface of symbols saved while parsing the template, and provided an environment
 * @details Booleans are just integer with operational semantics attached. Attributes are always resolved in strings before being added as symbols.
 */
struct symbol : std::variant<int,const pugi::xml_node, std::string, float> {
    using std::variant<int,const pugi::xml_node, std::string, float> :: variant ;
    std::optional<std::string> to_string(){
        if(std::holds_alternative<int>(*this))return std::to_string(std::get<int>(*this));
        else if(std::holds_alternative<float>(*this))return std::to_string(std::get<float>(*this));
        else if(std::holds_alternative<std::string>(*this))return std::get<std::string>(*this);
        else return {};
    }
};


//
/**
 * @brief Layered map of symbols with scoping enabled.
 * 
 */
struct symbol_map{
    private:
        std::vector<std::map<std::string,symbol>> symbols;
    public:
        symbol_map(){
            new_frame();
        }

        /**
         * @brief Open a new scope
         * 
         */
        void new_frame(){
            symbols.push_back({});
        };

        /**
         * @brief Delete the last scope
         * 
         */
        void remove_frame(){
            symbols.pop_back();
        };

        /**
         * @brief Entirely clear the layered table.
         * 
         */
        void reset(){symbols=decltype(symbols)();}

        /**
         * @brief Clear the table and appply some initial data
         * 
         * @param src the initial data source to apply as first layer.
         */
        void reset(std::map<std::string,symbol>& src){symbols=decltype(symbols)();symbols.push_back(std::move(src));}

        /**
         * @brief Find a symbol layer by layer until first match
         * 
         * @param _name 
         * @return std::optional<symbol> empty if no match
         */
        std::optional<symbol> resolve(std::string_view name) const{
            for(auto it = symbols.rbegin();it!=symbols.rend();it++){
                auto found = it->find(std::string(name));
                if(found!=it->end()){
                    return found->second;
                }
            }
            return {};
        }

        /**
         * @brief Set the value of a symbol in the latest layer.
         * 
         * @param name 
         * @param value 
         */
        void set(const std::string& name, const auto& value){
            symbols.back().emplace(name,value);
        }

        /**
         * @brief Utility object used to simplify the process of creating and deleting layers of symbols.
         * 
         */
        struct guard_t{
            private:
                symbol_map& ref;
            public:
                inline guard_t(symbol_map& r):ref(r){ref.new_frame();}
                inline ~guard_t(){ref.remove_frame();}
        };

        /**
         * @brief Prepare a new guarded layer for this table of symbols.
         * 
         * @return guard_t 
         */
        inline guard_t guard(){return guard_t(*this);}
};

    
}
}