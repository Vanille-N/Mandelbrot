#include "glob.h"

Assoc kw ;

std::string curr_name = "NULL" ;

double diverge_radius = 5 ;
int diverge_iter = 200 ;

cmd curr_scope = NIL ;
int curr_lspage = -1 ;
cmd ls_scope = NIL ;

std::vector<std::string> log_hist ;

double pic_vresol = 1000 ;
double pic_hresol = 1000 ;
double view_hdiv = 1 ;
double view_vdiv = 1 ;
double view_lt = -2.5 ;
double view_rt = .5 ;
double view_hi = 1 ;
double view_lo = -1 ;

std::vector<std::string> ls_text ;
std::vector<std::vector<rgb>> ls_colors ;
std::vector<cmd> exec ;
std::vector<slice> tokens ;
std::vector<rgb> curr_map ;

std::string command ;

int preview [view_hgt * view_wth] ;
int diverge_min = 0 ;

option allow_non_posix_filenames = OPT_INIT ;
