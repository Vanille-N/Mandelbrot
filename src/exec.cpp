#include "exec.h"

int execute () {
    cmd scope_restore = (cmd)curr_scope ;
    int idx = 0 ;
    char ans ;
    bool resol_just_set = false ;
    for (;;) {
        if (idx >= (int)exec.size()) {
            log_info(EMPTY, "Nothing left to do") ;
            goto end ;
        }
        switch (exec[idx]) {
            case SCOPE:
                if (idx+1 >= (int)exec.size()) {
                    log_err(PARSE, "No scope specified") ;
                    goto end ;
                }
                switch (exec[idx+1]) {
                    case HELP:
                        scope_help_print() ;
                        log_info(SIGHELP, "") ;
                        log_info(DONE, "Terminate") ;
                        goto end ;
                    case NIL: case REC: case MAP: case MAKE: case SAVE:
                        log_info(NEWSCOPE, "Switching to " + kw[exec[idx+1]]) ;
                        curr_scope = exec[idx+1] ;
                        log_info(DONE, "Terminate") ;
                        scope_enter_action(exec[idx+1]) ;
                        return 1 ;
                    case RESET:
                        log_info(NEWSCOPE, "Reset scope : NIL") ;
                        curr_scope = NIL ;
                        log_info(DONE, "Terminate") ;
                        return 1 ;
                    default:
                        log_err(PARSE, kw[exec[idx+1]]  + " not expected after keyword 'scope'") ;
                        goto keepls ;
                }
            case NIL: case REC: case MAP: case MAKE: case SAVE:
                log_info(NEWSCOPE, "Temporary scope change to " + kw[exec[idx]]) ;
                curr_scope = exec[idx] ;
                idx++ ;
                break ;
            case LS:
                switch (curr_scope) {
                    case MAP:
                        if (idx+1 < (int)exec.size() && exec[idx+1]==NUM) {
                            curr_lspage = exec[idx+2] ;
                        } else {
                            curr_lspage = 0 ;
                        }
                        ls_map_print() ;
                        log_info(SIGLS, "Listing maps") ;
                        log_info(DONE, "Terminate") ;
                        goto keepls ;
                    case NIL:
                        if (idx+1 < (int)exec.size() && exec[idx+1]==NUM) {
                            curr_lspage = exec[idx+2] ;
                        } else {
                            curr_lspage = 0 ;
                        }
                        ls_nil_print() ;
                        log_info(SIGLS, "Listing settings files") ;
                        log_info(DONE, "Terminate") ;
                        goto keepls ;
                    case SAVE:
                        if (idx+1 < (int)exec.size() && exec[idx+1]==NUM) {
                            curr_lspage = exec[idx+2] ;
                        } else {
                            curr_lspage = 0 ;
                        }
                        ls_save_print() ;
                        log_info(SIGLS, "Listing save files") ;
                        log_info(DONE, "Terminate") ;
                        goto keepls ;
                    case MAKE:
                        if (idx+1 < (int)exec.size() && exec[idx+1]==NUM) {
                            curr_lspage = exec[idx+2] ;
                        } else {
                            curr_lspage = 0 ;
                        }
                        ls_make_print() ;
                        log_info(SIGLS, "Listing existing images") ;
                        log_info(DONE, "Terminate") ;
                        goto keepls ;
                    case REC:
                        ls_rec_print() ;
                        log_info(SIGLS, "Displaying current view settings") ;
                        log_info(DONE, "Terminate") ;
                        goto end ;
                    default:
                        log_err(PARSE, "LS not expected in current scope") ;
                        goto keepls ;
                }
            case HELP:
                switch (curr_scope) {
                    case MAP:
                        map_help_print() ; break ;
                    case SAVE:
                        save_help_print() ; break ;
                    case REC:
                        rec_help_print() ; break ;
                    case MAKE:
                        make_help_print() ; break ;
                    case NIL:
                        nil_help_print() ; break ;
                    default: break ;
                    }
                log_info(SIGHELP, "") ;
                log_info(DONE, "Terminate") ;
                goto end ;
            case RESET:
                switch (curr_scope) {
                    case MAP:
                        map_reset() ;
                        scope_enter_action(MAP) ;
                        break ;
                    case REC:
                        rec_reset() ;
                        scope_enter_action(REC) ;
                        break ;
                    case MAKE:
                        make_reset() ;
                        scope_enter_action(MAKE) ;
                        break ;
                    case NIL:
                        nil_reset() ;
                        scope_enter_action(NIL) ;
                        break ;
                    default:
                        log_err(PARSE, "No reset for scope " + kw[curr_scope]) ;
                        goto keepls ;
                }
                log_info(SIGRESET, "") ;
                log_info(DONE, "Terminate") ;
                goto end ;
            case HASH:
                switch (curr_scope) {
                    case SAVE:
                        hash_name() ;
                        save_output() ;
                        goto keepls ;
                    case MAKE:
                        hash_name() ;
                        image_make() ;
                        goto keepls ;
                    case NIL:
                        hash_name() ;
                        meta_output() ;
                        goto keepls ;
                    default:
                        log_err(PARSE, kw[HASH] + " not expected in scope " + kw[curr_scope]) ;
                        goto keepls ;
                }
            case NEXT:
                if (curr_lspage == -1) {
                    log_err(EXCEPTION, "No ls page displayed") ;
                    goto end ;
                }
                curr_lspage++ ;
                switch (ls_scope) {
                    case MAP:
                        ls_map_print() ;
                        break ;
                    case SAVE:
                        ls_save_print() ;
                        break ;
                    case MAKE:
                        ls_make_print() ;
                        break ;
                    case REC:
                        ls_rec_print() ;
                        break ;
                    case NIL:
                        ls_nil_print() ;
                        break ;
                    default: break ;
                }
                log_info(DONE, "Terminate") ;
                goto keepls ;
            case LSIDE: case RSIDE: case USIDE: case DSIDE: case HSIDE: case ASIDE:
            case VSIDE: case ZOOMIN: case ZOOMOUT: case LSHIFT: case RSHIFT: case USHIFT: case DSHIFT:
            {
                // New scope is necessary to prevent
                // `error: jump to case label`
                if (curr_scope != REC) {
                    log_err(PARSE, "View specifications not expected here") ;
                    goto end ;
                }
                adjust chL {false, -1, -1} ;
                adjust chR = chL, chU = chL, chD = chL ;
                for (int i = idx; i < (int)exec.size(); i++) {
                    switch (exec[i]) {
                        case RSIDE:
                            chR.active = true ; break ;
                        case LSIDE:
                            chL.active = true ; break ;
                        case USIDE:
                            chU.active = true ; break ;
                        case DSIDE:
                            chD.active = true ; break ;
                        case HSIDE:
                            chR.active = chL.active = true ; break ;
                        case VSIDE:
                            chU.active = chD.active = true ; break ;
                        case ASIDE:
                            chL.active = chR.active = chU.active = chD.active = true ; break ;
                        case ZOOMIN:
                            if (chL.active && chL.indicator == -1) chL.indicator = ZOOMIN ;
                            if (chR.active && chR.indicator == -1) chR.indicator = ZOOMIN ;
                            if (chU.active && chU.indicator == -1) chU.indicator = ZOOMIN ;
                            if (chD.active && chD.indicator == -1) chD.indicator = ZOOMIN ;
                            break ;
                        case ZOOMOUT:
                            if (chL.active && chL.indicator == -1) chL.indicator = ZOOMOUT ;
                            if (chR.active && chR.indicator == -1) chR.indicator = ZOOMOUT ;
                            if (chU.active && chU.indicator == -1) chU.indicator = ZOOMOUT ;
                            if (chD.active && chD.indicator == -1) chD.indicator = ZOOMOUT ;
                            break ;
                        case LSHIFT:
                            if (chL.active && chL.indicator == -1) chL.indicator = ZOOMOUT ;
                            if (chR.active && chR.indicator == -1) chR.indicator = ZOOMIN ;
                            break ;
                        case RSHIFT:
                            if (chL.active && chL.indicator == -1) chL.indicator = ZOOMIN ;
                            if (chR.active && chR.indicator == -1) chR.indicator = ZOOMOUT ;
                            break ;
                        case USHIFT:
                            if (chU.active && chU.indicator == -1) chU.indicator = ZOOMOUT ;
                            if (chD.active && chD.indicator == -1) chD.indicator = ZOOMIN ;
                            break ;
                        case DSHIFT:
                            if (chU.active && chU.indicator == -1) chU.indicator = ZOOMIN ;
                            if (chD.active && chD.indicator == -1) chD.indicator = ZOOMOUT ;
                            break ;
                        case NUM:
                            i++ ;
                            if (chL.active && chL.quantifier == -1) chL.quantifier = exec[i] ;
                            if (chR.active && chR.quantifier == -1) chR.quantifier = exec[i] ;
                            if (chU.active && chU.quantifier == -1) chU.quantifier = exec[i] ;
                            if (chD.active && chD.quantifier == -1) chD.quantifier = exec[i] ;
                            break ;
                        default:
                            goto act ;
                    }
                }
                act:
                focus_adjust() ;
                double new_lt = view_lt, new_rt = view_rt, new_hi = view_hi, new_lo = view_lo ;
                save_zoom() ;
                if (chL.active && chL.indicator != -1) {
                    new_lt = calc_newfocus(LSIDE, chL.indicator, chL.quantifier) ;
                }
                if (chR.active && chR.indicator != -1) {
                    new_rt = calc_newfocus(RSIDE, chR.indicator, chR.quantifier) ;
                }
                if (chU.active && chU.indicator != -1) {
                    new_hi = calc_newfocus(USIDE, chU.indicator, chU.quantifier) ;
                }
                if (chD.active && chD.indicator != -1) {
                    new_lo = calc_newfocus(DSIDE, chD.indicator, chD.quantifier) ;
                }
                bool flipped = false ;
                if (new_lo > new_hi) {
                    log_info(FLIP, "Exchanged left/right bounds") ;
                    view_hi = new_lo ;
                    view_lo = new_hi ;
                    flipped = true ;
                } else {
                    view_hi = new_hi ;
                    view_lo = new_lo ;
                }
                if (new_lt > new_rt) {
                    log_info(FLIP, "Exchanged up/down bounds") ;
                    view_lt = new_rt ;
                    view_rt = new_rt ;
                    flipped = true ;
                } else {
                    view_lt = new_lt ;
                    view_rt = new_rt ;
                }
                focus_adjust() ;
                preview_redraw() ;
                if (flipped && log_warn(CANCEL, "(y/n)") == 'y') {
                    cancel_zoom() ;
                    focus_adjust() ;
                    preview_redraw() ;
                }
                log_info(DONE, "Terminate") ;
                goto end ;
            }
            case CANCELZOOM:
                cancel_zoom() ;
                focus_adjust() ;
                preview_redraw() ;
                log_info(DONE, "Cancelled last zoom");
                goto end ;
            case NUM:
                switch (curr_scope) {
                    case MAP:
                        map_choose(exec[idx+1]) ;
                        scope_enter_action(MAP) ;
                        log_info(DONE, "Changed map" ) ;
                        goto keepls ;
                    case MAKE:
                        if (resol_just_set) {
                            diverge_iter = exec[idx+1] ;
                        } else {
                            resol_set(exec[idx+1]) ;
                            resol_just_set = true ;
                        }
                        focus_adjust() ;
                        scope_enter_action(MAKE) ;
                        idx += 2 ;
                        continue ;
                    case SAVE:
                        save_input(exec[idx+1]) ;
                        focus_adjust() ;
                        preview_redraw() ;
                        log_info(DONE, "Terminate") ;
                        goto keepls ;
                    case NIL:
                        meta_input(exec[idx+1]) ;
                        log_info(DONE, "Terminate") ;
                        goto keepls ;
                    default:
                        log_err(PARSE, "Quantifier not expected here") ;
                        goto keepls ;
                }
            case STR:
                switch (curr_scope) {
                    case SAVE:
                        save_output() ;
                        log_info(SAVED, "." + curr_name + ".save") ;
                        log_info(DONE, "Terminate") ;
                        goto keepls ;
                    case MAKE:
                        image_make() ;
                        log_info(BUILT, curr_name + ".ppm") ;
                        log_info(DONE, "Terminate") ;
                        goto end ;
                    case NIL:
                        meta_output() ;
                        log_info(SAVED, "." + curr_name + ".meta") ;
                        log_info(DONE, "Terminate") ;
                        goto keepls ;
                    default:
                        log_err(PARSE, "String literal not expected here") ;
                        goto keepls ;
                }
                goto end ;
            case EXIT:
                if (idx+1 <= (int)exec.size() && exec[idx+1] == EXIT) {
                    return 0 ;
                } else {
                    ans = log_warn(QUIT, "(y/n)") ;
                    if (ans == 'y') {
                        return 0 ;
                    } else {
                        goto keepls ;
                    }
                }
            case REDRAW:
                screen_clear() ;
                focus_adjust() ;
                preview_redraw() ;
                log_draw_rect() ;
                goto end ;
            case ABORT:
                goto keepls ;
        }
    }
    end:
    curr_lspage = -1 ;
    keepls:
    curr_scope = scope_restore ;
    return 1 ;
}

void save_zoom () {
    old_hi = view_hi ;
    old_lo = view_lo ;
    old_lt = view_lt ;
    old_rt = view_rt ;
}

void cancel_zoom () {
    view_hi = old_hi ;
    view_lo = old_lo ;
    view_lt = old_lt ;
    view_rt = old_rt ;
}
