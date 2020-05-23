#include "parse.h"


/* Cut the string into tokens :
 * find beginning and end of keywords, integers, and strings
 * separate all other symbols
 */
void tokenize () {
    tokens.clear() ;
    int idx = 0, len = 0 ;
    types_chr curr ;
    while (idx < (int)command.length()) {
        curr = chr_type(command[idx]) ;
        if (curr == KEYWORD) {
            len = 0;
            while (idx+len < (int)command.length() && chr_type(command[idx+(++len)]) == KEYWORD) {}
            tokens.push_back({idx, len}) ;
            idx += len ;
        } else if (curr == SYMBOL || curr == SELECTOR || curr == INDICATOR) {
            tokens.push_back({idx, 1}) ;
            idx++ ;
        } else if (curr == MODIFIER) {
            len = 0 ;
            while (idx+len < (int)command.length() && chr_type(command[idx+(++len)]) != BLANK) {}
            tokens.push_back({idx, len}) ;
            idx += len ;
        } else if (curr == NUMERIC) {
            len = 0 ;
            while (idx+len < (int)command.length() && chr_type(command[idx+(++len)]) == NUMERIC) {}
            tokens.push_back({idx, len}) ;
            idx += len;
        } else if (curr == BLANK) {
            idx++ ;
        } else if (curr == UNKNOWN) {
            tokens.push_back({-1, -1}) ;
            log_info(UNKCHR, "'" + command.substr(idx, 1) + "' not recognized") ;
            break ;
        }
    }
}

/* Split command line into usable commands
 */
void parse () {
    exec.clear() ;
    bool nameset = false ;
    if (command.length() > cmd_maxlen) {
        char ans = log_warn(LONGCMD, command.substr(0, 5) + "... truncate ? (y/n)") ;
        if (ans == 'n') {
            exec.push_back(ABORT) ;
            return ;
        } else {
            command = command.substr(0, cmd_maxlen) ;
        }
    }
    tokenize() ;
    for (int i = 0; i < (int)tokens.size(); i++) {
        if (tokens[i].beg == -1) {
            exec.push_back(ABORT) ;
            return ;
        }
    }
    for (int i = 0; i < (int)tokens.size(); i++) {
        if (command[tokens[i].beg] == '\'') {
            auto name = str_parse(tokens[i].beg, tokens[i].len) ;
            if (name.length() == 0) {
                log_err(PARSE, "Process aborted : no name specified") ;
                exec.push_back(ABORT) ;
            } else if (nameset) {
                char ans = log_warn(RENAME, "Overwrite ? (y/n)") ;
                if (ans == 'y') {
                    curr_name = name ;
                } else {
                    continue ;
                }
            } else {
                exec.push_back(STR) ;
                curr_name = name ;
                nameset = true ;
            }
        } else if (is_num(command[tokens[i].beg])) {
            int n = int_parse(tokens[i].beg, tokens[i].len) ;
            if (n == -1) {
                log_err(PARSE, command.substr(tokens[i].beg, 5) + "... not a valid quantifier") ;
                exec.push_back(ABORT) ;
            } else {
                exec.push_back(NUM) ;
                exec.push_back((cmd)n) ;
            }
        } else {
            auto atom = command.substr(tokens[i].beg, tokens[i].len) ;
            if (kw.exists(atom)) {
                exec.push_back((cmd)kw[atom]) ;
            } else {
                log_err(NOSUCHKW, atom.substr(0, std::min(5, (int)atom.length())) + (atom.length() > 5 ? "..." : "")) ;
                exec.push_back(ABORT) ;
            }
        }
    }
    return ;
}

bool is_num (char c) {
    return '0' <= c && c <= '9' ;
}

types_chr chr_type (char c) {
    if ('a' <= c && c <= 'z') return KEYWORD ;
    if ('0' <= c && c <= 'Z') return NUMERIC ;
    switch (c) {
        case '?': case '#': case '/': case '.': case '~':
        case '!':
            return SYMBOL ;
        case 'L': case 'R': case 'U': case 'D': case 'V':
        case 'H': case 'A':
            return SELECTOR ;
        case '+': case '-': case '<': case '>': case '^':
        case '_':
            return INDICATOR ;
        case '\'':
            return MODIFIER ;
        case ' ':
            return BLANK ;
        default:
            return UNKNOWN ;
    }
}


/* Read name at given location
 */
std::string str_parse (int begin, int len) {
    if (len-1 > str_maxlen) {
        char ans = log_warn(LONGNAME, command.substr(begin+1, 5) + "... truncate ? (y/n)") ;
        if (ans == 'n') {
            return "" ;
        } else {
            len = str_maxlen + 1 ;
        }
    }
    return filename_sanitize(command.substr(begin+1, len-1)) ;
}

/* Read integer at given location
 */
int int_parse (int begin, int len) {
    int n = 0 ;
    if (len > num_maxlen) {
        char ans = log_warn(LONGQUANT, command.substr(begin, 5) + "... truncate ? (y/n)") ;
        if (ans == 'n') {
            return -1 ;
        } else {
            len = num_maxlen ;
        }
    }
    for (int i = 0; i < len; i++) {
        if ('0' <= command[begin+i] && command[begin+i] <= '9') {
            n = n*10 + command[begin+i] - '0' ;
        } else {
            return -1 ;
        }
    }
    return n ;
}
