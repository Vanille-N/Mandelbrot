#pragma once

#include <string>
#include <math.h>
#include <fstream>
#include "tools.h"
#include "glob.h"
#include "consts.h"
#include "ioterm.h"
#include "iofile.h"
#include "meta.h"

void log_err (msg_log e, std::string s) ;
char log_warn (msg_log, std::string) ;
void log_info (msg_log, std::string) ;
void ls_save_read (bool) ;
void ls_nil_read (bool) ;
void ls_make_read (bool) ;
void ls_map_read () ;
void ls_nil_print () ;
void ls_rec_print () ;
void ls_make_print () ;
void ls_map_print () ;
void ls_save_print () ;
void help_print (std::string, std::string) ;
void scope_help_print () ;
void map_help_print () ;
void save_help_print () ;
void rec_help_print () ;
void make_help_print () ;
void nil_help_print () ;
std::string msg_header(msg_log) ;
void hist_shift () ;
