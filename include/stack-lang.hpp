#pragma once
#include "vs-templ.hpp"
#include <stack>
#include <symbols.hpp>


namespace vs{
namespace templ{

/**
 * @brief 
 * 
 */
struct repl{
    enum struct return_t{
        OK,                     //No error actually everything ended up being right
        WRONG_TYPE,             //Type mismatch in the argument being used
        STACK_EMPTY,            //The stack is empty, I cannot take any further element
        STACK_STILL_FULL,       //The stack should only be left with one element when closing a program. If more are present, it is an error.
        UNKNOWN_OPERATOR,       //The requested operator does not exists.
        UNKNOWN_ARITY,          //The requested operator cannot operate with the specified arity.
        MEMORY,                 //No more internal memory can be allocated.
        NOT_IMPLEMENTED,        //Operator or path not yet implemented.
    };
    static constexpr const char* error_s(return_t e){
        if(e==return_t::OK)return "ok";
        else if(e==return_t::WRONG_TYPE)return "wrong type";
        else if(e==return_t::STACK_EMPTY)return "stack is empty";
        else if(e==return_t::STACK_STILL_FULL)return "stack has still items";
        else if(e==return_t::UNKNOWN_OPERATOR)return "no matching operator found";
        else if(e==return_t::UNKNOWN_ARITY)return "operator has the wrong arity";
        else if(e==return_t::MEMORY)return "unable to allocate memory";
        else if(e==return_t::NOT_IMPLEMENTED)return "the operator is not fully implemented yet";
        else return "illegal error";
    }

    private:
        struct token_ret_t{
            enum {
                OPERAND_FOUND, 
                OPERATOR_FOUND,
                SKIP,               //Whitespaces, comments etc.
                INCOMPLETE,         //The current token is incomplete (or potentially incomplete). Waiting for more content to know.
                ERROR,              //Generic syntax error.
                END                 //End of message met. (\0) for example.
            }msg;

            size_t begin;           //Where we the token match starts
            size_t end;             //Where the token match ends
            size_t next;            //Where to jump before the next token is considered, avoiding irrelevant characters in the middle.
        };

        struct op_t{
            std::string_view name;
            size_t operands;
        };

        std::stack<symbol> stack;

        const preprocessor& ctx;

        struct command_t{
            return_t (*fn)(std::stack<symbol>& stack, size_t N, const preprocessor* ctx);
            int min_arity = 1;
            int max_arity = min_arity;
            int default_arity = min_arity;
        };
        
        token_ret_t parse_token(const char* str, size_t max_length=-1);

    public:

        repl(const preprocessor& _ctx);

        std::optional<symbol> eval(const char* expr) noexcept;     
        bool push_operand(const symbol& ref)noexcept;
};

}
}