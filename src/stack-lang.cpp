#include <cmath>
#include <format>
#include <string>
#include <string_view>
#include <variant>

#if (defined(__GNUC__) && __GNUC__ >= 13) || (defined(__clang__) && __clang_major__ >= 20) ||  (defined(_MSC_VER) &&  _MSC_VER >= 1910)
#include <charconv>
using std::from_chars;
#else
#include <fast_float/fast_float.h>
using fast_float::from_chars;
#endif

#include <frozen/unordered_map.h>
#include <frozen/string.h>

#include "logging.hpp"
#include "stack-lang.hpp"

#define VS_OPERATOR_N_MATH_HELPER(OPERATOR) \
{ +[](std::stack<symbol>& stack, size_t N, const preprocessor* ctx){\
    enum {NONE, INT, FLOAT} type = NONE;\
    int ret_i = 0;\
    float ret_f = 0.0;\
    for(size_t i = 0;i<N;i++){\
        auto tmp = std::move(stack.top());\
        stack.pop();\
        if(type!=FLOAT && is<int>(tmp)){\
            auto& ret = ret_i;\
            if(i==0)ret=as<int>(tmp);\
            else ret OPERATOR ( as<int>(tmp) );\
            type = INT;\
        }\
        else if(type!=INT && is<float>(tmp)){\
            auto& ret = ret_f;\
            if(i==0)ret=as<float>(tmp);\
            else ret OPERATOR ( as<float>(tmp) );\
            type = FLOAT;\
        }\
        else return error_t::WRONG_TYPE;\
    }\
    if(type==INT)stack.push(ret_i);\
    else if(type==FLOAT)stack.push(ret_f);\
    return error_t::OK;\
}, 2, MAX_ARITY}

#define VS_OPERATOR_1_MATH_HELPER(OPERATOR) \
{ +[](std::stack<symbol>& stack, size_t N, const preprocessor* ctx){\
    enum {NONE, INT, FLOAT} type;\
    int ret_i;\
    float ret_f;\
    auto tmp = std::move(stack.top());\
    stack.pop();\
    if(is<int>(tmp)){\
        auto& ret = ret_i;\
        ret = OPERATOR ( as<int>(tmp) );\
        type = INT;\
    }\
    else if(is<float>(tmp)){\
        auto& ret = ret_f;\
        ret = OPERATOR ( as<float>(tmp) );\
        type = FLOAT;\
    }\
    else return error_t::WRONG_TYPE;\
    if(type==INT)stack.push(ret_i);\
    else if(type==FLOAT)stack.push(ret_f);\
    return error_t::OK;\
}, 1}

#define VS_OPERATOR_CMP_HELPER(OPERATOR) \
{ +[](std::stack<symbol>& stack, size_t N, const preprocessor* ctx){\
    auto a = std::move(stack.top());\
    stack.pop();\
    auto b = std::move(stack.top());\
    stack.pop();\
    ctx->compare_symbols(a,b,preprocessor::order_t{});\
    if(is<int>(a)){\
        if(is<int>(b)){\
            stack.push(as<int>(a) OPERATOR as<int>(b));\
        }\
        else return error_t::WRONG_TYPE;\
    }\
    else if(is<float>(a)){\
        if(is<float>(b)){\
            stack.push( as<float>(a) OPERATOR  as<float>(b));\
        }\
        else return error_t::WRONG_TYPE;\
    }\
    else if(is<std::string>(a)){\
        if(is<std::string>(b)){\
            stack.push( as<std::string>(a) OPERATOR  as<std::string>(b));\
        }\
        else return error_t::WRONG_TYPE;\
    }\
    else return error_t::WRONG_TYPE;\
    return error_t::OK;\
}, 2}

#define VS_OPERATOR_N_HELPER(OPERATOR, TYPE) \
{ +[](std::stack<symbol>& stack, size_t N, const preprocessor* ctx){\
    TYPE ret = TYPE ();\
    for(size_t i = 0;i<N;i++){\
        auto tmp = std::move(stack.top());\
        stack.pop();\
        if(is< TYPE >(tmp))\
            ret OPERATOR ( as< TYPE >(tmp) );\
        else return error_t::WRONG_TYPE;\
    }\
    stack.push(ret);\
    return error_t::OK;\
}, 2, MAX_ARITY}

#define VS_OPERATOR_1_HELPER(OPERATOR, TYPE) \
{ +[](std::stack<symbol>& stack, size_t N, const preprocessor* ctx){\
    TYPE ret;\
    auto tmp = std::move(stack.top());\
    stack.pop();\
    if(is<  TYPE >(tmp))\
        ret = OPERATOR ( as< TYPE >(tmp) );\
    else return error_t::WRONG_TYPE;\
    stack.push(ret);\
    return error_t::OK;\
}, 1}

