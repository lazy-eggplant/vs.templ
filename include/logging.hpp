#pragma once

#include <string>

namespace vs{
namespace templ{

struct log_t{
    enum values{
        ERROR, WARNING, PANIC, INFO, LOG, OK
    };

    struct ctx{
        std::string template_path;
        ptrdiff_t template_offset;
        std::string generated_path;
        ptrdiff_t generated_offset;
        std::string data_path;
        ptrdiff_t data_offset;
        //const char* file = nullptr;
        //const char* path = nullptr;
        //int line_start = -1, line_end = -1, column_start = -1, column_end = -1;
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