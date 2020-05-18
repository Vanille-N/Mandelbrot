#include "meta.h"

void resol_set (double resol) {
    pic_vresol = resol ;
}

void focus_adjust () {
    double factor = (view_hi-view_lo) / (view_rt-view_lt) ;
    pic_hresol = pic_vresol / factor ;
    view_hdiv = (view_rt - view_lt) / view_wth ;
    view_vdiv = (view_hi - view_lo) / view_hgt ;
}

rgb view_colorspread (int dv) {
    int prop = (int)std::ceil(256 * (dv - diverge_min) / (diverge_iter - diverge_min)) ;
    if (prop < 128) {
        return {0, 0, prop * 2} ;
    } else {
        return {(prop - 128) * 2, (prop - 128) * 2, 255} ;
    }
}

int map_choose (int id) {
    ls_map_read() ;
    if (id < (int)ls_colors.size()) {
        curr_map = ls_colors[id] ;
    } else {
        log_err(NOFILE, "Map does not exist") ;
        return 1 ;
    }
    return 0 ;
}

void map_reset () {
    map_choose(0) ;
}

void make_reset () {
    resol_set(100) ;
}

void rec_reset () {
    view_lt = -2.5 ;
    view_rt = .5 ;
    view_hi = 1 ;
    view_lo = -1 ;
}

void nil_reset () {
    map_reset() ;
    make_reset() ;
    rec_reset() ;
}

double calc_newfocus (cmd side, int indic, int quant) {
    if (quant == -1) {
        quant = 1 ;
        log_info(DEFQUANT, "1 was inserted") ;
    }
    log_info(NEWFOCUS, "Side " + kw[side] + kw[indic] + " (" + std::to_string(quant) + ")") ;
    switch (side) {
        case LSIDE: return view_lt + view_hdiv * quant * (indic==ZOOMIN ? +1 : -1) ;
        case RSIDE: return view_rt + view_hdiv * quant * (indic==ZOOMIN ? -1 : +1) ;
        case USIDE: return view_hi + view_vdiv * quant * (indic==ZOOMIN ? -1 : +1) ;
        case DSIDE: return view_lo + view_vdiv * quant * (indic==ZOOMIN ? +1 : -1) ;
        default: return 0. ;
    }
}

void scope_enter_action (cmd s) {
    switch (s) {
        case NIL:  curr_lspage = -1 ; nil_help_print() ; break ;
        case MAKE: curr_lspage = 0 ;  ls_make_print() ; break ;
        case MAP:  curr_lspage = 0 ;  ls_map_print() ; break ;
        case REC:  curr_lspage = -1 ; focus_adjust() ; preview_redraw() ; break ;
        case SAVE: curr_lspage = 0 ;  ls_save_print() ; break ;
        default: break ;
    }
}
