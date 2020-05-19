#pragma once

#include <vector>
#include <string>
#include "tools.h"
#include "assoc.h"
#include "consts.h"

extern Assoc kw ;

extern std::string curr_name ;

extern double diverge_radius ;
extern int diverge_iter ;

extern cmd curr_scope ;
extern int curr_lspage ;
extern cmd ls_scope ;

extern std::vector<std::string> log_hist ;

extern double pic_vresol ;
extern double pic_hresol ;
extern double view_hdiv ;
extern double view_vdiv ;
extern double view_lt ;
extern double view_rt ;
extern double view_hi ;
extern double view_lo ;

extern std::vector<std::string> ls_text ;
extern std::vector<std::vector<rgb>> ls_colors ;
extern std::vector<cmd> exec ;
extern std::vector<slice> tokens ;
extern std::vector<rgb> curr_map ;

extern std::string command ;

extern int preview [view_hgt * view_wth] ;
extern int diverge_min ;

extern option allow_non_posix_filenames ;
