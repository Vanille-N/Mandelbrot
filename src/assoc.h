#pragma once

#include <string>
#include <unordered_map>

class Assoc {
public:
    void link (int, std::string) ;
    bool exists (int) ;
    bool exists (std::string) ;
    int operator[] (std::string) ;
    std::string operator[] (int) ;
private:
    std::unordered_map<int, std::string> str ;
    std::unordered_map<std::string, int> id ;
};
