#pragma once
#include "utils.hpp"
#include "vs-templ.hpp"
#include <stack>
#include <symbols.hpp>

namespace vs{
namespace templ{

struct repl{
    private:
        enum class item_t{
            ERROR, EXPR, FN
        };

        //TODO: Order by name & operands, so that shorter ones are selected first.
        struct op_t{
            std::string_view name;
            size_t operands;
        };

        std::stack<std::pair<item_t,std::string_view>> stack;

        /*
        Approach 1:
            Execution of a function which is not found results in a runtime error saved on stack.
            Types not being satisfied are evaluated dynamically by functions and will result in runtime errors saved on stack.
            Determinations about the numbers of element to pop from the stack are done at runtime as well.
        */
        std::optional<concrete_symbol> eval(const preprocessor& p, pugi::xml_node* base=nullptr) noexcept;

        bool parse(const char* str);
    public:
};

}
}