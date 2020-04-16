#include <iostream>
#include <vector>
#include <string>
#include <unordered_map>
#include <complex>
#include <fstream>
#include <stdlib.h>
#include <time.h>

/* Assoc class provides an easy two-way correspondance between user keywords
 * and internal cmd representation.
 */
class Assoc {
public:
    void link (int, std::string) ;
    bool exists (int) ;
    bool exists (std::string) ;
    int operator[] (std::string) ;
    std::string operator[] (int) ;
private:
    std::unordered_map<int, std::string> str ;
    std::unordered_map<std::string, int> id ;
};

void Assoc::link (int num, std::string name) {
    str[num] = name ;
    id[name] = num ;
}

bool Assoc::exists (int num) {
    return str.find(num) != str.end() ;
}

bool Assoc::exists (std::string name) {
    return id.find(name) != id.end() ;
}

int Assoc::operator[] (std::string name) {
    return id[name] ;
}

std::string Assoc::operator[] (int num) {
    return str[num] ;
}

struct rgb {
    int R ;
    int G ;
    int B ;
};

enum cmd {SCOPE=-1000, NIL, REC, MAP, MAKE, LOAD, SAVE, LS, HELP, RESET, HASH, NEXT, LSIDE, RSIDE, USIDE, DSIDE, HSIDE, VSIDE,
    ASIDE, ZOOMIN, ZOOMOUT, LSHIFT, RSHIFT, USHIFT, DSHIFT, NUM, STR, EXIT, ABORT} ;

enum msg_log {UNKCHR, NOSELEC, NOINDIC, NOFILE, PARSE, NOSUCHKW,
    RESELEC, REINDIC, LONGQUANT, LONGCMD, LONGNAME, FEXISTS, QUIT, RENAME,
    DEFQUANT, DONE, NEWSCOPE, LOADED, SAVED, NEWMAP, BUILT, EMPTY,
    SIGLS, SIGRESET, SIGHELP, NEWFOCUS
  } ;

static Assoc kw ;

static std::string curr_name = "NULL" ;

static const int num_maxlen = 7 ;
static const int str_maxlen = 20 ;
static const int cmd_maxlen = 50 ;
static const int view_hgt = 68 ;
static const int view_wth = 68 ;
static const int log_hgt = 20 ;
static const int log_vpos = 5 ;
static const int log_hpos = 85 ;
static const int view_vpos = 5 ;
static const int view_hpos = 5 ;
static const int entry_nb = 20 ;
static const int colors_nb = 40 ;

static double diverge_radius = 5 ;
static int diverge_iter = 200 ;

static cmd curr_scope = NIL ;
static int curr_lspage = -1 ;
static cmd ls_scope = NIL ;

static std::vector<std::string> log_hist ;

static double pic_vresol = 1000 ;
static double pic_hresol = 1000 ;
static double view_hdiv = 1 ;
static double view_vdiv = 1 ;
static double view_lt = -2.5 ;
static double view_rt = .5 ;
static double view_hi = 1 ;
static double view_lo = -1 ;

struct slice {
    int beg ;
    int len ;
};

static std::vector<std::string> ls_text ;
static std::vector<std::vector<rgb>> ls_colors ;
static std::vector<cmd> exec ;
static std::vector<slice> tokens ;
static std::vector<rgb> curr_map ;

static std::string command ;

static int * preview ;
static int diverge_min = 0 ;


void refresh () {
    printf("\033[1;1H\n") ;
}

void log_clear () {
    for (int i = 0; i < view_hgt; i++) {
        printf("\033[%d;%dH", log_vpos+i, log_hpos) ;
        for (int j = 0; j < 80; j++) {
            putchar(' ') ;
        }
    }
}

void prompt_clear () {
    for (int i = 1; i < 5; i++) {
        printf("\033[%d;%dH", i, 1) ;
        for (int j = 0; j < 90; j++) {
            putchar(' ') ;
        }
    }
}

void view_clear () {
    for (int i = 0; i < view_hgt; i++) {
        printf("\033[%d;%dH", view_vpos+i, view_hpos) ;
        for (int j = 0; j < view_wth; j++) {
            putchar(' ') ;
        }
    }
}

// CALCULATIONS

void resol_set (double resol) {
    pic_vresol = resol ;
    //std::cout << pic_vresol ;
}

char random_chr () {
    if (rand() % 2) {
        return (char)('a' + rand()%26) ;
    } else {
        return (char)('A' + rand()%26) ;
    }
}

std::string random_name () {
    srand((unsigned int)(time(NULL) + pic_vresol * (view_hi - view_lo) + pic_hresol * (view_rt - view_lt))) ;
    char name [5] ;
    for (int i = 0; i < 5; i++) {
        name[i] = random_chr() ;
    }
    return std::string(name) ;
}

bool fexists (std::string name) {
    system(("ls " + name + "* 1>.x.txt 2>/dev/null").c_str()) ;
    std::ifstream f (".x.txt") ;
    std::string line ;
    return (getline(f, line) && line.length() > 0) ;
}

void hash_name () {
    do {
        curr_name = random_name() ;
    } while(fexists(curr_name)) ;
}

