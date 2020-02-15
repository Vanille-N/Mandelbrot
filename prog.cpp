#include <iostream>
#include <vector>
#include <string>
#include <unordered_map>
#include <complex>
#include <fstream>


/* Assoc class provides an easy two-way correspondance between user keywords
 * and internal cmd representation.
 */
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

struct rgb {
    int R ;
    int G ;
    int B ;
};

enum cmd {SCOPE=-1000, NIL, REC, MAP, MAKE, LOAD, SAVE, LS, HELP, RESET, HASH, NEXT, LSIDE, RSIDE, USIDE, DSIDE, HSIDE, VSIDE,
    ZOOMIN, ZOOMOUT, LSHIFT, RSHIFT, USHIFT, DSHIFT, NUM, STR, EXIT, ABORT} ;

enum msg_log {UNKCHR, NOSELEC, NOINDIC, NOFILE, PARSE, NOSUCHKW,
    RESELEC, REINDIC, LONGQUANT, LONGCMD, LONGNAME, FEXISTS, QUIT, RENAME,
    DEFQUANT, DONE, NEWSCOPE, LOADED, SAVED, NEWMAP, BUILT} ;

static Assoc kw ;

static std::string curr_name = "NULL" ;
static std::string msg_info = "" ;

static const int num_maxlen = 5 ;
static const int str_maxlen = 10 ;
static const int cmd_maxlen = 50 ;
static const int view_hgt = 25 ;
static const int view_wth = 25 ;
static const int logI = 5 ;
static const int logJ = 50 ;
static const int viewI = 5 ;
static const int viewJ = 5 ;
static const int entry_nb = 20 ;
static const int colors_nb = 40 ;

static int curr_scope = NIL ;
static int curr_lspage = -1 ;

static std::vector<msg_log> hist_log ;

static int vresol = 1 ;
static int hresol = 1 ;
static double lt = -2 ;
static double rt = 1 ;
static double hi = 1 ;
static double lo = -1 ;

static std::vector<std::string> ls_text ;
static std::vector<std::vector<rgb>> ls_colors ;
static std::vector<cmd> exec ;
static std::vector<int> tokens ;

static std::string command ;

static int * preview ;


// GRAPHIC OUTPUT
void clear_screen () {
    std::system("reset") ;
}

void make_prompt () {
    printf("\033[2;5Hcmd\033[5m> \033[0m") ;
    printf("\033[3;7HCurrently inside scope ") ;
    for (int i = 0; i < kw[curr_scope].length(); i++) {
        putchar(kw[curr_scope][i]) ;
    }
}

void print_msg_header(msg_log m) {
    switch(m) {
        case UNKCHR:
            printf("\033[31;1;5mERROR:\033[0m Unknown character") ;
        case NOSELEC:
            printf("\033[31;1;5mERROR:\033[0m No selector specified") ;
        case NOINDIC:
            printf("\033[31;1;5mERROR:\033[0m No indicator specified") ;
        case NOFILE:
            printf("\033[31;1;5mERROR:\033[0m No such file") ;
        case PARSE:
            printf("\033[31;1;5mERROR:\033[0m Critical parsing error") ;
        case NOSUCHKW:
            printf("\033[31;1;5mERROR:\033[0m Not a valid keywordm") ;
        case RESELEC:
            printf("\033[32;1;4mWARNING:\033[0m Too many selectors specified") ;
        case RENAME:
            printf("\033[32;1;4mWARNING:\033[0m Name already specified") ;
        case REINDIC:
            printf("\033[32;1;4mWARNING:\033[0m Too many indicators specified") ;
        case LONGQUANT:
            printf("\033[32;1;4mWARNING:\033[0m Quantifier too long") ;
        case LONGCMD:
            printf("\033[32;1;4mWARNING:\033[0m Command too long") ;
        case LONGNAME:
            printf("\033[32;1;4mWARNING:\033[0m String literal too long") ;
        case FEXISTS:
            printf("\033[32;1;4mWARNING:\033[0m File already exists") ;
        case QUIT:
            printf("\033[32;1;4mWARNING:\033[0mQuit ?") ;
        case DEFQUANT:
            printf("\033[33;1mINFO:\033[0m Used default quantifier") ;
        case DONE:
            printf("\033[34;1mSUCCESS\033[0m") ;
        case NEWSCOPE:
            printf("\033[33;1mINFO:\033[0m Changed scope") ;
        case LOADED:
            printf("\033[33;1mINFO:\033[0m Loaded save file") ;
        case SAVED:
            printf("\033[33;1mINFO:\033[0m Current settings saved") ;
        case NEWMAP:
            printf("\033[33;1mINFO:\033[0m Changed color map") ;
        case BUILT:
            printf("\033[33;1mINFO:\033[0m Done building image") ;
    }
}

