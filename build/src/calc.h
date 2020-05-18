#pragma once

#include <complex>
#include <vector>
#include <iostream>
#include <fstream>
#include "ioterm.h"
#include "glob.h"
#include "meta.h"

double drand () ;
int diverge (std::complex<double>) ;
std::vector<double> linspace (double, double, int) ;
std::vector<int> round (std::vector<double>) ;
void preview_redraw () ;
void image_make () ;
