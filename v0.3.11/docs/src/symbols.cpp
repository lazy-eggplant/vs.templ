#include <symbols.hpp>
namespace vs{
namespace templ{

std::optional<std::string> to_string(const concrete_symbol& val){
    if(std::holds_alternative<int>(val))return std::to_string(std::get<int>(val));
    else if(std::holds_alternative<float>(val))return std::to_string(std::get<float>(val));
    else if(std::holds_alternative<std::string>(val))return std::get<std::string>(val);
    else return {};
}

}
}
