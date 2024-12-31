#include <stack-lang.hpp>
#include <string_view>

namespace vs{
namespace templ{

std::optional<concrete_symbol> repl::eval(const char* expr) noexcept{
    size_t current = 0;
    while(true){
        auto [status,begin,end,skip] = parse_token(expr+current, 0xffff);
        if(status==token_ret_t::OPERAND_FOUND){
            std::string ss(expr+current+begin,expr+current+end);
            printf("Found operand %s\n", ss.c_str());
            current += skip;

            auto tmp = ctx.resolve_expr(std::string_view(expr+current+begin,expr+current+end));
            if(tmp.has_value())stack.push(tmp.value());
            else{
                //ERROR HERE
                return {};
            }
        }
        else if(status==token_ret_t::OPERATOR_FOUND){
            std::string ss(expr+current+begin,expr+current+end);
            printf("Found operator %s\n", ss.c_str());
            current += skip;

            //TODO Perform operation!

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

    return {};
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
       