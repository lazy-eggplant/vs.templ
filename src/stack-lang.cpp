#include <stack-lang.hpp>
#include <string_view>


#include <frozen/unordered_map.h>
#include <frozen/string.h>

namespace vs{
namespace templ{
std::optional<concrete_symbol> repl::eval(const char* expr) noexcept{
    static const size_t MAX_ARITY = 100;
    static frozen::unordered_map<frozen::string, command_t, 2> commands = {
            {"nop", {+[](std::stack<concrete_symbol>& stack, size_t N){return repl::error_t::OK;}, 0}},
            {"cat", { +[](std::stack<concrete_symbol>& stack, size_t N){
                            std::string ret;
                            for(size_t i = 0;i<N;i++){
                                if(stack.size()==0)return error_t::STACK_EMPTY;
                                auto tmp = stack.top();
                                if(std::holds_alternative<std::string>(tmp))
                                    ret+=std::get<std::string>(tmp);
                                else return error_t::WRONG_TYPE;
                            }
                            stack.push(ret);
                            return error_t::OK;
                        }, 2, MAX_ARITY}}


    };
    

    size_t current = 0;
    while(true){
        auto [status,begin,end,skip] = parse_token(expr+current);
        if(status==token_ret_t::OPERAND_FOUND){
            std::string ss(expr+current+begin,expr+current+end);
            //printf("Found operand %s\n", ss.c_str());

            std::string sv(expr+current+begin,expr+current+end);

            auto tmp = ctx.resolve_expr(sv);
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
            //printf("Found operator %s\n", ss.c_str());

            //TODO Perform operation!
            current += skip;
        }
        else if(status==token_ret_t::SKIP){
            current += skip;
        }
        else if(status==token_ret_t::END){
            break;
        }
        else{
            //ERROR HERE
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
       