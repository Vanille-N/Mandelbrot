#include "disp.h"

void log_err (msg_log e, std::string s) {
    hist_shift() ;
    std::string n = msg_header(e) + s ;
    if (log_hist.size() <= log_hgt) {
        log_hist.push_back(n) ;
    } else {
        log_hist[log_hist.size()-1] = n ;
    }
    log_redraw() ;
}

char log_warn (msg_log w, std::string s) {
    hist_shift() ;
    std::string n = msg_header(w) + s ;
    if (log_hist.size() <= log_hgt) {
        log_hist.push_back(n) ;
    } else {
        log_hist[log_hist.size()-1] = n ;
    }
    log_redraw() ;
    char ans ;
    do {
        prompt_clear() ;
        prompt_make() ;
        scanf("%c", &ans) ;
    } while (ans != 'y' && ans != 'n') ;
    return ans ;
}

void log_info (msg_log i, std::string s) {
    hist_shift() ;
    std::string n = msg_header(i) + s ;
    if (log_hist.size() <= log_hgt) {
        log_hist.push_back(n) ;
    } else {
        log_hist[log_hist.size()-1] = n ;
    }
    log_redraw() ;
}

void ls_save_read () {
    ls_text.clear() ;
    system("ls -a .*.save 1>.tmp 2>/dev/null") ;
    std::ifstream x (".tmp") ;
    std::string line ;
    while (getline(x, line)) {
        ls_text.push_back(line) ;
    }
    system("rm .tmp") ;
}

void ls_nil_read () {
    ls_text.clear() ;
    system("ls -a .*.meta 1>.tmp 2>/dev/null") ;
    std::ifstream x (".tmp") ;
    std::string line ;
    while (getline(x, line)) {
        ls_text.push_back(line) ;
    }
    system("rm .tmp") ;
}

void ls_make_read () {
    ls_text.clear() ;
    system("ls *.ppm 1>.tmp 2>/dev/null") ;
    std::ifstream x (".tmp") ;
    std::string line ;
    while (getline(x, line)) {
        ls_text.push_back(line) ;
    }
    system("rm .tmp") ;
}

void ls_map_read () {
    ls_colors.clear() ;
    std::ifstream x (".cmaps.txt") ;
    int r, g, b ;
    int N ;
    x >> N ;
    for (int n = 0; n < N; n++) {
        std::vector<rgb> m ;
        for (int i = 0; i < colors_nb; i++) {
            x >> r ;
            x >> g ;
            x >> b ;
            m.push_back(rgb{r, g, b}) ;
        }
        ls_colors.push_back(m) ;
    }
}


void ls_nil_print () {
    ls_scope = NIL ;
    view_clear() ;
    ls_nil_read() ;
    int id = curr_lspage * entry_nb ;
    std::cout
        << PLAIN
        << cursor(view_vpos+2, view_hpos+2)
        << "Showing page "
        << YELLOW
        << BOLD
        << curr_lspage
        << PLAIN
        << " for "
        << BLUE
        << BOLD
        << "nil" ;
    int i ;
    for (i = 0; i < std::min((int)ls_text.size()-id, entry_nb); i++) {
        std::cout
            << cursor(view_vpos+4+i, view_hpos+2)
            << PLAIN
            << BOLD
            << id+i
            << ": "
            << YELLOW
            << ls_text[id+i] ;
    }
    if (i == 0) {
        std::cout
            << cursor(view_vpos+4, view_hpos+2)
            << PLAIN
            << RED
            << BOLD
            << "Empty" ;
    }
    std::cout << PLAIN ;
}

void ls_save_print () {
    ls_scope = SAVE ;
    view_clear() ;
    ls_save_read() ;
    int id = curr_lspage * entry_nb ;
    std::cout
        << PLAIN
        << cursor(view_vpos+2, view_hpos+2)
        << "Showing page "
        << YELLOW
        << BOLD
        << curr_lspage
        << PLAIN
        << " for "
        << BLUE
        << BOLD
        << "save" ;
    int i ;
    for (i = 0; i < std::min((int)ls_text.size()-id, entry_nb); i++) {
        std::cout
            << cursor(view_vpos+4+i, view_hpos+2)
            << PLAIN
            << BOLD
            << id+i
            << ": "
            << YELLOW
            << ls_text[id+i] ;
    }
    if (i == 0) {
        std::cout
            << cursor(view_vpos+4, view_hpos+2)
            << PLAIN
            << RED
            << BOLD
            << "Empty" ;
    }
    std::cout << PLAIN ;
}

