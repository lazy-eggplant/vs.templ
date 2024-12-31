#include <stack-lang.hpp>

namespace vs{
namespace templ{

std::optional<concrete_symbol> repl::eval(const preprocessor& p, pugi::xml_node* base) noexcept{
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

bool repl::parse(const char* str){
    bool is_operand=false;

    for(int i=0;str[i]!=0;){
        {int j=i;for(;str[j]==' ' || str[j]=='\t' || str[j]=='\n' || str[j]=='\0';j++);i=j;}
        if(str[i]=='`'){is_operand=!is_operand;i++;}

        if(is_operand){
            int j,old=i;
            for(;str[j]!='`' && str[j]!='\0';j++);
            is_operand=false;
            i=j;
            //TODO stringview given old and i
        }
        else{
            int j,old=i;
            for(;str[j]!=' ' && str[j]!='\t' && str[j]!='\n' && str[j]!='`' && str[j]!='\0';j++);
            is_operand=false;
            i=j;
            //TODO stringview
            if(str[j]!='\0')i--;    //Evaluate again the last character over the next cycle.
        }
    }
    //if whitespace, ignore up to the next not-whitespace
    //if ` tag the operand environment open.
    //if 

    //Identify blocks [...] as operands
    //Skip spaces
    //Other alphanumeric patterns are functions
}

}
}
       