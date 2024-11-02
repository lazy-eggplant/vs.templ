#pragma once

#include <utils.hpp>

namespace vs{
namespace templ{

//TODO: Implement anew
std::vector<std::string_view> split_string (const char* str, char delim) {
    std::vector<std::string_view> result;
    size_t i = 0, last = 0;
    for(;; i++){
        if(str[i]==','){result.emplace_back(str+i,i-last);i++;last=i;}
        if(str[i]==0){result.emplace_back(str+i,i-last);break;}
    }
    return result;
}

}
}