void ls_rec_print () {
    ls_scope = REC ;
    view_clear() ;
    focus_adjust() ;
    std::cout
        << PLAIN
        << cursor(view_vpos+2, view_hpos+2)
        << "Current settings for "
        << BLUE
        << BOLD
        << "rec"
        << PLAIN
        << cursor(view_vpos+4, view_hpos+2)
        << "Size (in pixels): horizontal "
        << GREEN
        << BOLD
        << (int)std::ceil(pic_hresol)
        << PLAIN
        << cursor(view_vpos+5, view_hpos+2)
        << "                    vertical "
        << GREEN
        << BOLD
        << (int)std::ceil(pic_vresol)
        << PLAIN
        << cursor(view_vpos+6, view_hpos+2)
        << "Showing complex plane"
        << cursor(view_vpos+7, view_hpos+2)
        << "    from "
        << RED
        << BOLD
        << view_lt
        << (view_lo >= 0. ? "+" : "")
        << view_lo
        << "i"
        << PLAIN
        << cursor(view_vpos+8, view_hpos+2)
        << "      to "
        << RED
        << BOLD
        << view_rt
        << (view_hi >= 0. ? "+" : "")
        << view_hi
        << "i"
        << PLAIN ;
}

void ls_make_print () {
    ls_scope = MAKE ;
    view_clear() ;
    ls_make_read() ;
    int id = curr_lspage * entry_nb ;
    std::cout
        << PLAIN
        << cursor(view_vpos+2, view_hpos+2)
        << "Showing page "
        << YELLOW
        << BOLD
        << curr_lspage
        << PLAIN
        << " for "
        << BLUE
        << BOLD
        << "make" ;
    int i ;
    for (i = 0; i < std::min((int)ls_text.size()-id, entry_nb); i++) {
        std::cout
            << cursor(view_vpos+4+i, view_hpos+2)
            << PLAIN
            << BOLD
            << id+i
            << ": "
            << YELLOW
            << ls_text[id+i] ;
    }
    if (i == 0) {
        std::cout
            << cursor(view_vpos+4, view_hpos+2)
            << PLAIN
            << RED
            << BOLD
            << "Empty" ;
    }
    std::cout << PLAIN ;
    focus_adjust() ;
    std::cout
        << cursor(view_vpos+entry_nb+7, view_hpos+2)
        << "Resolution: horizontal "
        << GREEN
        << BOLD
        << (int)std::ceil(pic_hresol)
        << PLAIN
        << cursor(view_vpos+entry_nb+8, view_hpos+2)
        << "              vertical "
        << GREEN
        << BOLD
        << (int)std::ceil(pic_vresol)
        << PLAIN
        << cursor(view_vpos+entry_nb+10, view_hpos+2)
        << "Divergence: radius "
        << YELLOW
        << BOLD
        << (int)std::ceil(diverge_radius)
        << PLAIN
        << cursor(view_vpos+entry_nb+11, view_hpos+2)
        << "              iter "
        << YELLOW
        << BOLD
        << diverge_iter
        << PLAIN ;
}

void ls_map_print () {
    ls_scope = MAP ;
    view_clear() ;
    ls_map_read() ;
    int id = curr_lspage * entry_nb ;
    std::cout
        << PLAIN
        << cursor(view_vpos+2, view_hpos+2)
        << "Showing page "
        << YELLOW
        << BOLD
        << curr_lspage
        << PLAIN
        << " for "
        << BLUE
        << BOLD
        << "map" ;
    int i ;
    for (i = 0; i < std::min((int)ls_colors.size()-id, entry_nb); i++) {
        std::cout
            << cursor(view_vpos+4+i, view_hpos+2)
            << PLAIN
            << BOLD
            << id+i
            << ": "
            << cursor(view_vpos+4+i, view_hpos+7) ;
        for (int j = 0; (long unsigned)j < ls_colors[id+i].size(); j++) {
            auto col = ls_colors[id+i][j] ;
            printf("\033[38;2;%d;%d;%dm█", col.r, col.g, col.b) ;
        }
    }
    if (i == 0) {
        std::cout
            << cursor(view_vpos+4, view_hpos+2)
            << PLAIN
            << RED
            << BOLD
            << "Empty" ;
    }
    std::cout
        << cursor(view_vpos+entry_nb+7, view_hpos+2)
        << PLAIN
        << "Currently selected:"
        << cursor(view_vpos+entry_nb+8, view_hpos+2) ;
    for (int j = 0; (long unsigned)j < curr_map.size(); j++) {
        auto col = curr_map[j] ;
        printf("\033[38;2;%d;%d;%dm█", col.r, col.g, col.b) ;
    }
}

