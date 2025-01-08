#pragma once

/**
 * @file vs-templ.hpp
 * @author karurochari
 * @brief 
 * 
 * @copyright Copyright (c) 2024
 * 
 */

#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <optional>
#include <stack>
#include <string>
#include <string_view>
#include <vector>

#include <pugixml.hpp>

#include "symbols.hpp"
#include "logging.hpp"

namespace vs{
namespace templ{


struct logctx_t{
    const char* file = nullptr;
    const char* path = nullptr;
    int line_start = -1, line_end = -1, column_start = -1, column_end = -1;
};

/**
 * @brief Type of a logging function passed to the preprocessor
 */
typedef  void (*logfn_t)(log_t::values, const char* str, const logctx_t& ctx);

/**
 * @brief Type of a function to load an xml document given a path. Used to load templates for `include` commands.
 */
typedef bool (*includefn_t)(const char* path, pugi::xml_document&);

/**
 * @brief Type of a function to load an xml document given a path. Used to load data for `data` commands.
 */
typedef bool (*loadfn_t)(const pugi::xml_node cfg, pugi::xml_document&);

struct preprocessor{

    inline static void default_logfn(log_t::values, const char* str, const logctx_t& ctx){}
    inline static bool default_includefn(const char* path, pugi::xml_document&){return false;}
    inline static bool default_loadfn(const pugi::xml_node cfg, pugi::xml_document& ){return false;}

    private:
        friend struct repl;

        std::string ns_prefix;
        uint64_t seed;
        logfn_t logfn = default_logfn;
        includefn_t includefn = default_includefn;
        loadfn_t loadfn = default_loadfn;

        //Final document to be shared
        pugi::xml_document compiled;

        //To track operations within the parser
        std::stack<pugi::xml_node> stack_compiled;
        std::stack<std::pair<pugi::xml_node_iterator,pugi::xml_node_iterator>> stack_template;

        //Stack-like table of symbols
        symbol_map symbols;

        //Entry point in the root document.
        pugi::xml_node root_data;

    public:
        inline preprocessor(const pugi::xml_node& root_data, const pugi::xml_node& root_template, const char* prefix="s:", logfn_t logfn = default_logfn, includefn_t includefn = default_includefn,  loadfn_t loadfn = default_loadfn, uint64_t seed = 0){
            init(root_data,root_template,prefix,logfn,includefn,loadfn,seed);
        }

        void init(const pugi::xml_node& root_data, const pugi::xml_node& root_template, const char* prefix="s:", logfn_t logfn = default_logfn, includefn_t includefn = default_includefn,  loadfn_t loadfn = default_loadfn,  uint64_t seed = 0);
        void reset();

        /**
         * @brief Reset the symbols and introduce an environment
         * 
         * @param env a table of symbols to introduce in the preprocessor
         */
        inline void load_env(std::map<std::string,symbol>& env){symbols.reset(env);symbols.set("$",root_data);}

        inline pugi::xml_document& parse(){_parse({});return compiled;}
        inline void ns(const char* str){ns_prefix = str;strings.prepare(str);}

        std::array<uint64_t,2> hash(const symbol& ref);

    private:
        struct order_method_t{
            enum values{
                UNKNOWN =  0, 
                ASC, 
                DESC, 
                RANDOM,

                USE_DOT_EVAL = 16 //For strings, split evaluation based on their dot groups. Valid for all methods.
            };

            static values from_string(std::string_view str);
        };

        //Precomputed string to avoid spawning an absurd number of small objects in heap at each cycle.
        struct ns_strings{
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

            const char *TEST_TAG;
                const char *CASE_TAG;

            const char *VALUE_TAG;
            const char *ELEMENT_TAG;
                const char *TYPE_ATTR;

            const char *LOG_TAG;

            const char *INCLUDE_TAG;
            const char *DATA_TAG;

            //S:PROPS
            const char *FOR_SRC_PROP;
            const char *FOR_FILTER_PROP;
            const char *FOR_SORT_BY_PROP;
            const char *FOR_ORDER_BY_PROP;
            const char *FOR_OFFSET_PROP;
            const char *FOR_LIMIT_PROP;

            const char *FOR_PROPS_SRC_PROP;
            const char *FOR_PROPS_FILTER_PROP;
            const char *FOR_PROPS_ORDER_BY_PROP;
            const char *FOR_PROPS_OFFSET_PROP;
            const char *FOR_PROPS_LIMIT_PROP;

            const char *VALUE_PROP;

            const char *PROP_NAME_PROP;
            const char *PROP_VALUE_PROP;

            const char *WHEN_PROP;

            void prepare(const char * ns_prefix);

            inline ~ns_strings(){if(data!=nullptr)delete[] data;}

        }strings;

        //Transforming a string into a parsed symbol, setting an optional base root or leaving it to a default evaluation.
        std::optional<concrete_symbol> resolve_expr(const std::string_view& str, const pugi::xml_node* base=nullptr) const;

        std::vector<pugi::xml_attribute> prepare_props_data(const pugi::xml_node& base, int limit, int offset, const char *filter, order_method_t::values criterion);

        std::vector<pugi::xml_node> prepare_children_data(const pugi::xml_node& base, int limit, int offset, const char *filter, const std::vector<std::pair<std::string,order_method_t::values>>& criteria);

        void _parse(std::optional<pugi::xml_node_iterator> stop_at);

        void log(log_t::values, const std::string&) const;

};


}
}