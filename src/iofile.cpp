#include "iofile.h"

void save_output () {
    std::ofstream sv ("." + curr_name + ".save") ;
    sv
        << pic_hresol
        << " "
        << pic_vresol
        << "\n" ;
    sv
        << view_lt
        << " "
        << view_rt
        << "\n" ;
    sv
        << view_lo
        << " "
        << view_hi
        << "\n" ;
}

void save_input () {
    std::ifstream sv (curr_name) ;
    sv >> pic_hresol ;
    sv >> pic_vresol ;
    sv >> view_lt ;
    sv >> view_rt ;
    sv >> view_lo ;
    sv >> view_hi ;
}

void save_input (int id) {
    ls_save_read() ;
    if (id < ls_text.size()) {
        curr_name = ls_text[id] ;
    } else {
        log_err(NOFILE, "Quantifier too big") ;
        return ;
    }
    save_input() ;
}

void meta_output () {
    std::ofstream sv ("." + curr_name + ".meta") ;
    sv
        << diverge_radius
        << " "
        << diverge_iter
        << "\n" ;
}

void meta_input () {
    std::ifstream sv (curr_name) ;
    sv >> diverge_radius ;
    sv >> diverge_iter ;
    focus_adjust() ;
}

void meta_input (int id) {
    ls_nil_read() ;
    if (id < ls_text.size()) {
        curr_name = ls_text[id] ;
    } else {
        log_err(NOFILE, "Quantifier too big") ;
        return ;
    }
    meta_input() ;
}
