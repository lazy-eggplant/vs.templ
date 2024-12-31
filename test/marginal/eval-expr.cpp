#include <stack-lang.hpp>
using namespace vs::templ;

int main(){
    repl repl;
    repl.parse("`hello` `world` banana `hello2` cs 2 hello:2");
    return 0;
}