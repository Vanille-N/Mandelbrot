#include <iostream>
#include <vector>
#include <string>
#include <unordered_map>
#include <complex>
#include <fstream>
#include <stdlib.h>
#include <time.h>
#include <cstdarg>

#include "ioterm.h"
#include "exec.h"
#include "parse.h"
#include "glob.h"
#include "disp.h"
#include "calc.h"

int read_int (const char *);
void assist_resize () ;
bool streq (const char *, const char *) ;

int main (int argc, char * argv []) {
    // Build dictionnary
    kw.link(SCOPE, "scope") ;
    kw.link(NIL, "nil") ;
    kw.link(REC, "rec") ;
    kw.link(MAP, "map") ;
    kw.link(MAKE, "make") ;
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
    kw.link(ASIDE, "A") ;
    kw.link(ZOOMIN, "+") ;
    kw.link(ZOOMOUT, "-") ;
    kw.link(LSHIFT, "<") ;
    kw.link(RSHIFT, ">") ;
    kw.link(USHIFT, "^") ;
    kw.link(DSHIFT, "_") ;
    kw.link(NUM, ":") ;
    kw.link(STR, "'") ;
    kw.link(EXIT, "~") ;
    kw.link(REDRAW, "!") ;

    bool s = false ;
    for (int i = 1; i < argc; i++) {
        if (streq(argv[i], "-s")) {
            s = true ;
        }
    }
    if (!s) {
        assist_resize() ;
    }

    screen_clear() ;

    focus_adjust() ;
    preview_redraw() ;

    ls_map_read() ;
    map_choose(0) ;
    log_draw_rect() ;

    do {
        prompt_make() ;
        if (log_hist.size() > 0) {
            log_redraw() ;
        }
        getline(std::cin, command) ;
        parse() ;
    } while (execute()) ;

    screen_clear() ;
    return 0 ;
}

int read_int (const char * cmd) {
    char buf [4] ;
    FILE * fp = popen(cmd, "r") ;
    fgets(buf, 4, fp) ;
    int ans = 0 ;
    for (int i = 0; i < 4; i++) {
        if ('0' <= buf[i] && buf[i] <= '9') {
            ans = ans * 10 +  buf[i] - '0' ;
        } else {
            break ;
        }
    }
    pclose(fp) ;
    return ans ;
}

void assist_resize () {
    int require_hgt = view_hgt / 2 + 8 ;
    int require_wth = view_wth + 95 ;
    int curr_hgt ;
    int curr_wth ;
    printf("\n\n") ;
    do {
        curr_hgt = read_int("tput lines") ;
        curr_wth = read_int("tput cols") ;
        printf("\033[3A\n") ;
        printf("Current size : (%d, %d)\n", curr_hgt, curr_wth) ;
        printf("Target       : (%d, %d)\n", require_hgt, require_wth) ;
        printf("^C to abort, run again with option -s to ignore this warning") ;
    } while(require_hgt > curr_hgt || require_wth > curr_wth) ;
}

bool streq (const char * a, const char * b) {
    int i = 0 ;
    while (a[i] != 0 && b[i] != 0 && a[i] == b[i]) i++ ;
    return (a[i] == b[i]) ;
}
