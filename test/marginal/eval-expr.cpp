#include "vs-templ.hpp"
#include <format>
#include <iostream>
#include <stack-lang.hpp>
#include <variant>
#include <cassert>

using namespace vs::templ;


void logfn(log_t::values type, const char* msg, const log_t::ctx&){
    static const char* severity_table[] = {
    "\033[31;1m[ERROR]\033[0m    : ",
    "\033[33;1m[WARNING]\033[0m  : ",
    "\033[41;30;1m[PANIC]\033[0m    : ",
    "\033[34;1m[INFO]\033[0m     : ",
    };
    //TODO show context information
    std::cerr<<std::format("{}{} \033[33;1m@\033[0m {}",severity_table[type],msg,"xxx")<<"\n";
}


int main(){
    preprocessor pp({{},{}, "s", logfn});
    {
        repl repl(pp);
        auto symbol =  repl.eval("`1` `2` +");
        assert(symbol.has_value() && std::holds_alternative<int>(*symbol) && std::get<int>(*symbol) == 3);
    }
    {
        repl repl(pp);
        auto symbol =  repl.eval("`#o` `#l` `#l` `#e` cat:*");
        assert(symbol.has_value() && std::holds_alternative<std::string>(*symbol) && std::get<std::string>(*symbol) == "ello");
    }
    {
        repl repl(pp);
        auto symbol =  repl.eval("`#o` `#l` `#l` `#e` `#.` join:*");
        assert(symbol.has_value() && std::holds_alternative<std::string>(*symbol) && std::get<std::string>(*symbol) == "e.l.l.o");
    }
    {
        repl repl(pp);
        auto symbol =  repl.eval("`1` `2` bg");
        assert(symbol.has_value() && std::holds_alternative<int>(*symbol) && std::get<int>(*symbol) == true);
    }
    {
        repl repl(pp);
        auto symbol =  repl.eval("`1` `2` lt");
        assert(symbol.has_value() && std::holds_alternative<int>(*symbol) && std::get<int>(*symbol) == false);
    }
    return 0;
} 