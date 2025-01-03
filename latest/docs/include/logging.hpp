#pragma once

#include <string>

namespace vs{
namespace templ{

struct log_t{
    enum values{
        ERROR, WARNING, PANIC, INFO
    };

    private:
        values _type;
        std::string _msg;
    public:
        inline log_t(values type, std::string msg):_type(type),_msg(msg){}

        values type() const{return _type;}
        const std::string& msg() const{return _msg;}
};

}
}