void print_msg_info() {
    for (int i = 0; i < msg_info.size(); i++) {
        putchar(msg_info[i]) ;
    }
}

void log_redraw () {
    int L = logI, C = logJ ;
    printf("\033[%d;%dH", L, C) ;
    print_msg_header(hist_log[0]) ;
    printf("\033[%d;%dH", L+1, C) ;
    print_msg_info() ;
    for (int i = 1; i < hist_log.size(); i++) {
        printf("\033[%d;%dH", L+i+1, C) ;
        print_msg_header(hist_log[i]) ;
    }
}

void shift_hist () {
    if (hist_log.size() <= 10) return ;
    for (int i = 1; i < hist_log.size(); i++) {
        hist_log[i-1] = hist_log[i] ;
    }
}

void log_err (msg_log e) {
    shift_hist() ;
    hist_log[hist_log.size()-1] = e ;
    log_redraw() ;
}

char log_warn (msg_log w) {
    shift_hist() ;
    hist_log[hist_log.size()-1] = w ;
    log_redraw() ;
    char ans ;
    do {
        scanf("%c", &ans) ;
    } while (ans != 'y' && ans != 'n') ;
    return ans ;
}

void log_info (msg_log i) {
    shift_hist() ;
    hist_log[hist_log.size()-1] = i ;
    log_redraw() ;
}

void print_ls_load () {
    read_ls_load() ;
    id = curr_lspage * entry_nb ;
    printf("\033[%d;%dHShowing page %d for LOAD", viewI, viewJ, curr_lspage) ;
    for (int i = 0; i < std::max(ls_text.size(), id + entry_nb); i++) {
        printf("\033[%d;%dH\033[1m%d:\033[0m", viewI+2, viewJ, id+i) ;
        for (int j = 0; j < ls_text[id+i].length(); j++) {
            putchar(ls_colors[id+i][j]) ;     
        }
    }
}

void print_ls_save () {
    read_ls_load() ;
    id = curr_lspage * entry_nb ;
    printf("\033[%d;%dHShowing page %d for SAVE", viewI, viewJ, curr_lspage) ;
    for (int i = 0; i < std::max(ls_text.length(), id + entry_nb); i++) {
        printf("\033[%d;%dH\033[1m%d:\033[0m", viewI+2, viewJ, id+i) ;
        for (int j = 0; j < ls_text[id+i].length()) {
            putchar(ls_text[id+i][j]) ;
        }
    }
}

void print_ls_rec () {
    printf("\033[%d;%dHCurrent settings for REC", viewI, viewJ) ;
    printf("\033[%d;%dHResolution (relative to preview): vertical %d ; horizontal %d", viewI+1, viewJ, vresol, hresol) ;
    printf("\033[%d;%dHSize (in pixels): vertical %d ; horizontal %d", viewI+2, viewJ, vresol*view_hgt, hresol*view_wth) ;
    printf("\033[%d;%dHShowing complex plane from %f+%fi to %f+%fi", viewI+3, viewJ, lt, lo, rt, hi) ;
}

void print_ls_make () {
    read_ls_make() ;
    id = curr_lspage * entry_nb ;
    printf("\033[%d;%dHShowing page %d for MAKE", viewI, viewJ, curr_lspage) ;
    for (int i = 0; i < std::max(ls_text.size(), id + entry_nb); i++) {
        printf("\033[%d;%dH\033[1m%d:\033[0m", viewI+2, viewJ, id+i) ;
        for (int j = 0; j < ls_text[id+i].length(); j++) {
            putchar(ls_text[id+i][j]) ;
        }
    }
}

