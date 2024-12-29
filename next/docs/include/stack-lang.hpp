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
        std::optional<concrete_symbol> eval(const preprocessor& p, pugi::xml_node* base=nullptr) noexcept{
            while(!stack.empty()){
                auto top = stack.top();
                stack.pop();
                if(top.first==item_t::EXPR)return p.resolve_expr(top.second, base);
                //If I end up with an error which I am not able to fix I will just return an empty result.
                else if (stack.size()==0 && top.first==item_t::ERROR)return {};
                else{
                    //Else extract from stack based on specs, perform operation, save on stack and repeat.


                    try{}
                    catch(...){}
                }
            }

            //Breaking failures or empty stack...
            return {};
        };

        bool parse(const char* str){
            //Identify blocks [...] as operands
            //Skip spaces
            //Other alphanumeric patterns are functions
        }

    public:
};

}
}