namespace vs{
namespace templ{


//Helpers just to make the code less of a mess
template<class T>
static inline constexpr bool is (const auto& v) { return std::holds_alternative<T>(v); }
 
template<class T>
static inline constexpr auto& as (const auto& v) { return std::get<T>(v); }


bool repl::push_operand(const symbol& ref)noexcept{
    stack.push(ref);
    //TODO: for now the assumption is no memory failure
    return true;
}

std::optional<symbol> repl::eval(const char* expr) noexcept{
    static const size_t MAX_ARITY = 100;
    static frozen::unordered_map<frozen::string, command_t, 39> commands = {
            {"nop", {+[](std::stack<symbol>& stack, size_t N, const preprocessor* ctx){return error_t::OK;}, 0}},
            {"(", {+[](std::stack<symbol>& stack, size_t N, const preprocessor* ctx){return error_t::OK;}, 0}},
            {")", {+[](std::stack<symbol>& stack, size_t N, const preprocessor* ctx){return error_t::OK;}, 0}},

            {"rem", {+[](std::stack<symbol>& stack, size_t N, const preprocessor* ctx){for(size_t i=0;i<N;i++){stack.pop();}return repl::error_t::OK;}, 1, MAX_ARITY}},
            {"dup", {+[](std::stack<symbol>& stack, size_t N, const preprocessor* ctx){stack.push(stack.top());return repl::error_t::OK;}, 1}},

            {"cat", VS_OPERATOR_N_HELPER(+=,std::string)},
            {"join", {+[](std::stack<symbol>& stack, size_t N, const preprocessor* ctx){
                            std::string ret;
                            std::string sep;
                            {
                                auto tmp = std::move(stack.top());
                                stack.pop();
                                if(is<std::string>(tmp))
                                    sep = std::move( as<std::string>(tmp) );
                                else return error_t::WRONG_TYPE;
                            }
                            for(size_t i = 1;i<N;i++){
                                auto tmp = std::move(stack.top());
                                stack.pop();
                                if(is<std::string>(tmp)){
                                    if(i!=1)ret += sep + ( as<std::string>(tmp) );
                                    else ret+=( as<std::string>(tmp) );
                                }
                                else return error_t::WRONG_TYPE;
                            }
                            stack.push(ret);
                            return error_t::OK;
            }, 3, MAX_ARITY}},

            //JOIN

            ////Math operators
            {"add", VS_OPERATOR_N_MATH_HELPER(+=)},
            {"+", VS_OPERATOR_N_MATH_HELPER(+=)},
            {"sub", VS_OPERATOR_N_MATH_HELPER(-=)},
            {"-", VS_OPERATOR_N_MATH_HELPER(-=)},
            {"mul", VS_OPERATOR_N_MATH_HELPER(*=)},
            {"*", VS_OPERATOR_N_MATH_HELPER(*=)},
            {"div", VS_OPERATOR_N_MATH_HELPER(/=)},
            {"/", VS_OPERATOR_N_MATH_HELPER(/=)},
            {"neg", VS_OPERATOR_1_MATH_HELPER(-)},

            {"mod", VS_OPERATOR_N_HELPER(%=, int)},
            {"%", VS_OPERATOR_N_HELPER(%=, int)},
            //{"log2", VS_OPERATOR_1_HELPER(std::log2, int)},
            //pow
            //log2

            ///Comparisons
            {"eqv", VS_OPERATOR_CMP_HELPER(==)},
            {"neq", VS_OPERATOR_CMP_HELPER(!=)},
            {"bg", VS_OPERATOR_CMP_HELPER(>)},
            {"bge", VS_OPERATOR_CMP_HELPER(>=)},
            {"lt", VS_OPERATOR_CMP_HELPER(<)},
            {"lte", VS_OPERATOR_CMP_HELPER(<=)},

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

            {"as.int", {+[](std::stack<symbol>& stack, size_t N, const preprocessor* ctx){
                auto t=std::move(stack.top());
                stack.pop();
                if(is<int>(t))stack.push(as<int>(t));
                else if(is<float>(t))stack.push((int)as<float>(t));
                else if(is<std::string>(t)){auto& str = as<std::string>(t);int result{};from_chars(str.c_str(),str.c_str()+str.size(),result);stack.push(result);}
                else return error_t::WRONG_TYPE;
                return error_t::OK;
            }, 1}},
            {"as.bool", {+[](std::stack<symbol>& stack, size_t N, const preprocessor* ctx){
                auto t=std::move(stack.top());
                stack.pop();
                if(is<int>(t))stack.push(as<int>(t)&1);
                else if(is<std::string>(t)){
                    auto& str = as<std::string>(t);
                    if(str=="false")stack.push(1);
                    else if(str=="true")stack.push(0);
                }
                else return error_t::WRONG_TYPE;
                return error_t::OK;
            }, 1}},
            {"as.float", {+[](std::stack<symbol>& stack, size_t N, const preprocessor* ctx){
                auto t=std::move(stack.top());
                stack.pop();
                if(is<int>(t))stack.push((float)as<int>(t));
                else if(is<float>(t))stack.push(as<float>(t));
                else if(is<std::string>(t)){auto& str = as<std::string>(t);float result{};from_chars(str.c_str(),str.c_str()+str.size(),result);stack.push(result);}
                else return error_t::WRONG_TYPE;
                return error_t::OK;
            }, 1}},
            {"as.str", {+[](std::stack<symbol>& stack, size_t N, const preprocessor* ctx){
                auto t=std::move(stack.top());
                stack.pop();
                if(is<int>(t))stack.push(std::to_string(as<int>(t)));
                else if(is<float>(t))stack.push(std::to_string(as<float>(t)));
                else if(is<std::string>(t))stack.push(as<std::string>(t));
                else return error_t::WRONG_TYPE;
                return error_t::OK;
            }, 1}},


            ///Constants
            {"APOS", {+[](std::stack<symbol>& stack, size_t N, const preprocessor* ctx){stack.push("`");return error_t::OK;}, 0}},
            {"PIPE", {+[](std::stack<symbol>& stack, size_t N, const preprocessor* ctx){stack.push("|");return error_t::OK;}, 0}},
            {"true", {+[](std::stack<symbol>& stack, size_t N, const preprocessor* ctx){stack.push(true);return error_t::OK;}, 0}},
            {"false", {+[](std::stack<symbol>& stack, size_t N, const preprocessor* ctx){stack.push(false);return error_t::OK;}, 0}},
            {"?", {+[](std::stack<symbol>& stack, size_t N, const preprocessor* ctx){
                auto condition = stack.top();
                stack.pop();
                auto if_true = stack.top();
                stack.pop();
                auto if_false = stack.top();
                stack.pop();
                if(!is<int>(condition))return error_t::WRONG_TYPE;
                if((as<int>(condition)&1)==0)stack.push(if_true);
                else stack.push(if_false);
                return error_t::OK;
            }, 3, 3}},

            ///Comparison operators
            /* List of operators to implement is being developed in ./docs/repl-vm.md */

    };

    size_t current = 0;
    while(true){
        auto [status,begin,end,skip] = parse_token(expr+current);
        if(status==token_ret_t::OPERAND_FOUND){
            std::string ss(expr+current+begin,expr+current+end);
            //printf("Found operand %s\n", ss.c_str());

            auto tmp = ctx.resolve_expr(std::string_view(expr+current+begin,expr+current+end));
            if(tmp.has_value()){
                stack.push(tmp.value());
                //TODO: for now the assumption is no memory failure
            }
            else{
                ctx.log(log_t::PANIC,std::format("VM Error: cannot insert empty object on stack @{}",current+begin));
                return {};
            }

            current += skip;
        }
        else if(status==token_ret_t::OPERATOR_FOUND){
            std::string ss(expr+current+begin,expr+current+end);
            //printf("Found operator %s\n", ss.c_str());

            {   
                int arity = -1;

                //Parsing operator to split name from arity specifier
                size_t i=current+begin;
                for(;i!=current+end && expr[i]!=':';i++);
                if(expr[i]==':'){
                    if(expr[i+1]=='*'){arity = stack.size();}
                    else{
                        from_chars(expr+i+1, expr+current+end, arity,10);
                    }
                }

                auto command_name =std::string_view(expr+current+begin,expr+i);
                auto it = commands.find(command_name);
                if(it!=commands.end()){
                    if(arity==-1)arity=it->second.default_arity;
                    if(arity>it->second.max_arity || arity<it->second.min_arity){
                        ctx.log(log_t::PANIC,std::format("VM Error: arity `{}` asked for command `{}` @{} but it must be in [{},{}]",arity,command_name,current+begin,it->second.min_arity ,it->second.max_arity ));
                        return {};
                    }
                    if(arity>(int)stack.size()){
                        ctx.log(log_t::PANIC,std::format("VM Error: arity asked for command `{}` @{} but the stack has less {}",command_name,current+begin,arity));
                        return {};
                    }
                    auto ret = it->second.fn(stack,arity, &ctx);
                    if(ret!=error_t::OK){
                        ctx.log(log_t::ERROR,std::format("VM Error: the operator `{}` @{} returned with error `{}`",command_name,current+begin,error_s(ret)));
                        return {};
                    }
                }
                else{
                    ctx.log(log_t::PANIC,std::format("VM Error: asked for command `{}` @{} but there is no implementation of it",command_name,current+begin));
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
            ctx.log(log_t::PANIC,std::format("VM Error: parsing error @{}",current+begin));
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

#undef VS_OPERATOR_N_MATH_HELPER
#undef VS_OPERATOR_1_MATH_HELPER
#undef VS_OPERATOR_CMP_HELPER
#undef VS_OPERATOR_1_HELPER
#undef VS_OPERATOR_N_HELPER