void print_ls_map () {
    read_ls_map() ;
    int id = curr_lspage * entry_nb ;
    printf("\033[%d;%dHShowing page %d for MAP", viewI, viewJ, curr_lspage) ;
    for (int i = 0; i < std::max((int)ls_text.size(), id + entry_nb); i++) {
        printf("\033[%d;%dH\033[1m%d:\033[0m", viewI+2, viewJ, id+i) ;
        for (int j = 0; j < ls_colors[id+i].size(); j++) {
            auto col = ls_colors[id+i][j] ;
            printf("\033[38;2;%d;%d;%d█", col.R, col.G, col.B) ;        
        }
    }
}

void display_preview () {
    for (int i = 0; i < view_hgt/2; i++) {
        for (int j = 0; j < view_wth; j++) {
            int bg = preview[2*i][j] ? 91 : 90 ;
            int fg = preview[2*i+1][j] ? 101 : 100 ;
            printf("\033[%d;%d▄", bg, fg) ;
        }
    }
}



// TEXT IO

void output_save () {
    std::ofstream sv (curr_name) ;
    sv << hresol << " " << vresol << "\n" ;
    sv << lt << " " << rt << "\n" ;
    sv << lo << " " << hi << "\n" ;
}

void input_save () {
    std::ifstream sv (curr_name) ;
    sv >> hresol ;
    sv >> vresol ;
    sv >> lt ;
    sv >> rt ;
    sv >> lo ;
    sv >> hi ;
}

void read_ls_load () {
    ls_text.clear() ;
    system("ls *.sv 1>.x.txt 2>/dev/null") ;
    std::ifstream x (".x.txt") ;
    std::string line ;
    while (getline(x, line)) {
        ls_text.push_back(line) ;
    }
}

void read_ls_make () {
    ls_text.clear() ;
    system("ls *.ppm 1>.x.txt 2>/dev/null") ;
    std::ifstream x (".x.txt") ;
    std::string line ;
    while (getline(x, line)) {
        ls_text.push_back(line) ;
    }
}

void read_ls_map () {
    ls_colors.clear() ;
    std::ifstream x ("cmaps.txt") ;
    std::string line ;
    int r, g, b ;
    for (;;) {
        try {
            std::vector<rgb> m ;
            for (int i = 0; i < colors_nb; i++) {
                x >> r ;
                x >> g ;
                x >> b ;
                m.push_back(rgb{r, g, b}) ;
            }
            ls_colors.push_back(m) ;
        } catch (...) {
            break ;
        }
    }
}

enum types_chr {KEYWORD, SELECTOR, INDICATOR, MODIFIER, SYMBOL, UNKNOWN, BLANK} ;

int chr_type (char c) {
    switch (c) {
        case 'a': case 'c': case 'e': case 'i': case 'k':
        case 'l': case 'm': case 'n': case 'o': case 'p':
        case 'r': case 's': case 'v':
            return KEYWORD ;
        case '?': case '#': case '/': case '.': case '~':
            return SYMBOL ;
        case 'L': case 'R': case 'U': case 'D': case 'V':
        case 'H':
            return SELECTOR ;
        case '+': case '-': case '<': case '>': case '^':
        case '_':
            return INDICATOR ;
        case '\'': case ':':
            return MODIFIER ;
        case ' ':
            return BLANK ;
        default:
            return UNKNOWN ;
    }
}

/* Cut the string into tokens :
 * find beginning and end of keywords, integers, and strings
 * separate all other symbols
 */
void tokenize () {
    tokens.clear() ;
    int idx = 0, len = 0 ;
    types_chr curr ;
    while (idx+len < command.length()) {
        curr = chr_type(command[idx+len]) ;
        if (curr == KEYWORD) {
            if (idx+len+1 == command.length()) {
                tokens.push_back(idx) ;
                break ;
            } else if (chr_type(command[idx+len+1]) == KEYWORD) {
                len++ ;
            } else {
                tokens.push_back(idx) ;
                idx += len ;
                len = 0 ;
            }
        } else if (curr == SYMBOL || curr == SELECTOR || curr == INDICATOR) {
            tokens.push_back(idx) ;
            idx++ ;
        } else if (curr == MODIFIER) {
            while (idx+len < command.length() && chr_type(command[idx+len]) != BLANK) {
                len++ ;
            }
            tokens.push_back(idx) ;
            idx += len ;
            len = 0 ;
        } else if (curr == BLANK) {
            idx++ ;
        } else if (curr == UNKNOWN) {
            tokens.push_back(-1) ;
        }
    }
    tokens.push_back(command.length()) ;
}

