#pragma once

#include <string>
#include <fstream>
#include <sstream>
#include "glob.h"
#include "consts.h"
#include "disp.h"

char random_chr () ;
std::string random_name () ;
bool fexists (std::string) ;
void hash_name () ;
bool is_portable_char(char) ;
std::string filename_sanitize (std::string) ;
