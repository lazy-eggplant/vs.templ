#pragma once

#include <cstdint>
#include <cstring>
#include <variant>
#include <vector>
#include <string>

namespace vs{
namespace templ{

template <typename T, typename... Args> struct concatenator;

template <typename... Args0, typename... Args1>
struct concatenator<std::variant<Args0...>, Args1...> {
    using type = std::variant<Args0..., Args1...>;
};

template<typename T>
const T& get_or(const auto& ref, const T& defval) noexcept{
    if(std::holds_alternative<T>(ref))return std::get<T>(ref);
    else return defval;
}

/**
 * @brief Generate a list of string views when a delimiter is matched on a reference string.
 * 
 * @param str the string to check
 * @param delim the delimiter character
 * @return std::vector<std::string_view> the list of slices
 */
std::vector<std::string_view> split_string (const char* str, char delim);

///Compute a const string size at comptime
inline constexpr std::size_t cexpr_strlen(const char* s){return std::char_traits<char>::length(s);}

///Compare strings where the right one is defined at comptime
inline bool cexpr_strneqv(const char* s, const char* c){return strncmp(s, c, cexpr_strlen(c))==0;}

///Compare strings where the right one is defined at comptime
inline bool cexpr_streqv(const char* s, const char* c){return strcmp(s, c)==0;}


/**
 * @brief Compare two strings assuming the dot notation for ordering nested fields
 * 
 * @param a 
 * @param b 
 * @return int 
 */
int cmp_dot_str(const char* a, const char* b);

/**
 * @brief Compare two strings (natural comparison) assuming the dot notation for ordering nested fields
 * 
 * @param a 
 * @param b 
 * @return int 
 */
int cmp_dot_natstr(const char* a, const char* b);

//TODO: Add natural ordering

namespace hash{
    void MurmurHash3_x64_128 ( const void * key, const int len,
                           const uint32_t seed, void * out );
}

}
}