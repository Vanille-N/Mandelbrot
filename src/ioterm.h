#pragma once

#include <iostream>
#include <string>
#include <sstream>

#include "consts.h"
#include "tools.h"
#include "glob.h"
#include "meta.h"

const std::string PLAIN     = "\033[0m" ;
const std::string BLACK     = "\033[30m" ;
const std::string BOLD      = "\033[1m" ;
const std::string UNDERLINE = "\033[4m" ;
const std::string BLINK     = "\033[5m" ;
const std::string RED       = "\033[31m" ;
const std::string GREEN     = "\033[32m" ;
const std::string YELLOW    = "\033[33m" ;
const std::string BLUE      = "\033[34m" ;
const std::string PURPLE    = "\033[35m" ;
const std::string GREY      = "\033[90m" ;
const std::string LGREEN    = "\033[102m" ;

const std::string ULCORNER = "╔" ;
const std::string URCORNER = "╗" ;
const std::string DLCORNER = "╚" ;
const std::string DRCORNER = "╝" ;
const std::string HBOX = "═" ;
const std::string VBOX = "║" ;

std::string cursor (int, int) ;
std::string repeat (int, std::string) ;
void refresh ();
void log_clear () ;
void prompt_clear () ;
void view_clear () ;
void view_display () ;
void log_redraw () ;
void view_display () ;
void screen_clear () ;
void prompt_make () ;
void log_draw_rect () ;
void log_redraw () ;
