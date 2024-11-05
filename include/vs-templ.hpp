#pragma once

#include <cstdlib>
#include <cstring>
#include <optional>
#include <stack>
#include <string>
#include <vector>

#include <pugixml.hpp>

#include "symbols.hpp"
#include "logging.hpp"

namespace vs{
namespace templ{


struct preprocessor{
    private:
        std::string ns_prefix;

        //Final document to be shared
        pugi::xml_document compiled;

        //To track operations within the parser
        std::stack<pugi::xml_node> stack_compiled;
        std::stack<pugi::xml_node_iterator> stack_data;
        std::stack<std::pair<pugi::xml_node_iterator,pugi::xml_node_iterator>> stack_template;

        std::vector<log_t> _logs;

        //Stack-like table of symbols
        symbol_map symbols;

        //Entry point in the root document.
        pugi::xml_node root_data;

    public:
        inline preprocessor(const pugi::xml_document& root_data, const pugi::xml_document& root_template, const char* prefix="s:"){
            init(root_data,root_template,prefix);
        }

        void init(const pugi::xml_document& root_data, const pugi::xml_document& root_template, const char* prefix="s:");
        void reset();

        inline const std::vector<log_t> logs(){return _logs;}
        inline void log(log_t::values type, const char* msg, ...){
            //TODO: Handling of panic should terminate the process right away (?)
            //TODO: Interpolate/format string
            _logs.emplace_back(type,msg);
        }

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
            const char *CALC_TAG;
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

            const char *CALC_SRC_PROP;
            const char *CALC_FORMAT_PROP;

            const char *USE_SRC_PROP;

            void prepare(const char * ns_prefix);

            inline ~ns_strings(){if(data!=nullptr)delete[] data;}

        }strings;

        //Transforming a string into a parsed symbol, setting an optional base root or leaving it to a default evaluation.
        std::optional<concrete_symbol> resolve_expr(const char* _str, const pugi::xml_node* base=nullptr);

        std::vector<pugi::xml_attribute> prepare_props_data(const pugi::xml_node& base, int limit, int offset, bool(*filter)(const pugi::xml_attribute&), order_method_t::values criterion);

        std::vector<pugi::xml_node> prepare_children_data(const pugi::xml_node& base, int limit, int offset, bool(*filter)(const pugi::xml_node&), const std::vector<std::pair<std::string,order_method_t::values>>& criteria);

        void _parse(std::optional<pugi::xml_node_iterator> stop_at);

};


}
}