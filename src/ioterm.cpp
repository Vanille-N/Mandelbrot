#include "ioterm.h"

/* All of these are low-level interactions with the terminal.
 * Although the project relies heavily on ANSI escape codes for color output,
 * I guarantee that \033[ appears nowhere outside of this file.
 * (except one in main)
 */

std::string cursor (int i, int j) {
    std::ostringstream str ;
    str
        << "\033["
        << i
        << ";"
        << j
        << "H" ;
    return str.str() ;
}

std::string repeat (int n, std::string s) {
    std::ostringstream str ;
    for (int i = 0; i < n; i++) {
        str << s ;
    }
    return str.str() ;
}

void refresh () {
    std::cout
        << PLAIN
        << cursor(1, 1)
        << '\n' ;
}

void log_clear () {
    std::cout << PLAIN ;
    for (int i = 0; i < view_hgt; i++) {
        std::cout
            << cursor(log_vpos+i, log_hpos)
            << std::string(80, ' ') ;
    }
}

void prompt_clear () {
    std::cout << PLAIN ;
    for (int i = 1; i < 5; i++) {
        std::cout
            << cursor(i, 1)
            << std::string(view_wth, ' ') ;
    }
}

void view_clear () {
    std::cout
        << PLAIN
        << BLACK
        << cursor(view_vpos-1, view_hpos-2)
        << ULCORNER
        << repeat(view_wth+2, HBOX)
        << "╗" ;
    for (int i = 0; i <= view_hgt/2; i++) {
        std::cout
            << cursor(view_vpos+i, view_hpos-2)
            << VBOX
            << std::string(view_wth+2, ' ')
            << VBOX ;
    }
    std::cout
        << cursor(view_vpos+view_hgt/2, view_hpos-2)
        << DLCORNER
        << repeat(view_wth+2, HBOX)
        << DRCORNER
        << PLAIN ;
}

void screen_clear () {
    std::system("clear") ;
}

void prompt_make () {
    prompt_clear() ;
    std::cout
        << cursor(2, 5)
        << "cmd"
        << BLINK
        << "> "
        << PLAIN
        << cursor(3, 7)
        << "Currently inside scope "
        << BLUE
        << BOLD
        << kw[curr_scope]
        << PLAIN ;
    refresh() ;
    std::cout
        << cursor(2, 10)
        << YELLOW
        << BOLD ;
}

void log_draw_rect () {
    std::cout
        << PLAIN
        << BLACK
        << cursor(log_vpos, log_hpos-2)
        << ULCORNER
        << repeat(80, HBOX)
        << URCORNER ;
    for (int i = 0; i <= log_hgt; i++) {
        std::cout
            << cursor(log_vpos+i+1, log_hpos-2)
            << VBOX
            << cursor(log_vpos+i+1, log_hpos+79)
            << VBOX;
    }
    std::cout
        << cursor(log_vpos+log_hgt+1, log_hpos-2)
        << DLCORNER
        << repeat(80, HBOX)
        << DRCORNER
        << PLAIN ;
}

void log_redraw () {
    int L = log_vpos, C = log_hpos ;
    log_clear() ;
    std::cout << PLAIN ;
    for (int i = (int)log_hist.size()-1; i >= 0; i--) {
        std::cout
            << cursor(L-i+(int)log_hist.size(), C)
            << log_hist[i] ;
    }
    log_draw_rect() ;
    prompt_make() ;
}

void view_display () {
    for (int i = 0; i < view_hgt; i++) {
        for (int j = 0; j < view_wth; j++) {
            diverge_min = std::min(diverge_min, preview[i*view_wth + j]) ;
        }
    }
    for (int i = 0; i < view_hgt/2; i++) {
        std::cout << cursor(view_vpos+i, view_hpos) ;
        for (int j = 0; j < view_wth; j++) {
            rgb bg = view_colorspread(preview[2*i*view_wth + j]) ;
            rgb fg = view_colorspread(preview[(2*i+1)*view_wth + j]) ;
            printf("\033[48;2;%d;%d;%dm\033[38;2;%d;%d;%dm▄", bg.r, bg.g, bg.b, fg.r, fg.g, fg.b) ;
        }
    }
    std::cout << PLAIN ;
}

void rgb::display () {
    printf("\033[38;2;%d;%d;%dm█", r, g, b) ;
}