/* Read integer at given location
 */
int parse_int (std::string command, int begin, int end) {
    int n = 0 ;
    // First chr is a ':', ignore it
    if (end-begin-1 > num_maxlen) {
        msg_info = command.substr(begin+1, 5) + "... truncate ? (y/n)" ;
        char ans = log_warn(LONGQUANT) ;
        if (ans == 'n') {
            return -1 ;
        } else {
            end = begin + num_maxlen + 1;
        }
    }
    for (int i = begin+1; i < end; i++) {
        if ('0' <= command[i] && command[i] <= '9') {
            n = n*10 + command[i] - '0' ;
        } else {
            return -1 ;
        }
    }
    return n ;
}

/* Read name at given location
 */
std::string parse_name (int begin, int end) {
    if (end-begin-1 > str_maxlen) {
        msg_info = command.substr(begin+1, 5) + "... truncate ? (y/n)" ;
        char ans = log_warn(LONGNAME) ;
        if (ans == 'n') {
            return "" ;
        } else {
            end = begin + str_maxlen + 1 ;
        }
    }
    return command.substr(begin+1, end-begin-1) ;
}

/* Split command line into usable commands
 */
void parse () {
    tokenize() ;
    exec.clear() ;
    bool nameset = false ;
    if (command.length() > cmd_maxlen) {
        msg_info = command.substr(0, 5) + "... truncate ? (y/n)" ;
        char ans = log_warn(LONGCMD) ;
        if (ans == 'n') {
            exec.push_back(ABORT) ;
            return ;
        } else {
            command = command.substr(0, cmd_maxlen) ;
        }
    }
    for (int i = 0; i < tokens.size(); i++) {
        if (tokens[i] == -1) {
            log_err(UNKCHR) ;
            exec.push_back(ABORT) ;
        } else if (command[tokens[i]] == '\'') {
            auto name = parse_name(tokens[i], tokens[i+1]) ;
            if (name.length() == 0) {
                msg_info = "Process aborted : no name specified" ;
                log_err(PARSE) ;
                exec.push_back(ABORT) ;
            } else if (nameset) {
                msg_info = "Overwrite ? (y/n)" ;
                char ans = log_warn(RENAME) ;
                if (ans == 'n') {
                    continue ;
                } else {
                    curr_name = name ;
                }
            } else {
                exec.push_back(STR) ;
                curr_name = name ;
                nameset = true ;
            }
        } else if (command[tokens[i]] == ':') {
            int n = parse_int(command, tokens[i], tokens[i+1]) ;
            if (n == -1) {
                msg_info = command.substr(tokens[i], 5) + "... not a valid quantifier" ;
                log_err(PARSE) ;
                exec.push_back(ABORT) ;
            } else {
                exec.push_back(NUM) ;
                exec.push_back(n) ;
            }
        } else {
            auto atom = command.substr(tokens[i], tokens[i+1]-tokens[i]) ;
            if (kw.exists(atom)) {
                exec.push_back(kw[atom]) ;
            } else {
                msg_info = atom.substr(0, std::min(5, (int)atom.length())) + (atom.length() > 5 ? "..." : "") ;
                log_err(NOSUCHKW) ;
                exec.push_back(ABORT) ;
            }
        }
    }
    return ;
}


struct adjust {
    bool active ;
    cmd indicator ;
    int quantifier ;
};


