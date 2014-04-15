#ifndef STRINGUTILS_HPP
#define STRINGUTILS_HPP

#include <vector>
#include <string>
#include <sstream>
#include <algorithm>
#include <functional>
#include <cctype>
#include <locale>
#include <cstring>


typedef std::vector<std::string> stringVec;

namespace StrUtil{

// split a string following an array of delimiter ( strtok like )
std::vector<std::string> tokenSplit(const std::string & str, const std::string & delimiter);

// compare two C++ string case insensitive
int compare_ncase(const std::string & str1, const std::string & str2);

int compare_ncase(const std::string & str1, off_t offset, size_t size, const char* cstr2);

inline size_t copy_std_string_to_buff(char* buffer, size_t max_size, const std::string & str){
    const size_t str_size = str.copy(buffer, max_size-1);
    buffer[str_size]= '\0';
    return str_size;
}

// trim from start
template <typename Func>
inline std::string &ltrim(std::string &s, Func  pred = static_cast<int (*)(int)>(std::isspace)) {
        s.erase(s.begin(), std::find_if(s.begin(), s.end(), std::not1(std::ptr_fun<int, int>(pred))));
        return s;
}

// trim from end
template <typename Func>
inline std::string &rtrim(std::string &s, Func pred = static_cast<int (*)(int)>(std::isspace)) {
        s.erase(std::find_if(s.rbegin(), s.rend(), std::not1(std::ptr_fun<int, int>(pred))).base(), s.end());
        return s;
}

// trim from both
template <typename Func>
inline std::string &trim(std::string &s, Func pred = static_cast<int (*)(int)>(std::isspace)) {
    return ltrim<Func>(rtrim<Func>(s, static_cast<int (*)(int)>(pred)), static_cast<int (*)(int)>(pred));
}

inline std::vector<std::string> &split(const std::string &s, char delim, std::vector<std::string> &res) {
    std::stringstream ss(s);
    std::string item;
    item.reserve(s.size());
    while (std::getline(ss, item, delim)) {
        res.push_back(item);
    }
    return res;
}


inline int isslash(int c){
    return ( c == '/');
}

inline int isCrLf(int c){
    return ( c == '\n') || (c == '\n');
}

inline std::string & remove(std::string & str, char c){
    std::string::iterator it = std::remove(str.begin(), str.end(), c);
    str.erase(it, str.end());
    return str;
}


// predicates
inline bool charEqCase(char c1, char c2){
    return (c1 == c2 || ::tolower(c1) == ::tolower(c2));
}


}




#endif // STRINGUTILS_HPP
