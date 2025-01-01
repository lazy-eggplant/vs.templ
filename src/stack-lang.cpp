#include <charconv>
#include <stack-lang.hpp>
#include <string>
#include <string_view>
#include <bit>

#include <frozen/unordered_map.h>
#include <frozen/string.h>

#define VS_OPERATOR_N_HELPER(OPERATOR, TYPE) \
{ +[](std::stack<concrete_symbol>& stack, size_t N){\
    TYPE ret = TYPE ();\
    for(size_t i = 0;i<N;i++){\
        if(stack.size()==0)return error_t::STACK_EMPTY;\
        auto tmp = std::move(stack.top());\
        stack.pop();\
        if(std::holds_alternative<  TYPE >(tmp))\
            ret OPERATOR ( std::get< TYPE >(tmp) );\
        else return error_t::WRONG_TYPE;\
    }\
    stack.push(ret);\
    return error_t::OK;\
}, 2, MAX_ARITY}

#define VS_OPERATOR_1_HELPER(OPERATOR, TYPE) \
{ +[](std::stack<concrete_symbol>& stack, size_t N){\
    TYPE ret;\
    if(stack.size()==0)return error_t::STACK_EMPTY;\
    auto tmp = std::move(stack.top());\
    stack.pop();\
    if(std::holds_alternative<  TYPE >(tmp))\
        ret = OPERATOR ( std::get< TYPE >(tmp) );\
    else return error_t::WRONG_TYPE;\
    stack.push(ret);\
    return error_t::OK;\
}, 1}