int execute () {
    cmd scope_restore = (cmd)curr_scope ;
    int idx = 0 ;
    for (;;) {
        if (idx >= exec.size()) {
            msg_info = "Nothing left to do" ;
            log_info(DONE) ;
            goto end ;
        }
        switch (exec[idx]) {
            case SCOPE:
                if (idx+1 >= exec.size()) {
                    msg_info = "No scope specified" ;
                    log_err(PARSE) ;
                    goto end ;
                }
                switch (exec[idx+1]) {
                    case HELP:
                        print_scope_help() ;
                        msg_info = "Terminate" ;
                        log_info(DONE) ;
                        goto end ;
                    case NIL: case REC: case MAP: case MAKE: case LOAD: case SAVE:
                        msg_info = "Switching to " + kw[exec[idx+1]] ;
                        log_info(NEWSCOPE) ;
                        curr_scope = exec[idx+1] ;
                        msg_info = "Terminate" ;
                        log_info(DONE) ;
                        return 1 ;
                    case RESET:
                        msg_info = "Reset scope : NIL" ;
                        log_info(NEWSCOPE) ;
                        curr_scope = NIL ;
                        msg_info = "Terminate" ;
                        log_info(DONE) ;
                        return 1 ;
                    default:
                        msg_info = kw[exec[idx+1]]  + " not expected after keyword scope" ;
                        log_err(PARSE) ;
                        goto end ;
                }
            case NIL: case REC: case MAP: case MAKE: case LOAD: case SAVE:
                msg_info = "Temporary scope change" ;
                log_info(NEWSCOPE) ;
                curr_scope = NIL ;
                idx += 1 ;
            case LS:
                switch (curr_scope) {
                    case MAP:
                        if (idx+1 < exec.size() && exec[idx+1]==NUM) {
                            curr_lspage = exec[idx+2] ;
                        } else {
                            curr_lspage = 0 ;
                        }
                        print_ls_map() ;
                        msg_info = "Terminate" ;
                        log_info(DONE) ;
                        goto keepls ;
                    case LOAD:
                        if (idx+1 < exec.size() && exec[idx+1]==NUM) {
                            curr_lspage = exec[idx+2] ;
                        } else {
                            curr_lspage = 0 ;
                        }
                        print_ls_load() ;
                        msg_info = "Terminate" ;
                        log_info(DONE) ;
                        goto keepls ;
                    case SAVE:
                        if (idx+1 < exec.size() && exec[idx+1]==NUM) {
                            curr_lspage = exec[idx+2] ;
                        } else {
                            curr_lspage = 0 ;
                        }
                        print_ls_save() ;
                        msg_info = "Terminate" ;
                        goto keepls ;
                    case MAKE:
                        if (idx+1 < exec.size() && exec[idx+1]==NUM) {
                            curr_lspage = exec[idx+2] ;
                        } else {
                            curr_lspage = 0 ;
                        }
                        print_ls_make() ;
                        msg_info = "Terminate" ;
                        log_info(DONE) ;
                        goto keepls ;
                    case REC:
                        if (idx+1 < exec.size() && exec[idx+1]==NUM) {
                            curr_lspage = exec[idx+2] ;
                        } else {
                            curr_lspage = 0 ;
                        }
                        print_ls_rec() ;
                        msg_info = "Terminate" ;
                        log_info(DONE) ;
                        goto keepls ;
                    default:
                        msg_info = "LS not expected in scope" + curr_scope ;
                        log_err(PARSE) ;
                        goto end ;
                }
            case HELP:
                switch (curr_scope) {
                    case MAP:
                        print_map_help() ; break ;
                    case SAVE:
                        print_save_help() ; break ;
                    case LOAD:
                        print_load_help() ; break ;
                    case REC:
                        print_rec_help() ; break ;
                    case MAKE:
                        print_make_help() ; break ;
                    case NIL:
                        print_nil_help() ; break ;
                    }
                msg_info = "Terminate" ;
                log_info(DONE) ;
                goto end ;
            case RESET:
                switch (curr_scope) {
                    case MAP:
                        reset_map() ; break ;
                    case REC:
                        reset_rec() ; break ;
                    case MAKE:
                        reset_make() ; break ;
                    case NIL:
                        reset_nil() ; break ;
                    default:
                        msg_info = "No reset for scope " + kw[curr_scope] ;
                        log_err(PARSE) ;
                        goto end ;
                }
                msg_info = "Terminate" ;
                log_info(DONE) ;
                goto end ;
            case HASH:
                switch (curr_scope) {
                    case SAVE:
                        hash_name() ;
                        save_settings() ;
                        goto end ;
                    case MAKE:
                        hash_name() ;
                        make_image() ;
                        goto end ;
                    default:
                        msg_info = kw[HASH] + " not expected in scope" + kw[curr_scope] ;
                        log_err(PARSE) ;
                        goto end ;
                }
            case NEXT:
                if (curr_lspage == -1) {
                    msg_info = "No ls page displayed" ;
                    log_err(PARSE) ;
                    goto end ;
                }
                curr_lspage++ ;
                switch (curr_scope) {
                    case MAP:
                        print_ls_map() ;
                    case LOAD:
                        print_ls_load() ;
                    case SAVE:
                        print_ls_save() ;
                    case MAKE:
                        print_ls_make() ;
                    case REC:
                        print_ls_rec() ;
                }
                msg_info = "Terminate" ;
                log_info(DONE) ;
                goto keepls ;
            case LSIDE: case RSIDE: case USIDE: case DSIDE: case HSIDE:
            case VSIDE: case ZOOMIN: case ZOOMOUT: case LSHIFT: case RSHIFT: case USHIFT: case DSHIFT:
                {
                if (curr_scope != REC) {
                    msg_info = "View specifications not expected here" ;
                    log_err(PARSE) ;
                    goto end ;
                }
                adjust chL {false, (cmd)-1, (cmd)-1} ;
                adjust chR = chL, chU = chL, chD = chL ;
                for (int i = idx; i < exec.size(); i++) {
                    switch (exec[i]) {
                        case RSIDE:
                            if (chR.active) {
                                msg_info = "Right side is already selected. Overwrite ? (y/n)" ;
                                char ans = log_warn(RESELEC) ;
                                if (ans == 'y') {
                                    chR.active = true ;
                                }
                                break ;
                            }
                            chR.active = true ; break ;
                        case LSIDE:
                            if (chL.active) {
                                msg_info = "Left side is already selected. Overwrite ? (y/n)" ;
                                char ans = log_warn(RESELEC) ;
                                if (ans == 'y') {
                                    chL.active = true ;
                                }
                                break ;
                            }
                            chL.active = true ; break ;
                        case USIDE:
                            if (chU.active) {
                                msg_info = "Up side is already selected. Overwrite ? (y/n)" ;
                                char ans = log_warn(RESELEC) ;
                                if (ans == 'y') {
                                    chU.active = true ;
                                }
                                break ;
                            }
                            chU.selected = true ; break ;
                        case DSIDE:
                            if (chD.active) {
                                msg_info = "Right side is already selected. Overwrite ? (y/n)" ;
                                char ans = log_warn(RESELEC) ;
                                if (ans == 'y') {
                                    chD.active = true ;
                                }
                                break ;
                            }
                            chD.selected = true ; break ;
                        case HSIDE:
                            if (chR.active) {
                                msg_info = "Right side is already selected. Overwrite ? (y/n)" ;
                                char ans = log_warn(RESELEC) ;
                                if (ans == 'y') {
                                    chR.active = true ;
                                }
                                break ;
                            }
                            chR.active = true ;
                            if (chL.active) {
                                msg_info = "Left side is already selected. Overwrite ? (y/n)" ;
                                char ans = log_warn(RESELEC) ;
                                if (ans == 'y') {
                                    chL.active = true ;
                                }
                                break ;
                            }
                            chL.active = true ; break ;
                        case VSIDE:
                            if (chU.active) {
                                msg_info = "Up side is already selected. Overwrite ? (y/n)" ;
                                char ans = log_warn(RESELEC) ;
                                if (ans == 'y') {
                                    chU.active = true ;
                                }
                                break ;
                            }
                            chU.active = true ;
                            if (chD.active) {
                                msg_info = "Right side is already selected. Overwrite ? (y/n)" ;
                                char ans = log_warn(RESELEC) ;
                                if (ans == 'y') {
                                    chD.active = true ;
                                }
                                break ;
                            }
                            chD.selected = true ; break ;
                        case ZOOMIN:
                            if (chL.active && chL.indicator == -1) chL.indicator = ZOOMIN ;
                            if (chR.active && chR.indicator == -1) chR.indicator = ZOOMIN ;
                            if (chU.active && chU.indicator == -1) chU.indicator = ZOOMIN ;
                            if (chD.active && chD.indicator == -1) chD.indicator = ZOOMIN ;
                        case ZOOMOUT:
                            if (chL.active && chL.indicator == -1) chL.indicator = ZOOMOUT ;
                            if (chR.active && chR.indicator == -1) chR.indicator = ZOOMOUT ;
                            if (chU.active && chU.indicator == -1) chU.indicator = ZOOMOUT ;
                            if (chD.active && chD.indicator == -1) chD.indicator = ZOOMOUT ;
                        case LSHIFT:
                            if (chL.active && chL.indicator == -1) chL.indicator = ZOOMOUT ;
                            if (chR.active && chR.indicator == -1) chR.indicator = ZOOMIN ;
                        case RSHIFT:
                            if (chL.active && chL.indicator == -1) chL.indicator = ZOOMIN ;
                            if (chR.active && chR.indicator == -1) chR.indicator = ZOOMOUT ;
                        case USHIFT:
                            if (chU.active && chU.indicator == -1) chU.indicator = ZOOMOUT ;
                            if (chD.active && chD.indicator == -1) chD.indicator = ZOOMIN ;
                        case DSHIFT:
                            if (chU.active && chU.indicator == -1) chU.indicator = ZOOMIN ;
                            if (chD.active && chD.indicator == -1) chD.indicator = ZOOMOUT ;
                        case NUM:
                            i++ ;
                            if (chL.active && chL.quantifier == -1) chL.quantifier = exec[i] ;
                            if (chR.active && chR.quantifier == -1) chR.quantifier = exec[i] ;
                            if (chU.active && chU.quantifier == -1) chU.quantifier = exec[i] ;
                            if (chD.active && chD.quantifier == -1) chD.quantifier = exec[i] ;
                        default:
                            goto act ;
                    }
                }
                act:
                if (chL.active) change_focus(LSIDE, chL.indicator, chL.quantifier) ;
                if (chR.active) change_focus(RSIDE, chR.indicator, chR.quantifier) ;
                if (chU.active) change_focus(USIDE, chU.indicator, chU.quantifier) ;
                if (chD.active) change_focus(DSIDE, chD.indicator, chD.quantifier) ;
                preview_redraw() ;
                msg_info = "Terminate" ;
                log_info(DONE) ;
                goto end ;
                }
            case NUM:
                switch (curr_scope) {
                    case MAP:
                        set_map(exec[idx+1]) ;
                        msg_info = "Terminate" ;
                        log_info(DONE) ;
                        idx += 2 ;
                        goto end ;
                    case MAKE:
                        set_resolution(exec[idx+1]) ;
                        idx += 2 ;
                        continue ;
                    case LOAD:
                        load_settings(exec[idx+1]) ;
                        msg_info = "Terminate" ;
                        log_info(DONE) ;
                        goto end ;
                    default:
                        msg_info = "Quantifier not expected here" ;
                        log_err(PARSE) ;
                        goto end ;
                }
            case STR:
                switch (curr_scope) {
                    case SAVE:
                        load_settings() ;
                        msg_info = "Terminate" ;
                        log_info(DONE) ;
                        goto end ;
                    case MAKE:
                        make_image() ;
                        msg_info = "Terminate" ;
                        log_info(DONE) ;
                        goto end ;
                    default:
                        msg_info = "String literal not expected here" ;
                        log_err(PARSE) ;
                        goto end ;
                }
                goto end ;
            case EXIT:
                return 0 ;
            case ABORT:   
                goto end ;  
        }
    }
    end:
    curr_lspage = -1 ;
    keepls:
    curr_scope = scope_restore ;
    return 1 ;
}




int main () {
    // Build dictionnary
    kw.link(SCOPE, "scope") ;
    kw.link(NIL, "nil") ;
    kw.link(REC, "rec") ;
    kw.link(MAP, "map") ;
    kw.link(MAKE, "make") ;
    kw.link(LOAD, "load") ;
    kw.link(SAVE, "save") ;
    kw.link(LS, "ls") ;
    kw.link(HELP, "?") ;
    kw.link(RESET, ".") ;
    kw.link(HASH, "#") ;
    kw.link(NEXT, "/") ;
    kw.link(LSIDE, "L") ;
    kw.link(RSIDE, "R") ;
    kw.link(USIDE, "U") ;
    kw.link(DSIDE, "D") ;
    kw.link(HSIDE, "H") ;
    kw.link(VSIDE, "V") ;
    kw.link(ZOOMIN, "+") ;
    kw.link(ZOOMOUT, "-") ;
    kw.link(LSHIFT, "<") ;
    kw.link(RSHIFT, ">") ;
    kw.link(USHIFT, "^") ;
    kw.link(DSHIFT, "_") ;
    kw.link(NUM, ":") ;
    kw.link(STR, "'") ;
    kw.link(EXIT, "~") ;
}

