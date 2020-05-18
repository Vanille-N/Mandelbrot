#pragma once

#include <string>
#include "tools.h"
#include "glob.h"
#include "strutils.h"

void tokenize () ;
void parse () ;
bool is_num (char) ;
types_chr chr_type (char) ;
std::string str_parse (int, int) ;
int int_parse (int, int) ;