namespace vs{
namespace templ{
std::optional<concrete_symbol> repl::eval(const char* expr) noexcept{
    static const size_t MAX_ARITY = 100;
    static frozen::unordered_map<frozen::string, command_t, 22> commands = {
            {"nop", {+[](std::stack<concrete_symbol>& stack, size_t N){return repl::error_t::OK;}, 0}},
            {"cat", VS_OPERATOR_N_HELPER(+=,std::string)},

            ////Math operators
            {"add", VS_OPERATOR_N_HELPER(+=,int)},
            {"+", VS_OPERATOR_N_HELPER(+=,int)},
            {"sub", VS_OPERATOR_N_HELPER(-=,int)},
            {"-", VS_OPERATOR_N_HELPER(-=,int)},
            {"mul", VS_OPERATOR_N_HELPER(*=,int)},
            {"*", VS_OPERATOR_N_HELPER(*=,int)},
            {"div", VS_OPERATOR_N_HELPER(/=,int)},
            {"/", VS_OPERATOR_N_HELPER(/=,int)},
            {"mod", VS_OPERATOR_N_HELPER(%=,int)},
            {"%", VS_OPERATOR_N_HELPER(%=,int)},
            {"neg", VS_OPERATOR_1_HELPER(-,int)},
            {"-", VS_OPERATOR_1_HELPER(-,int)},
            //pow
            //log2

            ////Logic operators
            {"and", VS_OPERATOR_N_HELPER(&=,int)},
            {"or", VS_OPERATOR_N_HELPER(|=,int)},
            {"not", VS_OPERATOR_1_HELPER(~,int)},
            {"!", VS_OPERATOR_1_HELPER(~,int)},
            {"count.1", VS_OPERATOR_1_HELPER(__builtin_popcount,int)},  //I wanted to use std::popcount but it failed to be found for some reason :/
            {"count.0", VS_OPERATOR_1_HELPER(sizeof(int)*8-__builtin_popcount,int)},
                
            //{"lrot", VS_OPERATOR_N_HELPER(<<=,int)},
            //{"rrot", VS_OPERATOR_N_HELPER(>>=,int)},
            //{"lsh", VS_OPERATOR_N_HELPER(<<=,int)},
            //{"rsh", VS_OPERATOR_N_HELPER(>>=,int)},
            {"true", {+[](std::stack<concrete_symbol>& stack, size_t N){stack.push(true);return repl::error_t::OK;}, 0}},
            {"false", {+[](std::stack<concrete_symbol>& stack, size_t N){stack.push(false);return repl::error_t::OK;}, 0}},

            /* List of operators to implement is being developed in ./docs/repl-vm.md */

    };

    size_t current = 0;
    while(true){
        auto [status,begin,end,skip] = parse_token(expr+current);
        if(status==token_ret_t::OPERAND_FOUND){
            std::string ss(expr+current+begin,expr+current+end);
            printf("Found operand %s\n", ss.c_str());

            auto tmp = ctx.resolve_expr(std::string_view(expr+current+begin,expr+current+end));
            if(tmp.has_value()){
                stack.push(tmp.value());
            }
            else{
                //ERROR HERE
                return {};
            }

            current += skip;
        }
        else if(status==token_ret_t::OPERATOR_FOUND){
            std::string ss(expr+current+begin,expr+current+end);
            printf("Found operator %s\n", ss.c_str());

            {   
                int arity = -1;

                //Parsing operator to split name from arity specifier
                size_t i=current+begin;
                for(;i!=current+end && expr[i]!=':';i++);
                if(expr[i]==':'){
                    if(expr[i+1]=='*'){arity = stack.size();}
                    else{
                        std::from_chars(expr+i+1, expr+current+end, arity);
                    }
                }

                auto it = commands.find(std::string_view(expr+current+begin,expr+i));
                if(it!=commands.end()){
                    if(arity==-1)arity=it->second.default_arity;
                    if(arity>it->second.max_arity || arity<it->second.min_arity){
                        //ERROR HERE Bad arity
                        return {};
                    }
                    auto ret = it->second.fn(stack,arity);
                    if(ret!=error_t::OK){
                        //ERROR HERE Specify
                        return {};
                    }
                }
                else{
                    //ERROR HERE Command not found
                    return {};
                }
            }

            current += skip;
        }
        else if(status==token_ret_t::SKIP){
            current += skip;
        }
        else if(status==token_ret_t::END){
            break;
        }
        else{
            //ERROR HERE Parsing error
            return {};
        }
    }

    if(stack.size()!=1){
        //Error!
        return {};
    }
    return stack.top();
};


repl::repl(const preprocessor& _ctx):ctx(_ctx){}

/*
bool repl::parse(const char* str){
    bool is_operand=false;

    for(int i=0;str[i]!=0;){
        {   //Skip trailing whitespace
            int j=i;
            for(;(str[j]==' ' || str[j]=='\t' || str[j]=='\n') && str[j]!='\0';j++);
            i=j;
            if(str[j]=='\0')break;
        }

        if(str[i]=='`'){is_operand=!is_operand;i++;}

        if(is_operand){
            int j=i,old=i;
            for(;str[j]!='`' && str[j]!='\0';j++);
            is_operand=false;
            i=j;
            if(str[j]!='\0')i++;

            std::string ss(str+old,str+j);
            printf("Found operand %s\n", ss.c_str());

        }
        else{
            int j=i,old=i;
            for(;str[j]!=' ' && str[j]!='\t' && str[j]!='\n' && str[j]!='`' && str[j]!='\0';j++);
            is_operand=false;
            i=j;
            if(str[j]!='\0')i++;    //Evaluate again the last character over the next cycle.

            std::string ss(str+old,str+j);
            printf("Found operator %s\n", ss.c_str());
        }
    }

    return false;
}
*/

repl::token_ret_t repl::parse_token(const char* str, size_t max_length){
    bool is_operand=false;
    size_t i = 0;
    if(str[0]=='\0')return {repl::token_ret_t::END,0,0,0};
    {   //Skip trailing whitespace
        size_t j=i;
        for(;((str[j]==' ' || str[j]=='\t' || str[j]=='\n') && i<max_length) && str[j]!='\0';j++);
        i=j;
        if(str[j]=='\0')return {repl::token_ret_t::SKIP,0,i,i};
        else if(i==max_length)return {repl::token_ret_t::SKIP,0,i,i};
    }

    if(str[i]=='`'){is_operand=!is_operand;i++;}
    if(is_operand){
        size_t j=i,old=i;
        for(;str[j]!='`' && str[j]!='\0'  && i<max_length;j++);
        is_operand=false;
        i=j+1;

        if(str[j]=='`')return {repl::token_ret_t::OPERAND_FOUND,old,j,i};
        else return {repl::token_ret_t::INCOMPLETE,old,j,old};
    }
    else{
        size_t j=i,old=i;
        for(;str[j]!=' ' && str[j]!='\t' && str[j]!='\n' && str[j]!='`' && str[j]!='\0';j++);
        is_operand=false;
        i=j+1;

        if(str[j]=='\0')return {repl::token_ret_t::OPERATOR_FOUND,old,j,j};
        else if(i==max_length)return {repl::token_ret_t::INCOMPLETE,old,j,old};

        else return {repl::token_ret_t::OPERATOR_FOUND,old,j,i};
    }

    return {repl::token_ret_t::ERROR,0,0,0};
}


}
}

#undef VS_OPERATOR_1_HELPER
#undef VS_OPERATOR_N_HELPER