void focus_adjust () {
    double factor = (view_hi-view_lo) / (view_rt-view_lt) ;
    pic_hresol = pic_vresol / factor ;
    view_hdiv = (view_rt - view_lt) / view_wth ;
    view_vdiv = (view_hi - view_lo) / view_hgt ;
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

int view_colorspread (int dv) {
    int prop = (int)std::ceil(7.0 * (dv - diverge_min) / (diverge_iter - diverge_min)) ;
    switch (prop) {
        case 0: return 34 ;
        case 1: return 94 ;
        case 2: return 36 ;
        case 3: return 96 ;
        case 4: return 97 ;
        case 5: return 37 ;
        case 6: return 90 ;
        case 7: return 30 ;
    }
}

void view_display () {
    for (int i = 0; i < view_hgt; i++) {
        for (int j = 0; j < view_wth; j++) {
            diverge_min = std::min(diverge_min, preview[i*view_wth + j]) ;
        }
    }
    for (int i = 0; i < view_hgt/2; i++) {
        printf("\033[%d;%dH", view_vpos+i, view_hpos) ;
        for (int j = 0; j < view_wth; j++) {
            int bg = 10 + view_colorspread(preview[2*i*view_wth + j]) ;
            int fg = view_colorspread(preview[(2*i+1)*view_wth + j]) ;
            printf("\033[%d;%dm▄", bg, fg) ;
        }
    }
    printf("\033[0m") ;
}

void preview_redraw () {
    view_clear() ;
    auto hspace = linspace(view_lt, view_rt, view_wth) ;
    auto vspace = linspace(view_hi, view_lo, view_hgt) ;
    // Note: hi->lo and not lo->hi to compensate for the fact that
    // i increases downward while y increases upward !
    for (int i = 0; i < view_hgt; i++) {
        for (int j = 0; j < view_wth; j++) {
            preview[i*view_wth + j] = diverge_iter - diverge(std::complex<double> (hspace[j], vspace[i])) ;
        }
    }
    view_display() ;
}

std::vector<int> round (std::vector<double> orig) {
    std::vector<int> r ;
    for (int i = 0; i < orig.size(); i++) {
        r.push_back((int)std::floor(orig[i])) ;
    }
    return r ;
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
    //std::vector<int> cnt (diverge_iter) ;
    for (int i = 0; i < I; i++) {
        for (int j = 0; j < J; j++) {
            n = diverge(std::complex<double> (X[j], Y[i])) ;
            printf("\033[%d;%dH\033[102m \033[1;1H\n", view_vpos+indic_i, view_hpos+indic_j) ;
            otmp << n << " " ;
            if (n < mindv) mindv = n ;
            if (n > maxdv) maxdv = n ;
            //cnt[n]++ ;
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
    while (indic_j < view_wth) {
        printf("\033[%d;%dH\033[102m \033[1;1H\n", view_vpos+indic_i, view_hpos+indic_j) ;
        indic_j++ ;
    }
    printf("\033[1;1H\033[0m") ;
    // Calculations done, now convert to an image !
    std::ofstream pic (curr_name + ".ppm") ;
    std::ifstream itmp ("tmp") ;
    pic << "P3\n" << J << " " << I << "\n255\n" ;
    //std::vector<int> spread (diverge_iter) ;
    // for (int i = 0; i < diverge_iter; i++) {
    //     std::cout << cnt[i] << " " ;
    // } std::cout << "\n" ;
    // Turn cnt to a cumulative
    // for (int i = 1; i < diverge_iter; i++) {
    //     cnt[i] += cnt[i-1] ;
    // }
    // int a = 0 ;
    // int N = I*J ;
    // for (int i = 0; i < diverge_iter; i++) {
    //     if (a < colors_nb-1) a++ ;
    //     while (a < colors_nb-1 && cnt[a] < i * N / colors_nb) {
    //         a++ ;
    //     }
    //     spread[i] = a ;
    // }
    // for (int i = 0; i < diverge_iter; i++) {
    //     std::cout << cnt[i] << " " ;
    // } std::cout << "\n" ;
    // for (int i = 0; i < diverge_iter; i++) {
    //     std::cout << "(i:"<< i << " s:" << spread[i] << ") " ;
    // } std::cout << "\n" ;
    rgb C ;
    for (int i = 0; i < I; i++) {
        for (int j = 0; j < J; j++) {
            itmp >> n ;
            if (n == maxdv) {
                C = curr_map[0] ;
            } else {
                C = curr_map[colors_nb - (int)std::max(std::ceil((colors_nb-1) * ((double)n - mindv) / ((maxdv-1) - mindv)), 1.)] ;
            }
            pic << C.R << " " << C.G << " " << C.B << " " ;
        }
        pic << "\n" ;
    }
    itmp.close() ;
    pic.close() ;
    system("rm tmp") ;
}

// GRAPHIC OUTPUT
void screen_clear () {
    std::system("clear") ;
}

void prompt_make () {
    prompt_clear() ;
    printf("\033[2;5Hcmd\033[5m> \033[0m") ;
    printf("\033[3;7HCurrently inside scope ") ;
    for (int i = 0; i < kw[curr_scope].length(); i++) {
        putchar(kw[curr_scope][i]) ;
    }
    refresh() ;
    printf("\033[2;10H\033[0m") ;
}

std::string msg_header(msg_log m) {
    switch(m) {
        case UNKCHR:    return "\033[31;1;5mERROR:\033[0m Unknown character                " ;
        case NOSELEC:   return "\033[31;1;5mERROR:\033[0m No selector specified            " ;
        case NOINDIC:   return "\033[31;1;5mERROR:\033[0m No indicator specified           " ;
        case NOFILE:    return "\033[31;1;5mERROR:\033[0m No such file                     " ;
        case PARSE:     return "\033[31;1;5mERROR:\033[0m Critical parsing error           " ;
        case NOSUCHKW:  return "\033[31;1;5mERROR:\033[0m Not a valid keyword              " ;
        case RESELEC:   return "\033[33;1;4mWARNING:\033[0m Too many selectors specified   " ;
        case RENAME:    return "\033[33;1;4mWARNING:\033[0m Name already specified         " ;
        case REINDIC:   return "\033[33;1;4mWARNING:\033[0m Too many indicators specified  " ;
        case LONGQUANT: return "\033[33;1;4mWARNING:\033[0m Quantifier too long            " ;
        case LONGCMD:   return "\033[33;1;4mWARNING:\033[0m Command too long               " ;
        case LONGNAME:  return "\033[33;1;4mWARNING:\033[0m String literal too long        " ;
        case FEXISTS:   return "\033[33;1;4mWARNING:\033[0m File already exists            " ;
        case QUIT:      return "\033[33;1;4mWARNING:\033[0m Quit ?                         " ;
        case DEFQUANT:  return "\033[34;1mINFO:\033[0m Used default quantifier           " ;
        case DONE:      return "\033[32;1mSUCCESS\033[0m                                 " ;
        case NEWSCOPE:  return "\033[34;1mINFO:\033[0m Changed scope                     " ;
        case LOADED:    return "\033[34;1mINFO:\033[0m Loaded save file                  " ;
        case SAVED:     return "\033[34;1mINFO:\033[0m Current settings saved            " ;
        case NEWMAP:    return "\033[34;1mINFO:\033[0m Changed color map                 " ;
        case BUILT:     return "\033[34;1mINFO:\033[0m Done building image               " ;
        case EMPTY:     return "\033[34;1mINFO:\033[0m Blank expression                  " ;
        case SIGLS:     return "\033[34;1mINFO:\033[0m Asked for listing                 " ;
        case SIGRESET:  return "\033[34;1mINFO:\033[0m Asked for reset                   " ;
        case SIGHELP:   return "\033[34;1mINFO:\033[0m Asked for help                    " ;
        case NEWFOCUS:  return "\033[34;1mINFO:\033[0m Adjusting focus                   " ;
        default:        return "" ;
    }
}

void log_redraw () {
    int L = log_vpos, C = log_hpos ;
    log_clear() ;
    for (int i = log_hist.size()-1; i >= 0; i--) {
        printf("\033[%d;%dH", L-i+(int)log_hist.size(), C) ;
        std::cout << log_hist[i] ;
    }
    prompt_make() ;
}

void hist_shift () {
    if (log_hist.size() <= log_hgt) return ;
    for (int i = 1; i < log_hist.size(); i++) {
        log_hist[i-1] = log_hist[i] ;
    }
}

void log_err (msg_log e, std::string s) {
    hist_shift() ;
    std::string n = msg_header(e) + s ;
    if (log_hist.size() <= log_hgt) {
        log_hist.push_back(n) ;
    } else {
        log_hist[log_hist.size()-1] = n ;
    }
    log_redraw() ;
}

char log_warn (msg_log w, std::string s) {
    //std::cout << "Got here before shift hist\n" ;
    hist_shift() ;
    //std::cout << "Hist shifted \n" ;
    std::string n = msg_header(w) + s ;
    if (log_hist.size() <= log_hgt) {
        log_hist.push_back(n) ;
    } else {
        log_hist[log_hist.size()-1] = n ;
    }
    //std::cout << "Redraw?\n" ;
    log_redraw() ;
    //std::cout << "Redrawn.\n" ;
    char ans ;
    do {
        scanf("%c", &ans) ;
    } while (ans != 'y' && ans != 'n') ;
    return ans ;
}

void log_info (msg_log i, std::string s) {
    hist_shift() ;
    std::string n = msg_header(i) + s ;
    if (log_hist.size() <= log_hgt) {
        log_hist.push_back(n) ;
    } else {
        log_hist[log_hist.size()-1] = n ;
    }
    log_redraw() ;
}

void ls_load_read () {
    ls_text.clear() ;
    system("ls -a .*.save 1>.x.txt 2>/dev/null") ;
    std::ifstream x (".x.txt") ;
    std::string line ;
    while (getline(x, line)) {
        ls_text.push_back(line) ;
    }
}

void ls_nil_read () {
    ls_text.clear() ;
    system("ls -a .*.meta 1>.x.txt 2>/dev/null") ;
    std::ifstream x (".x.txt") ;
    std::string line ;
    while (getline(x, line)) {
        ls_text.push_back(line) ;
    }
}

void ls_make_read () {
    ls_text.clear() ;
    system("ls *.ppm 1>.x.txt 2>/dev/null") ;
    std::ifstream x (".x.txt") ;
    std::string line ;
    while (getline(x, line)) {
        ls_text.push_back(line) ;
    }
}

void ls_map_read () {
    ls_colors.clear() ;
    std::ifstream x (".cmaps.txt") ;
    int r, g, b ;
    int N ;
    x >> N ;
    //std::cout << N << "\n" ;
    for (int n = 0; n < N; n++) {
        std::vector<rgb> m ;
        for (int i = 0; i < colors_nb; i++) {
            //std::cout << "." << i << "\n" ;
            x >> r ;
            x >> g ;
            x >> b ;
            m.push_back(rgb{r, g, b}) ;
        }
        ls_colors.push_back(m) ;
    }
}

int map_choose (int id) {
    ls_map_read() ;
    if (id < ls_colors.size()) {
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
    resol_set(1) ;
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

void focus_change (cmd side, int indic, int quant) {
    if (quant == -1) {
        quant = 1 ;
        log_info(DEFQUANT, "1") ;
    }
    switch (side) {
        case LSIDE: view_lt += view_hdiv * quant * (indic==ZOOMIN ? +1 : -1) ; break ;
        case RSIDE: view_rt += view_hdiv * quant * (indic==ZOOMIN ? -1 : +1) ; break ;
        case USIDE: view_hi += view_vdiv * quant * (indic==ZOOMIN ? -1 : +1) ; break ;
        case DSIDE: view_lo += view_vdiv * quant * (indic==ZOOMIN ? +1 : -1) ; break ;
    }
    if (view_lo > view_hi) {
        double tmp = view_hi ;
        view_hi = view_lo ;
        view_lo = tmp ;
    }
    if (view_lt > view_rt) {
        double tmp = view_lt ;
        view_lt = view_rt ;
        view_rt = tmp ;
    }
    log_info(NEWFOCUS, "Side " + kw[side] + kw[indic]) ;
}

void ls_load_print () {
    ls_scope = LOAD ;
    view_clear() ;
    ls_load_read() ;
    int id = curr_lspage * entry_nb ;
    printf("\033[%d;%dHShowing page %d for LOAD", view_vpos, view_hpos, curr_lspage) ;
    int i ;
    for (i = 0; i < std::min((int)ls_text.size()-id, entry_nb); i++) {
        printf("\033[%d;%dH\033[1m%d: \033[0m", view_vpos+2+i, view_hpos, id+i) ;
        for (int j = 0; j < ls_text[id+i].length(); j++) {
            putchar(ls_text[id+i][j]) ;
        }
    }
    if (i == 0) {
        printf("\033[%d;%dHEmpty", view_vpos+2, view_hpos) ;
    }
}

void ls_nil_print () {
    ls_scope = NIL ;
    view_clear() ;
    ls_nil_read() ;
    int id = curr_lspage * entry_nb ;
    printf("\033[%d;%dHShowing page %d for NIL", view_vpos, view_hpos, curr_lspage) ;
    int i ;
    for (i = 0; i < std::min((int)ls_text.size()-id, entry_nb); i++) {
        printf("\033[%d;%dH\033[1m%d: \033[0m", view_vpos+2+i, view_hpos, id+i) ;
        for (int j = 0; j < ls_text[id+i].length(); j++) {
            putchar(ls_text[id+i][j]) ;
        }
    }
    if (i == 0) {
        printf("\033[%d;%dHEmpty", view_vpos+2, view_hpos) ;
    }
}

void ls_save_print () {
    ls_scope = SAVE ;
    view_clear() ;
    ls_load_read() ;
    int id = curr_lspage * entry_nb ;
    printf("\033[%d;%dHShowing page %d for SAVE", view_vpos, view_hpos, curr_lspage) ;
    int i ;
    for (i = 0; i < std::min((int)ls_text.size()-id, entry_nb); i++) {
        printf("\033[%d;%dH\033[1m%d: \033[0m", view_vpos+2+i, view_hpos, id+i) ;
        for (int j = 0; j < ls_text[id+i].length(); j++) {
            putchar(ls_text[id+i][j]) ;
        }
    }
    if (i == 0) {
        printf("\033[%d;%dHEmpty", view_vpos+2, view_hpos) ;
    }
}

void ls_rec_print () {
    ls_scope = REC ;
    view_clear() ;
    focus_adjust() ;
    printf("\033[%d;%dHCurrent settings for REC", view_vpos, view_hpos) ;
    printf("\033[%d;%dHSize (in pixels): vertical %d ; horizontal %d", view_vpos+2, view_hpos, (int)std::ceil(pic_vresol), (int)std::ceil(pic_hresol)) ;
    printf("\033[%d;%dHShowing complex plane", view_vpos+3, view_hpos) ;
    printf("\033[%d;%dHfrom %f+%fi", view_vpos+4, view_hpos, view_lt, view_lo) ;
    printf("\033[%d;%dHto %f+%fi", view_vpos+5, view_hpos, view_rt, view_hi) ;
}

void ls_make_print () {
    ls_scope = MAKE ;
    view_clear() ;
    ls_make_read() ;
    int id = curr_lspage * entry_nb ;
    printf("\033[%d;%dHShowing page %d for MAKE", view_vpos, view_hpos, curr_lspage) ;
    int i ;
    for (i = 0; i < std::min((int)ls_text.size()-id, entry_nb); i++) {
        printf("\033[%d;%dH\033[1m%d: \033[0m", view_vpos+2+i, view_hpos, id+i) ;
        for (int j = 0; j < ls_text[id+i].length(); j++) {
            putchar(ls_text[id+i][j]) ;
        }
    }
    if (i == 0) {
        printf("\033[%d;%dHEmpty", view_vpos+2, view_hpos) ;
    }
    focus_adjust() ;
    printf("\033[%d;%dHResolution: horizontal %d", view_vpos+entry_nb+5, view_hpos, (int)std::ceil(pic_hresol)) ;
    printf("\033[%d;%dH            vertical   %d", view_vpos+entry_nb+6, view_hpos, (int)std::ceil(pic_vresol)) ;
    printf("\033[%d;%dHDiverge iter: %d", view_vpos+entry_nb+7, view_hpos, diverge_iter) ;
    printf("\033[%d;%dHdiverge radius: %d", view_vpos+entry_nb+8, view_hpos, (int)std::ceil(diverge_radius)) ;
}

void ls_map_print () {
    ls_scope = MAP ;
    view_clear() ;
    ls_map_read() ;
    int id = curr_lspage * entry_nb ;
    printf("\033[%d;%dHShowing page %d for MAP", view_vpos, view_hpos, curr_lspage) ;
    int i ;
    for (i = 0; i < std::min((int)ls_colors.size()-id, entry_nb); i++) {
        printf("\033[%d;%dH\033[0m\033[1m%d:", view_vpos+2+i, view_hpos, id+i) ;
        printf("\033[%d;%dH\033[0m", view_vpos+2+i, view_hpos+5) ;
        for (int j = 0; j < ls_colors[id+i].size(); j++) {
            auto col = ls_colors[id+i][j] ;
            printf("\033[38;2;%d;%d;%dm█", col.R, col.G, col.B) ;
        }
    }
    if (i == 0) {
        printf("\033[%d;%dHEmpty", view_vpos+2, view_hpos) ;
    }
    printf("\033[%d;%dH\033[0mCurrently selected:", view_vpos+entry_nb+5, view_hpos) ;
    printf("\033[%d;%dH\033[0m", view_vpos+entry_nb+6, view_hpos) ;
    for (int j = 0; j < curr_map.size(); j++) {
        auto col = curr_map[j] ;
        printf("\033[38;2;%d;%d;%dm█", col.R, col.G, col.B) ;
    }
}

void help_print (std::string indic, std::string term) {
    view_clear() ;
    std::ifstream helpf (".help.txt") ;
    std::string line ;
    bool printing = false ;
    int cnt = 0 ;
    while (getline(helpf, line)) {
        if (line == indic) {
            printing = true ;
        } else if (printing && line == term) {
            return ;
        } else if (printing) {
            printf("\033[%d;%dH", view_vpos+cnt, view_hpos) ;
            for (int i = 0; i < line.length(); i++) {
                putchar(line[i]) ;
            }
            cnt++ ;
        }
    }
}

void scope_help_print () {
    help_print("<SCOPE>", "<END>") ;
}

void map_help_print () {
    help_print("<MAP>", "<END>") ;
}

void save_help_print () {
    help_print("<SAVE>", "<END>") ;
}

void load_help_print () {
    help_print("<LOAD>", "<END>") ;
}

void rec_help_print () {
    help_print("<REC>", "<END>") ;
}

void make_help_print () {
    help_print("<MAKE>", "<END>") ;
}

void nil_help_print () {
    help_print("<NIL>", "<END>") ;
}

// TEXT IO

void save_output () {
    std::ofstream sv ("." + curr_name + ".save") ;
    sv << pic_hresol << " " << pic_vresol << "\n" ;
    sv << view_lt << " " << view_rt << "\n" ;
    sv << view_lo << " " << view_hi << "\n" ;
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
    ls_load_read() ;
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
    sv << diverge_radius << " " << pic_vresol << "\n" ;
}

void meta_input () {
    std::ifstream sv (curr_name) ;
    sv >> diverge_radius ;
    sv >> pic_vresol ;
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

enum types_chr {KEYWORD, SELECTOR, INDICATOR, MODIFIER, SYMBOL, UNKNOWN, BLANK} ;

types_chr chr_type (char c) {
    switch (c) {
        case 'a': case 'c': case 'e': case 'i': case 'k':
        case 'l': case 'm': case 'n': case 'o': case 'p':
        case 'r': case 's': case 'v': case 'd':
            return KEYWORD ;
        case '?': case '#': case '/': case '.': case '~':
            return SYMBOL ;
        case 'L': case 'R': case 'U': case 'D': case 'V':
        case 'H': case 'A':
            return SELECTOR ;
        case '+': case '-': case '<': case '>': case '^':
        case '_':
            return INDICATOR ;
        case '\'': case ':':
            return MODIFIER ;
        case ' ':
            return BLANK ;
        default:
            return UNKNOWN ;
    }
}

/* Cut the string into tokens :
 * find beginning and end of keywords, integers, and strings
 * separate all other symbols
 */
void tokenize () {
    //std::cout << "Begin parse" ;
    tokens.clear() ;
    int idx = 0, len = 0 ;
    types_chr curr ;
    while (idx+len < command.length()) {
        //std:: cout << "parsing char " << idx+len << "\n" ;
        //std::cout << "Length:" << len << "\n" ;
        curr = chr_type(command[idx+len]) ;
        if (curr == KEYWORD) {
            //std::cout << "ItsaKW!\n" ;
            if (idx+len+1 == command.length()) {
                tokens.push_back({idx, len+1}) ;
                break ;
            } else if (chr_type(command[idx+len+1]) == KEYWORD) {
                len++ ;
            } else {
                tokens.push_back({idx, len+1}) ;
                idx += len+1 ;
                len = 0 ;
            }
        } else if (curr == SYMBOL || curr == SELECTOR || curr == INDICATOR) {
            //std::cout << "ItsaSYMBOL!\n" ;
            tokens.push_back({idx, 1}) ;
            idx++ ;
        } else if (curr == MODIFIER) {
            //std::cout << "ItsaMODIF!\n" ;
            while (idx+len < command.length() && chr_type(command[idx+len]) != BLANK) {
                len++ ;
            }
            tokens.push_back({idx, len}) ;
            idx += len ;
            len = 0 ;
        } else if (curr == BLANK) {
            //std::cout << "blank...\n" ;
            idx++ ;
        } else if (curr == UNKNOWN) {
            //std::cout << "wtf?\n" ;
            tokens.push_back({-1, -1}) ;
            log_info(PARSE, "'" + command.substr(idx+len, 1) + "' not recognized") ;
            break ;
            //std::cout << "\n\n\n\n\n\n\n\n<<<" + command.substr(idx+len, 1) + ">>>\n" ;
        }
    }
    // for (int i = 0; i < tokens.size(); i++) {
    //     std::cout << "(" << tokens[i].beg << ", " << tokens[i].len << ") " ;
    // } std::cout << "\n" ;
}

/* Read integer at given location
 */
int int_parse (std::string command, int begin, int len) {
    int n = 0 ;
    // First chr is a ':', ignore it
    if (len-1 > num_maxlen) {
        char ans = log_warn(LONGQUANT, command.substr(begin+1, 5) + "... truncate ? (y/n)") ;
        if (ans == 'n') {
            return -1 ;
        } else {
            len = num_maxlen + 1 ;
        }
    }
    for (int i = 1; i < len; i++) {
        if ('0' <= command[begin+i] && command[begin+i] <= '9') {
            n = n*10 + command[begin+i] - '0' ;
        } else {
            return -1 ;
        }
    }
    return n ;
}

/* Read name at given location
 */
std::string str_parse (int begin, int len) {
    if (len-1 > str_maxlen) {
        char ans = log_warn(LONGNAME, command.substr(begin+1, 5) + "... truncate ? (y/n)") ;
        if (ans == 'n') {
            return "" ;
        } else {
            len = str_maxlen + 1 ;
        }
    }
    return command.substr(begin+1, len-1) ;
}

/* Split command line into usable commands
 */
void parse () {
    //std::cout << "Tokenized!\n" ;
    exec.clear() ;
    //std::cout << "Cleared!\n" ;
    bool nameset = false ;
    if (command.length() > cmd_maxlen) {
        //std::cout << "Too long !\n" ;
        //std::cout << msg_info << "\n" ;
        char ans = log_warn(LONGCMD, command.substr(0, 5) + "... truncate ? (y/n)") ;
        //std::cout << "Passed\n" ;
        if (ans == 'n') {
            exec.push_back(ABORT) ;
            return ;
        } else {
            command = command.substr(0, cmd_maxlen) ;
        }
    }
    tokenize() ;
    for (int i = 0; i < tokens.size(); i++) {
        if (tokens[i].beg == -1) {
            log_err(UNKCHR, "") ;
            exec.push_back(ABORT) ;
        } else if (command[tokens[i].beg] == '\'') {
            auto name = str_parse(tokens[i].beg, tokens[i].len) ;
            if (name.length() == 0) {
                log_err(PARSE, "Process aborted : no name specified") ;
                exec.push_back(ABORT) ;
            } else if (nameset) {
                char ans = log_warn(RENAME, "Overwrite ? (y/n)") ;
                if (ans == 'n') {
                    continue ;
                } else {
                    curr_name = name ;
                }
            } else {
                exec.push_back(STR) ;
                curr_name = name ;
                nameset = true ;
            }
        } else if (command[tokens[i].beg] == ':') {
            int n = int_parse(command, tokens[i].beg, tokens[i].len) ;
            if (n == -1) {
                log_err(PARSE, command.substr(tokens[i].beg, 5) + "... not a valid quantifier") ;
                exec.push_back(ABORT) ;
            } else {
                exec.push_back(NUM) ;
                exec.push_back((cmd)n) ;
            }
        } else {
            auto atom = command.substr(tokens[i].beg, tokens[i].len) ;
            if (kw.exists(atom)) {
                exec.push_back((cmd)kw[atom]) ;
            } else {
                log_err(NOSUCHKW, atom.substr(0, std::min(5, (int)atom.length())) + (atom.length() > 5 ? "..." : "")) ;
                exec.push_back(ABORT) ;
            }
        }
    }
    return ;
}


struct adjust {
    bool active ;
    int indicator ;
    int quantifier ;
};

std::ostream& operator<< (std::ostream &o, const adjust& a) {
    return o << "active? " << (a.active ? "true" : "false") << "    dir? " << (a.indicator == ZOOMIN ? "in" : "out") <<  "    quant? " << a.quantifier << std::endl ;
}

void scope_enter_action (cmd s) {
    switch (s) {
        case NIL: curr_lspage = -1 ; nil_help_print() ; break ;
        case MAKE: curr_lspage = 0 ; ls_make_print() ; break ;
        case MAP: curr_lspage = 0 ; ls_map_print() ; break ;
        case REC: curr_lspage = -1 ; focus_adjust() ; preview_redraw() ; break ;
        case LOAD: curr_lspage = 0 ; ls_load_print() ; break ;
        case SAVE: curr_lspage = -1 ; save_help_print() ; break ;
    }
}


int execute () {
    cmd scope_restore = (cmd)curr_scope ;
    int idx = 0 ;
    char ans ;
    bool resol_just_set = false ;
    for (;;) {
        if (idx >= exec.size()) {
            log_info(EMPTY, "Nothing left to do") ;
            goto end ;
        }
        switch (exec[idx]) {
            case SCOPE:
                if (idx+1 >= exec.size()) {
                    log_err(PARSE, "No scope specified") ;
                    goto end ;
                }
                switch (exec[idx+1]) {
                    case HELP:
                        scope_help_print() ;
                        log_info(DONE, "Terminate") ;
                        goto end ;
                    case NIL: case REC: case MAP: case MAKE: case LOAD: case SAVE:
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
                        goto end ;
                }
            case NIL: case REC: case MAP: case MAKE: case LOAD: case SAVE:
                log_info(NEWSCOPE, "Temporary scope change to " + kw[exec[idx]]) ;
                curr_scope = exec[idx] ;
                idx++ ;
                break ;
            case LS:
                switch (curr_scope) {
                    case MAP:
                        if (idx+1 < exec.size() && exec[idx+1]==NUM) {
                            curr_lspage = exec[idx+2] ;
                        } else {
                            curr_lspage = 0 ;
                        }
                        ls_map_print() ;
                        log_info(SIGLS, "Listing maps") ;
                        log_info(DONE, "Terminate") ;
                        goto keepls ;
                    case LOAD:
                        if (idx+1 < exec.size() && exec[idx+1]==NUM) {
                            curr_lspage = exec[idx+2] ;
                        } else {
                            curr_lspage = 0 ;
                        }
                        ls_load_print() ;
                        log_info(SIGLS, "Listing save files") ;
                        log_info(DONE, "Terminate") ;
                        goto keepls ;
                    case NIL:
                        if (idx+1 < exec.size() && exec[idx+1]==NUM) {
                            curr_lspage = exec[idx+2] ;
                        } else {
                            curr_lspage = 0 ;
                        }
                        ls_nil_print() ;
                        log_info(SIGLS, "Listing settings files") ;
                        log_info(DONE, "Terminate") ;
                        goto keepls ;
                    case SAVE:
                        if (idx+1 < exec.size() && exec[idx+1]==NUM) {
                            curr_lspage = exec[idx+2] ;
                        } else {
                            curr_lspage = 0 ;
                        }
                        ls_save_print() ;
                        log_info(SIGLS, "Listing save files") ;
                        log_info(DONE, "Terminate") ;
                        goto keepls ;
                    case MAKE:
                        if (idx+1 < exec.size() && exec[idx+1]==NUM) {
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
                        goto end ;
                }
            case HELP:
                switch (curr_scope) {
                    case MAP:
                        map_help_print() ; break ;
                    case SAVE:
                        save_help_print() ; break ;
                    case LOAD:
                        load_help_print() ; break ;
                    case REC:
                        rec_help_print() ; break ;
                    case MAKE:
                        make_help_print() ; break ;
                    case NIL:
                        nil_help_print() ; break ;
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
                        goto end ;
                }
                log_info(SIGRESET, "") ;
                log_info(DONE, "Terminate") ;
                goto end ;
            case HASH:
                switch (curr_scope) {
                    case SAVE:
                        hash_name() ;
                        save_output() ;
                        goto end ;
                    case MAKE:
                        hash_name() ;
                        image_make() ;
                        goto end ;
                    default:
                        log_err(PARSE, kw[HASH] + " not expected in scope" + kw[curr_scope]) ;
                        goto end ;
                }
            case NEXT:
                if (curr_lspage == -1) {
                    log_err(PARSE, "No ls page displayed") ;
                    goto end ;
                }
                curr_lspage++ ;
                switch (ls_scope) {
                    case MAP:
                        ls_map_print() ;
                        break ;
                    case LOAD:
                        ls_load_print() ;
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
                }
                log_info(DONE, "Terminate") ;
                goto keepls ;
            case LSIDE: case RSIDE: case USIDE: case DSIDE: case HSIDE: case ASIDE:
            case VSIDE: case ZOOMIN: case ZOOMOUT: case LSHIFT: case RSHIFT: case USHIFT: case DSHIFT:
                {
                if (curr_scope != REC) {
                    log_err(PARSE, "View specifications not expected here") ;
                    goto end ;
                }
                adjust chL {false, -1, -1} ;
                adjust chR = chL, chU = chL, chD = chL ;
                for (int i = idx; i < exec.size(); i++) {
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
                if (chL.active && chL.indicator != -1) focus_change(LSIDE, chL.indicator, chL.quantifier) ;
                if (chR.active && chR.indicator != -1) focus_change(RSIDE, chR.indicator, chR.quantifier) ;
                if (chU.active && chU.indicator != -1) focus_change(USIDE, chU.indicator, chU.quantifier) ;
                if (chD.active && chD.indicator != -1) focus_change(DSIDE, chD.indicator, chD.quantifier) ;
                focus_adjust() ;
                preview_redraw() ;
                log_info(DONE, "Terminate") ;
                goto end ;
                }
            case NUM:
                switch (curr_scope) {
                    case MAP:
                        map_choose(exec[idx+1]) ;
                        log_info(DONE, "Terminate" ) ;
                        goto end ;
                    case MAKE:
                        if (resol_just_set) {
                            diverge_iter = exec[idx+1] ;
                        } else {
                            resol_set(exec[idx+1]) ;
                            resol_just_set = true ;
                        }
                        focus_adjust() ;
                        idx += 2 ;
                        continue ;
                    case LOAD:
                        save_input(exec[idx+1]) ;
                        focus_adjust() ;
                        preview_redraw() ;
                        log_info(DONE, "Terminate") ;
                        goto end ;
                    case NIL:
                        meta_input(exec[idx+1]) ;
                        log_info(DONE, "Terminate") ;
                        goto end ;
                    default:
                        log_err(PARSE, "Quantifier not expected here") ;
                        goto end ;
                }
            case STR:
                switch (curr_scope) {
                    case SAVE:
                        save_output() ;
                        log_info(SAVED, "." + curr_name + ".save") ;
                        log_info(DONE, "Terminate") ;
                        goto end ;
                    case MAKE:
                        image_make() ;
                        log_info(BUILT, curr_name + ".ppm") ;
                        log_info(DONE, "Terminate") ;
                        goto end ;
                    case NIL:
                        meta_output() ;
                        log_info(SAVED, "." + curr_name + ".meta") ;
                        log_info(DONE, "Terminate") ;
                        goto end ;
                    default:
                        log_err(PARSE, "String literal not expected here") ;
                        goto end ;
                }
                goto end ;
            case EXIT:
                ans = log_warn(QUIT, "(y/n)") ;
                if (ans == 'y') {
                    return 0 ;
                } else {
                    goto end ;
                }
            case ABORT:
                goto end ;
        }
    }
    end:
    curr_lspage = -1 ;
    keepls:
    curr_scope = scope_restore ;
    return 1 ;
}


int main () {
    // Build dictionnary
    kw.link(SCOPE, "scope") ;
    kw.link(NIL, "nil") ;
    kw.link(REC, "rec") ;
    kw.link(MAP, "map") ;
    kw.link(MAKE, "make") ;
    kw.link(LOAD, "load") ;
    kw.link(SAVE, "save") ;
    kw.link(LS, "ls") ;
    kw.link(HELP, "?") ;
    kw.link(RESET, ".") ;
    kw.link(HASH, "#") ;
    kw.link(NEXT, "/") ;
    kw.link(LSIDE, "L") ;
    kw.link(RSIDE, "R") ;
    kw.link(USIDE, "U") ;
    kw.link(DSIDE, "D") ;
    kw.link(HSIDE, "H") ;
    kw.link(VSIDE, "V") ;
    kw.link(ASIDE, "A") ;
    kw.link(ZOOMIN, "+") ;
    kw.link(ZOOMOUT, "-") ;
    kw.link(LSHIFT, "<") ;
    kw.link(RSHIFT, ">") ;
    kw.link(USHIFT, "^") ;
    kw.link(DSHIFT, "_") ;
    kw.link(NUM, ":") ;
    kw.link(STR, "'") ;
    kw.link(EXIT, "~") ;

    preview = new int [view_hgt * view_wth] ;
    screen_clear() ;
    // focus_adjust() ;
    //
    // focus_change(LSIDE, ZOOMOUT, 10) ;
    // focus_change(USIDE, ZOOMOUT, 20) ;
    // focus_change(DSIDE, ZOOMOUT, 10) ;
    // focus_change(RSIDE, ZOOMOUT, 10) ;

    focus_adjust() ;
    preview_redraw() ;

    ls_map_read() ;
    map_choose(0) ;

    do {
        //db << "OK\n" ;
        prompt_make() ;
        if (log_hist.size() > 0) {
            log_redraw() ;
        }
        getline(std::cin, command) ;
        //std::cout << "read line\n" ;
        //std::cout << "<<" << command << ">>\n" ;
        parse() ;
        //std::cout << command ;
    } while (execute()) ;

    screen_clear() ;
    return 0 ;
}
