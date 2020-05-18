#include "calc.h"

double drand () {
    return (double)rand() / (double)RAND_MAX ;
}

int diverge (std::complex<double> c) {
    std::complex<double> z (0, 0) ;
    for (int i = 1; i <= diverge_iter; i++) {
        z = z * z + c ;
        if (abs(z) > diverge_radius) return i ;
    }
    return diverge_iter ;
}

std::vector<double> linspace (double a, double b, int n) {
    std::vector<double> v ;
    for (int i = 0; i < n; i++) {
        v.push_back(a + ((b-a)*i)/(n-1)) ;
    }
    return v ;
}

std::vector<int> round (std::vector<double> orig) {
    std::vector<int> r ;
    for (int i = 0; i < (int)orig.size(); i++) {
        r.push_back((int)std::floor(orig[i])) ;
    }
    return r ;
}

void preview_redraw () {
    auto hspace = linspace(view_lt, view_rt, view_wth) ;
    auto vspace = linspace(view_hi, view_lo, view_hgt) ;
    double xvar = (view_rt - view_lt) / view_wth ;
    double yvar = (view_hi - view_lo) / view_hgt ;
    // Note: hi->lo and not lo->hi to compensate for the fact that
    // i increases downward while y increases upward !
    for (int i = 0; i < view_hgt; i++) {
        for (int j = 0; j < view_wth; j++) {
            int nb_samples = 5 ;
            int dv_tot = 0 ;
            for (int k = 0; k < nb_samples; k++) {
                dv_tot += diverge(std::complex<double>(hspace[j] + drand()*xvar, vspace[i] + drand()*yvar)) ;
            }
            preview[i*view_wth + j] = diverge_iter - dv_tot / nb_samples ;
        }
    }
    view_clear() ;
    view_display() ;
}


void image_make () {
    focus_adjust() ;
    preview_redraw() ;
    int I = (int)std::ceil(pic_vresol) ;
    int J = (int)std::ceil(pic_hresol) ;
    std::ofstream otmp ("tmp") ;
    auto X = linspace(view_lt, view_rt, J) ;
    auto Y = linspace(view_hi, view_lo, I) ;
    auto colorspread = round(linspace(0, colors_nb, diverge_iter-diverge_min)) ;
    int indic_i = 0, indic_j = 0 ;
    int corresp = (int)std::ceil((float)(I*J) / (.5 * view_hgt * view_wth)) ;
    int pxdrawn = 0 ;
    int mindv = diverge_iter, maxdv = 0 ;
    int n ;
    for (int i = 0; i < I; i++) {
        for (int j = 0; j < J; j++) {
            n = diverge(std::complex<double> (X[j], Y[i])) ;
            std::cout
                << cursor(view_vpos+indic_i, view_hpos+indic_j)
                << LGREEN
                << ' '
                << cursor(1, 1)
                << '\n' ;
            otmp
                << n
                << " " ;
            if (n < mindv) mindv = n ;
            if (n > maxdv) maxdv = n ;
            if (++pxdrawn % corresp == 0) {
                indic_j++ ;
                if (indic_j == view_wth) {
                    indic_j = 0 ;
                    indic_i++ ;
                }
            }
        }
    }
    otmp.close() ;
    while (indic_i*2 < view_hgt) {
        while (indic_j < view_wth) {
            std::cout << cursor(view_vpos+indic_i, view_hpos+indic_j) ;
            std::cout
                << LGREEN
                << ' '
                << cursor(1, 1)
                << '\n' ;
            indic_j++ ;
        }
        indic_i++ ;
        indic_j = 0 ;
    }
    std::cout
        << cursor(1, 1)
        << PLAIN ;
    // Calculations done, now convert to an image !
    std::ofstream pic (curr_name + ".ppm") ;
    std::ifstream itmp ("tmp") ;
    pic
        << "P3\n"
        << J
        << " "
        << I
        << "\n255\n" ;
    rgb C ;
    for (int i = 0; i < I; i++) {
        for (int j = 0; j < J; j++) {
            itmp >> n ;
            if (n == maxdv) {
                C = curr_map[0] ;
            } else {
                C = curr_map[colors_nb - (int)std::max(std::ceil((colors_nb-1) * ((double)n - mindv) / ((maxdv-1) - mindv)), 1.)] ;
            }
            pic
                << C.r
                << " "
                << C.g
                << " "
                << C.b
                << " " ;
        }
        pic << "\n" ;
    }
    itmp.close() ;
    pic.close() ;
    system("rm tmp") ;
    preview_redraw() ;
}
