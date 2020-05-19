#pragma once

struct rgb {
    int r ;
    int g ;
    int b ;
};

enum cmd {
    SCOPE=-1000, NIL, REC, MAP, MAKE, SAVE, LS, HELP,
    RESET, HASH, NEXT, LSIDE, RSIDE, USIDE, DSIDE, HSIDE, VSIDE,
    ASIDE, ZOOMIN, ZOOMOUT, LSHIFT, RSHIFT, USHIFT, DSHIFT, NUM,
    STR, EXIT, ABORT, REDRAW,
} ;

enum msg_log {
    UNKCHR, NOSELEC, NOINDIC, NOFILE, PARSE, NOSUCHKW,
    RESELEC, REINDIC, LONGQUANT, LONGCMD, LONGNAME, FEXISTS, QUIT, RENAME,
    DEFQUANT, DONE, NEWSCOPE, LOADED, SAVED, NEWMAP, BUILT, EMPTY,
    SIGLS, SIGRESET, SIGHELP, NEWFOCUS, EXCEPTION, FLIP, NPORTABLE,
};

enum types_chr {
    KEYWORD, SELECTOR, INDICATOR, MODIFIER,
    SYMBOL, UNKNOWN, BLANK, NUMERIC
};

enum option {
    OPT_INIT, OPT_DENY, OPT_ALLOW,
};

struct slice {
    int beg ;
    int len ;
};

struct adjust {
    bool active ;
    int indicator ;
    int quantifier ;
};
