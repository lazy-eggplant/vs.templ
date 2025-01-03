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
typedef  void (*logfn_t)(log_t::values, const char* str, const logctx_t& ctx);

struct preprocessor{

    inline static void default_logfn(log_t::values, const char* str, const logctx_t& ctx){}

    private:
        friend struct repl;

        std::string ns_prefix;
        uint64_t seed;
        logfn_t logfn = default_logfn;

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
        inline preprocessor(const pugi::xml_node& root_data, const pugi::xml_node& root_template, const char* prefix="s:", logfn_t logfn = default_logfn, uint64_t seed = 0){
            init(root_data,root_template,prefix,logfn,seed);
        }

        void init(const pugi::xml_node& root_data, const pugi::xml_node& root_template, const char* prefix="s:", logfn_t logfn = default_logfn, uint64_t seed = 0);
        void reset();

        inline pugi::xml_document& parse(){_parse({});return compiled;}
        inline void ns(const char* str){ns_prefix = str;strings.prepare(str);}

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

            const char *WHEN_TAG;
                const char *IS_TAG;

            const char *VALUE_TAG;
            const char *ELEMENT_TAG;
                const char *TYPE_ATTR;

            const char *LOG_TAG;

            //S:PROPS
            const char *FOR_IN_PROP;
            const char *FOR_SRC_PROP;
            const char *FOR_FILTER_PROP;
            const char *FOR_SORT_BY_PROP;
            const char *FOR_ORDER_BY_PROP;
            const char *FOR_OFFSET_PROP;
            const char *FOR_LIMIT_PROP;

            const char *FOR_PROPS_IN_PROP;
            const char *FOR_PROPS_SRC_PROP;
            const char *FOR_PROPS_FILTER_PROP;
            const char *FOR_PROPS_ORDER_BY_PROP;
            const char *FOR_PROPS_OFFSET_PROP;
            const char *FOR_PROPS_LIMIT_PROP;

            const char *VALUE_SRC_PROP;
            const char *VALUE_EXPR_PROP;
            const char *VALUE_FORMAT_PROP;

            const char *USE_SRC_PROP;

            void prepare(const char * ns_prefix);

            inline ~ns_strings(){if(data!=nullptr)delete[] data;}

        }strings;

        //Transforming a string into a parsed symbol, setting an optional base root or leaving it to a default evaluation.
        std::optional<concrete_symbol> resolve_expr(const std::string_view& str, const pugi::xml_node* base=nullptr) const;

        std::vector<pugi::xml_attribute> prepare_props_data(const pugi::xml_node& base, int limit, int offset, bool(*filter)(const pugi::xml_attribute&), order_method_t::values criterion);

        std::vector<pugi::xml_node> prepare_children_data(const pugi::xml_node& base, int limit, int offset, bool(*filter)(const pugi::xml_node&), const std::vector<std::pair<std::string,order_method_t::values>>& criteria);

        void _parse(std::optional<pugi::xml_node_iterator> stop_at);

        void log(log_t::values, const std::string&) const;

};


}
}