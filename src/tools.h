#pragma once

/*
 * This file groups all major enums and structs used throughout the project
 */

/*
 * Methods implemented in ioterm
 */
struct rgb {
    int r ;
    int g ;
    int b ;

    void display () ;
};

/*
 * Token identifiers
 * Enum starts at -1000 so that positive values can be reserved for
 * actual numerals. This means that a token in actually either a cmd, or
 * a positive integer casted to a cmd.
 */
enum cmd {
    SCOPE=-1000, NIL, REC, MAP, MAKE, SAVE, LS, HELP,
    RESET, HASH, NEXT, LSIDE, RSIDE, USIDE, DSIDE, HSIDE, VSIDE,
    ASIDE, ZOOMIN, ZOOMOUT, LSHIFT, RSHIFT, USHIFT, DSHIFT, NUM,
    STR, EXIT, ABORT, REDRAW, CANCELZOOM,
} ;

enum msg_log {
    UNKCHR, NOSELEC, NOINDIC, NOFILE, PARSE, NOSUCHKW, CANCEL,
    RESELEC, REINDIC, LONGQUANT, LONGCMD, LONGNAME, FEXISTS, QUIT, RENAME,
    DEFQUANT, DONE, NEWSCOPE, LOADED, SAVED, NEWMAP, BUILT, EMPTY,
    SIGLS, SIGRESET, SIGHELP, NEWFOCUS, EXCEPTION, FLIP, NPORTABLE,
};

/*
 * Categories of character distinguished by the tokenizer.
 * A word boundary is whenever at least one of the two adjacent characters
 * is neither a KEYWORD nor a NUMERIC, nor accessed without spaces from a
 * MODIFIER. All non-alphabetic keywords are a single character
 * long.
 */
enum types_chr {
    KEYWORD, SELECTOR, INDICATOR, MODIFIER,
    SYMBOL, UNKNOWN, BLANK, NUMERIC
};

/*
 * To remember user choices between executions
 */
enum option {
    OPT_INIT, OPT_DENY, OPT_ALLOW,
};

/*
 * A slice of the command. Created by the tokenizer and read by the lexer.
 */
struct slice {
    int beg ;
    int len ;
};

/*
 * Zoom command
 */
struct adjust {
    bool active ;
    int indicator ;
    int quantifier ;
};
