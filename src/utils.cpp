#include <utils.hpp>

namespace vs{
namespace templ{

std::vector<std::string_view> split_string (const char* str, char delim) {
    std::vector<std::string_view> result;
    size_t i = 0, last = 0;
    for(;; i++){
        if(str[i]==delim){result.emplace_back(str+last,i-last);i++;last=i;}
        if(str[i]==0){result.emplace_back(str+last,i-last);break;}
    }
    return result;
}

int cmp_dot_str(const char* a, const char* b){
    auto va = split_string(a, '.');
    auto vb = split_string(b, '.');

    if(va.size()<vb.size())return -1;
    else if(va.size()>vb.size())return 1;

    for(int i =0;i<va.size();i++){
        if(va.at(i)<vb.at(i))return -1;
        else if(va.at(i)>vb.at(i))return 1;
    }
    //TODO: Implement logic
    return -1;
}

}
}