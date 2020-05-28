#include "assoc.h"

/* Assoc class provides an easy two-way correspondance between user keywords
 * and internal cmd representation.
 * The [] operator is overloaded so that
 * assoc[KW] == "kw" && assoc["kw"] == KW
 */

void Assoc::link (int num, std::string name) {
    str[num] = name ;
    id[name] = num ;
}

bool Assoc::exists (int num) {
    return str.find(num) != str.end() ;
}

bool Assoc::exists (std::string name) {
    return id.find(name) != id.end() ;
}

int Assoc::operator[] (std::string name) {
    return id[name] ;
}

std::string Assoc::operator[] (int num) {
    return str[num] ;
}
