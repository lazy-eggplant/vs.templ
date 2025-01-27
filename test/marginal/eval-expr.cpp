#include "vs-templ.hpp"
#include <stack-lang.hpp>
using namespace vs::templ;

int main(){
    preprocessor pp({{},{}});
    repl repl(pp);
    repl.eval("`hello` `world` banana `hello2` cs 2 hello:2");
    return 0;
} 