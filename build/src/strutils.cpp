#include "strutils.h"

char random_chr () {
    if (rand() % 2) {
        return (char)('a' + rand()%26) ;
    } else {
        return (char)('A' + rand()%26) ;
    }
}

std::string random_name () {
    srand((unsigned int)(time(NULL) + pic_vresol * (view_hi - view_lo) + pic_hresol * (view_rt - view_lt))) ;
    char name [5] ;
    for (int i = 0; i < 5; i++) {
        name[i] = random_chr() ;
    }
    return std::string(name) ;
}

bool fexists (std::string name) {
    system(("ls " + name + "* 1>.x.txt 2>/dev/null").c_str()) ;
    std::ifstream f (".x.txt") ;
    std::string line ;
    return (getline(f, line) && line.length() > 0) ;
}

void hash_name () {
    do {
        curr_name = random_name() ;
    } while(fexists(curr_name)) ;
}

bool is_portable_char(char c) {
    return '0' <= c && c <= '9'
        || 'a' <= c && c <= 'z'
        || 'A' <= c && c <= 'Z'
        || c == '.'
        || c == '-'
        || c == '_' ;
}

std::string filename_sanitize (std::string fname) {
    std::ostringstream str ;
    int portable = 0 ;
    for (char c: fname) {
        if (portable == 2 || is_portable_char(c)) {
            str << c ;
        } else if (portable == 0) {
            char ans = log_warn(NPORTABLE, "(" + std::string(1, c) + ") remove from name ? (y/n)") ;
            if (ans == 'n') {
                portable = 2 ;
                str << c ;
            } else {
                portable = 1 ;
            }
        }
    }
    return str.str();
}
