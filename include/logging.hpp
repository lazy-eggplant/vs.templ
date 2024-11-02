#pragma once

#include <string>

namespace vs{
namespace templ{

struct log_t{
    enum values{
        ERROR
    };

    private:
        values _type;
        std::string _description;
    public:
        inline log_t(values type, std::string description):_type(type),_description(description){}

        values type() const{return _type;}
        const std::string& description() const{return _description;}
};

}
}