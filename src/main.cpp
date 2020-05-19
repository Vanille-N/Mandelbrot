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


int main () {
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
