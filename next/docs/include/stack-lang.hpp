#pragma once
#include "utils.hpp"
#include "vs-templ.hpp"
#include <stack>
#include <symbols.hpp>


namespace vs{
namespace templ{

struct repl{
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

    enum struct error_t{
        OK,                     //No error actually everything ended up being right
        WRONG_TYPE,             //Type mismatch in the argument being used
        STACK_EMPTY,            //The stack is empty, I cannot take any further element
        STACK_STILL_FULL,       //The stack should only be left with one element when closing a program. If more are present, it is an error.
        UNKNOWN_OPERATOR,       //The requested operator does not exists.
        UNKNOWN_ARITY,         //The requested operator cannot operate with the specified arity.
        MEMORY,                 //No more internal memory can be allocated.
    };

    private:
        struct op_t{
            std::string_view name;
            size_t operands;
        };

        std::stack<concrete_symbol> stack;

        const preprocessor& ctx;

        struct command_t{
            //const char* opname;
            error_t (*fn)(std::stack<concrete_symbol>& stack, size_t N);
            int min_arity = 1;
            int max_arity = min_arity;
            int default_arity = min_arity;
        };
        
    public:

        repl(const preprocessor& _ctx);

        /**
         * @brief 
         * 
         * @param expr 
         * @return std::optional<concrete_symbol> 
         */
        std::optional<concrete_symbol> eval(const char* expr) noexcept;     

        token_ret_t parse_token(const char* str, size_t max_length=-1);
};

}
}