void help_print (std::string indic, std::string term) {
    view_clear() ;
    std::ifstream helpf (".help.txt") ;
    std::string line ;
    bool printing = false ;
    int cnt = 0 ;
    while (getline(helpf, line)) {
        if (line == indic) {
            printing = true ;
        } else if (printing && line == term) {
            return ;
        } else if (printing) {
            std::cout << cursor(view_vpos+cnt+2, view_hpos+2) ;
            bool color = false ;
            for (int i = 0; (long unsigned)i < line.length(); i++) {
                if (line[i] == '$' || line[i] == '&') {
                    if (line[i] == '&') {
                        color = false ;
                        std::cout << PLAIN ;
                    }
                    if (!color) {
                        switch (line[++i]) {
                            case 'g': std::cout << GREEN ; break ;
                            case 'r': std::cout << RED ; break ;
                            case 'y': std::cout << YELLOW ; break ;
                            case 'b': std::cout << BLUE ; break ;
                            case 'k': std::cout << BLACK ; break ;
                            case 'p': std::cout << PURPLE ; break ;
                            case 'e': std::cout << GREY ; break ;
                            case 'G': std::cout << GREEN << BOLD ; break ;
                            case 'R': std::cout << RED << BOLD ; break ;
                            case 'Y': std::cout << YELLOW << BOLD ; break ;
                            case 'B': std::cout << BLUE << BOLD ; break ;
                            case 'K': std::cout << BLACK << BOLD ; break ;
                            case 'P': std::cout << PURPLE << BOLD ; break ;
                            case 'E': std::cout << GREY << BOLD ; break ;
                        }
                        color = true ;
                    } else {
                        color = false ;
                        std::cout << PLAIN ;
                    }
                } else {
                    std::cout << line[i] ;
                }
            }
            std::cout << PLAIN ;
            cnt++ ;
        }
    }
}

void scope_help_print () {
    help_print("<SCOPE>", "<END>") ;
}

void map_help_print () {
    help_print("<MAP>", "<END>") ;
}

void save_help_print () {
    help_print("<SAVE>", "<END>") ;
}

void rec_help_print () {
    help_print("<REC>", "<END>") ;
}

void make_help_print () {
    help_print("<MAKE>", "<END>") ;
}

void nil_help_print () {
    help_print("<NIL>", "<END>") ;
}

std::string msg_header(msg_log m) {
    std::ostringstream str ;
    switch(m) {
        case UNKCHR: case NOSELEC: case NOINDIC: case NOFILE: case PARSE:
        case EXCEPTION: case NOSUCHKW:
            str
                << RED
                << BOLD
                << BLINK
                << "ERROR:"
                << PLAIN ;
            break ;
        case RESELEC: case RENAME: case REINDIC: case LONGQUANT:
        case LONGCMD: case LONGNAME: case FEXISTS: case QUIT: case NPORTABLE:
            str
                << YELLOW
                << BOLD
                << UNDERLINE
                << "WARNING:"
                << PLAIN ;
            break ;
        case DONE:
            str
                << GREEN
                << BOLD
                << "SUCCESS"
                << PLAIN ;
            break ;
        case DEFQUANT: case NEWSCOPE: case LOADED: case SAVED:
        case NEWMAP: case BUILT: case EMPTY: case SIGLS:
        case SIGRESET: case SIGHELP: case NEWFOCUS: case FLIP:
            str
                << BLUE
                << BOLD
                << "INFO:"
                << PLAIN ;
            break ;
        default:
            return "" ;
    }
    switch(m) {
        case UNKCHR:    str <<  " Unknown character                " ; break ;
        case NOSELEC:   str <<  " No selector specified            " ; break ;
        case NOINDIC:   str <<  " No indicator specified           " ; break ;
        case NOFILE:    str <<  " No such file                     " ; break ;
        case PARSE:     str <<  " Critical parsing error           " ; break ;
        case EXCEPTION: str <<  " Runtime exception                " ; break ;
        case NOSUCHKW:  str <<  " Not a valid keyword              " ; break ;
        case RESELEC:   str <<  " Too many selectors specified   " ; break ;
        case RENAME:    str <<  " Name already specified         " ; break ;
        case REINDIC:   str <<  " Too many indicators specified  " ; break ;
        case LONGQUANT: str <<  " Quantifier too long            " ; break ;
        case LONGCMD:   str <<  " Command too long               " ; break ;
        case LONGNAME:  str <<  " String literal too long        " ; break ;
        case FEXISTS:   str <<  " File already exists            " ; break ;
        case NPORTABLE: str <<  " Not a portable filename        " ; break ;
        case QUIT:      str <<  " Quit ?                         " ; break ;
        case DONE:      str <<  "                                 " ; break ;
        case DEFQUANT:  str <<  " Used default quantifier           " ; break ;
        case NEWSCOPE:  str <<  " Changed scope                     " ; break ;
        case LOADED:    str <<  " Loaded save file                  " ; break ;
        case SAVED:     str <<  " Current settings saved            " ; break ;
        case NEWMAP:    str <<  " Changed color map                 " ; break ;
        case BUILT:     str <<  " Done building image               " ; break ;
        case EMPTY:     str <<  " Blank expression                  " ; break ;
        case SIGLS:     str <<  " Asked for listing                 " ; break ;
        case SIGRESET:  str <<  " Asked for reset                   " ; break ;
        case SIGHELP:   str <<  " Asked for help                    " ; break ;
        case NEWFOCUS:  str <<  " Adjusting focus                   " ; break ;
        case FLIP:      str <<  " Focus was flipped                 " ; break ;
    }
    return str.str() ;
}

void hist_shift () {
    if (log_hist.size() <= log_hgt) return ;
    for (int i = 1; (long unsigned)i < log_hist.size(); i++) {
        log_hist[i-1] = log_hist[i] ;
    }
}
