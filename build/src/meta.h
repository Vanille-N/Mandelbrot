#pragma once

#include "tools.h"
#include "glob.h"
#include "disp.h"
#include "calc.h"

void resol_set (double) ;
void focus_adjust () ;
rgb view_colorspread (int) ;
int map_choose (int) ;
void map_reset () ;
void make_reset () ;
void rec_reset () ;
void nil_reset () ;
double calc_newfocus (cmd, int, int) ;
void scope_enter_action (cmd) ;
