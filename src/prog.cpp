#include <iostream>
#include <vector>
#include <string>
#include <unordered_map>
#include <complex>
#include <fstream>
#include <stdlib.h>
#include <time.h>
#include <cstdarg>




/*********************************
* CHANGE HERE                   *
* If your terminal is too small *
*********************************/
static const int view_hgt = 140 ;
static const int view_wth = 160 ;
/********************************/




static const std::string PLAIN     = "\033[0m" ;
static const std::string BLACK     = "\033[30m" ;
static const std::string BOLD      = "\033[1m" ;
static const std::string UNDERLINE = "\033[4m" ;
static const std::string BLINK     = "\033[5m" ;
static const std::string RED       = "\033[31m" ;
static const std::string GREEN     = "\033[32m" ;
static const std::string YELLOW    = "\033[33m" ;
static const std::string BLUE      = "\033[34m" ;
static const std::string LGREEN    = "\033[102m" ;

static const std::string ULCORNER = "╔" ;
static const std::string URCORNER = "╗" ;
static const std::string DLCORNER = "╚" ;
static const std::string DRCORNER = "╝" ;
static const std::string HBOX = "═" ;
static const std::string VBOX = "║" ;

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
    int r ;
    int g ;
    int b ;
};

enum cmd {
    SCOPE=-1000, NIL, REC, MAP, MAKE, SAVE, LS, HELP,
    RESET, HASH, NEXT, LSIDE, RSIDE, USIDE, DSIDE, HSIDE, VSIDE,
    ASIDE, ZOOMIN, ZOOMOUT, LSHIFT, RSHIFT, USHIFT, DSHIFT, NUM,
    STR, EXIT, ABORT,
} ;

enum msg_log {
    UNKCHR, NOSELEC, NOINDIC, NOFILE, PARSE, NOSUCHKW,
    RESELEC, REINDIC, LONGQUANT, LONGCMD, LONGNAME, FEXISTS, QUIT, RENAME,
    DEFQUANT, DONE, NEWSCOPE, LOADED, SAVED, NEWMAP, BUILT, EMPTY,
    SIGLS, SIGRESET, SIGHELP, NEWFOCUS, EXCEPTION, FLIP, NPORTABLE,
} ;


static Assoc kw ;

static std::string curr_name = "NULL" ;

static const int num_maxlen = 7 ;
static const int str_maxlen = 20 ;
static const int cmd_maxlen = 50 ;
static const int view_vpos = 7 ;
static const int view_hpos = 7 ;
static const int log_hgt = 20 ;
static const int log_vpos = 5 ;
static const int log_hpos = view_hpos + view_wth + 7 ;
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

static int preview [view_hgt * view_wth] ;
static int diverge_min = 0 ;


std::string cursor (int i, int j) {
    std::ostringstream str ;
    str
        << "\033["
        << i
        << ";"
        << j
        << "H" ;
    return str.str() ;
}

std::string repeat (int n, std::string s) {
    std::ostringstream str ;
    for (int i = 0; i < n; i++) {
        str << s ;
    }
    return str.str() ;
}

void refresh () {
    std::cout
        << PLAIN
        << cursor(1, 1)
        << '\n' ;
}

void log_clear () {
    std::cout << PLAIN ;
    for (int i = 0; i < view_hgt; i++) {
        std::cout << cursor(log_vpos+i, log_hpos) ;
        for (int j = 0; j < 80; j++) {
            std::cout << ' ' ;
        }
    }
}

void prompt_clear () {
    std::cout << PLAIN ;
    for (int i = 1; i < 5; i++) {
        std::cout << cursor(i, 1) ;
        for (int j = 0; j < 90; j++) {
            putchar(' ') ;
        }
    }
}

void view_clear () {
    std::cout
        << PLAIN
        << BLACK
        << cursor(view_vpos-1, view_hpos-2)
        << ULCORNER
        << repeat(view_wth+2, HBOX)
        << "╗" ;
    for (int i = 0; i < view_hgt-1; i++) {
        std::cout
            << cursor(view_vpos+i, view_hpos-2)
            << VBOX
            << std::string(view_wth+2, ' ')
            << VBOX ;
    }
    std::cout
        << cursor(view_vpos+view_hgt-1, view_hpos-2)
        << DLCORNER
        << repeat(view_wth+2, HBOX)
        << DRCORNER
        << PLAIN ;
}

void resol_set (double resol) {
    pic_vresol = resol ;
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

rgb view_colorspread (int dv) {
    int prop = (int)std::ceil(256 * (dv - diverge_min) / (diverge_iter - diverge_min)) ;
    if (prop < 128) {
        return {0, 0, prop * 2} ;
    } else {
        return {(prop - 128) * 2, (prop - 128) * 2, 255} ;
    }
}

void view_display () {
    for (int i = 0; i < view_hgt; i++) {
        for (int j = 0; j < view_wth; j++) {
            diverge_min = std::min(diverge_min, preview[i*view_wth + j]) ;
        }
    }
    for (int i = 0; i < view_hgt/2; i++) {
        std::cout << cursor(view_vpos+i, view_hpos) ;
        for (int j = 0; j < view_wth; j++) {
            rgb bg = view_colorspread(preview[2*i*view_wth + j]) ;
            rgb fg = view_colorspread(preview[(2*i+1)*view_wth + j]) ;
            printf("\033[48;2;%d;%d;%dm\033[38;2;%d;%d;%dm▄", bg.r, bg.g, bg.b, fg.r, fg.g, fg.b) ;
        }
    }
    std::cout << PLAIN ;
}

double drand () {
    return (double)rand() / (double)RAND_MAX ;
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

void screen_clear () {
    std::system("clear") ;
}

void prompt_make () {
    prompt_clear() ;
    std::cout
        << cursor(2, 5)
        << "cmd"
        << BLINK
        << "> "
        << PLAIN
        << cursor(3, 7)
        << "Currently inside scope "
        << kw[curr_scope] ;
    refresh() ;
    std::cout
        << cursor(2, 10)
        << YELLOW
        << BOLD ;
}

std::string msg_header(msg_log m) {
    std::ostringstream str ;
    switch(m) {
        case UNKCHR: case NOSELEC: case NOINDIC: case NOFILE: case PARSE:
        case EXCEPTION: case NOSUCHKW:
            str
                << RED
                << BOLD
                << BLINK
                << "ERROR:"
                << PLAIN ;
            break ;
        case RESELEC: case RENAME: case REINDIC: case LONGQUANT:
        case LONGCMD: case LONGNAME: case FEXISTS: case QUIT: case NPORTABLE:
            str
                << YELLOW
                << BOLD
                << UNDERLINE
                << "WARNING:"
                << PLAIN ;
            break ;
        case DONE:
            str
                << GREEN
                << BOLD
                << "SUCCESS"
                << PLAIN ;
            break ;
        case DEFQUANT: case NEWSCOPE: case LOADED: case SAVED:
        case NEWMAP: case BUILT: case EMPTY: case SIGLS:
        case SIGRESET: case SIGHELP: case NEWFOCUS: case FLIP:
            str
                << BLUE
                << BOLD
                << "INFO:"
                << PLAIN ;
            break ;
        default:
            return "" ;
    }
    switch(m) {
        case UNKCHR:    str <<  " Unknown character                " ; break ;
        case NOSELEC:   str <<  " No selector specified            " ; break ;
        case NOINDIC:   str <<  " No indicator specified           " ; break ;
        case NOFILE:    str <<  " No such file                     " ; break ;
        case PARSE:     str <<  " Critical parsing error           " ; break ;
        case EXCEPTION: str <<  " Runtime exception                " ; break ;
        case NOSUCHKW:  str <<  " Not a valid keyword              " ; break ;
        case RESELEC:   str <<  " Too many selectors specified   " ; break ;
        case RENAME:    str <<  " Name already specified         " ; break ;
        case REINDIC:   str <<  " Too many indicators specified  " ; break ;
        case LONGQUANT: str <<  " Quantifier too long            " ; break ;
        case LONGCMD:   str <<  " Command too long               " ; break ;
        case LONGNAME:  str <<  " String literal too long        " ; break ;
        case FEXISTS:   str <<  " File already exists            " ; break ;
        case NPORTABLE: str <<  " Not a portable filename        " ; break ;
        case QUIT:      str <<  " Quit ?                         " ; break ;
        case DONE:      str <<  "                                 " ; break ;
        case DEFQUANT:  str <<  " Used default quantifier           " ; break ;
        case NEWSCOPE:  str <<  " Changed scope                     " ; break ;
        case LOADED:    str <<  " Loaded save file                  " ; break ;
        case SAVED:     str <<  " Current settings saved            " ; break ;
        case NEWMAP:    str <<  " Changed color map                 " ; break ;
        case BUILT:     str <<  " Done building image               " ; break ;
        case EMPTY:     str <<  " Blank expression                  " ; break ;
        case SIGLS:     str <<  " Asked for listing                 " ; break ;
        case SIGRESET:  str <<  " Asked for reset                   " ; break ;
        case SIGHELP:   str <<  " Asked for help                    " ; break ;
        case NEWFOCUS:  str <<  " Adjusting focus                   " ; break ;
        case FLIP:      str <<  " Focus was flipped                 " ; break ;
    }
    return str.str() ;
}

void log_draw_rect () {
    std::cout
        << PLAIN
        << BLACK
        << cursor(log_vpos, log_hpos-2)
        << ULCORNER
        << repeat(80, HBOX)
        << URCORNER ;
    for (int i = 0; i <= log_hgt; i++) {
        std::cout
            << cursor(log_vpos+i+1, log_hpos-2)
            << VBOX
            << cursor(log_vpos+i+1, log_hpos+79)
            << VBOX;
    }
    std::cout
        << cursor(log_vpos+log_hgt+1, log_hpos-2)
        << DLCORNER
        << repeat(80, HBOX)
        << DRCORNER
        << PLAIN ;
}

void log_redraw () {
    int L = log_vpos, C = log_hpos ;
    log_clear() ;
    std::cout << PLAIN ;
    for (int i = log_hist.size()-1; i >= 0; i--) {
        std::cout
            << cursor(L-i+(int)log_hist.size(), C)
            << log_hist[i] ;
    }
    log_draw_rect() ;
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

void ls_save_read () {
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

void ls_nil_print () {
    ls_scope = NIL ;
    view_clear() ;
    ls_nil_read() ;
    int id = curr_lspage * entry_nb ;
    std::cout
        << PLAIN
        << cursor(view_vpos+2, view_hpos+2)
        << "Showing page "
        << curr_lspage
        << " for nil" ;
    int i ;
    for (i = 0; i < std::min((int)ls_text.size()-id, entry_nb); i++) {
        std::cout
            << cursor(view_vpos+4+i, view_hpos+2)
            << PLAIN
            << BOLD
            << id+i
            << ": "
            << YELLOW
            << ls_text[id+i] ;
    }
    if (i == 0) {
        std::cout
            << cursor(view_vpos+4, view_hpos+2)
            << "Empty" ;
    }
    std::cout << PLAIN ;
}

void ls_save_print () {
    ls_scope = SAVE ;
    view_clear() ;
    ls_save_read() ;
    int id = curr_lspage * entry_nb ;
    std::cout
        << PLAIN
        << cursor(view_vpos+2, view_hpos+2)
        << "Showing page "
        << curr_lspage
        << " for save" ;
    int i ;
    for (i = 0; i < std::min((int)ls_text.size()-id, entry_nb); i++) {
        std::cout
            << cursor(view_vpos+4+i, view_hpos+2)
            << PLAIN
            << BOLD
            << id+i
            << ": "
            << YELLOW
            << ls_text[id+i] ;
    }
    if (i == 0) {
        std::cout
            << cursor(view_vpos+4, view_hpos+2)
            << "Empty" ;
    }
    std::cout << PLAIN ;
}

void ls_rec_print () {
    ls_scope = REC ;
    view_clear() ;
    focus_adjust() ;
    std::cout
        << PLAIN
        << cursor(view_vpos+2, view_hpos+2)
        << "Current settings for rec"
        << cursor(view_vpos+4, view_hpos+2)
        << "Size (in pixels): vertical "
        << (int)std::ceil(pic_vresol)
        << " ; horizontal "
        << (int)std::ceil(pic_hresol)
        << cursor(view_vpos+5, view_hpos+2)
        << "Showing complex plane"
        << cursor(view_vpos+6, view_hpos+2)
        << "    from "
        << view_lt
        << "+"
        << view_lo
        << "i"
        << cursor(view_vpos+7, view_hpos+2)
        << "    to "
        << view_rt
        << "+"
        << view_hi
        << "i" ;
}

void ls_make_print () {
    ls_scope = MAKE ;
    view_clear() ;
    ls_make_read() ;
    int id = curr_lspage * entry_nb ;
    std::cout
        << PLAIN
        << cursor(view_vpos+2, view_hpos+2)
        << "Showing page "
        << curr_lspage
        << " for make" ;
    int i ;
    for (i = 0; i < std::min((int)ls_text.size()-id, entry_nb); i++) {
        std::cout
            << cursor(view_vpos+4+i, view_hpos+2)
            << PLAIN
            << BOLD
            << id+i
            << ": "
            << YELLOW
            << ls_text[id+i] ;
    }
    if (i == 0) {
        std::cout
            << cursor(view_vpos+4, view_hpos+2)
            << "Empty" ;
    }
    std::cout << PLAIN ;
    focus_adjust() ;
    std::cout
        << cursor(view_vpos+entry_nb+7, view_hpos+2)
        << "Resolution: horizontal "
        << (int)std::ceil(pic_hresol)
        << cursor(view_vpos+entry_nb+8, view_hpos+2)
        << "            vertical   "
        << (int)std::ceil(pic_vresol)
        << cursor(view_vpos+entry_nb+9, view_hpos+2)
        << "Diverge iter: "
        << diverge_iter
        << cursor(view_vpos+entry_nb+10, view_hpos+2)
        << "Diverge radius: "
        << (int)std::ceil(diverge_radius) ;
}

void ls_map_print () {
    ls_scope = MAP ;
    view_clear() ;
    ls_map_read() ;
    int id = curr_lspage * entry_nb ;
    std::cout
        << PLAIN
        << cursor(view_vpos+2, view_hpos+2)
        << "Showing page "
        << curr_lspage
        << " for map" ;
    int i ;
    for (i = 0; i < std::min((int)ls_colors.size()-id, entry_nb); i++) {
        std::cout
            << cursor(view_vpos+4+i, view_hpos+2)
            << PLAIN
            << BOLD
            << id+i
            << ": "
            << cursor(view_vpos+4+i, view_hpos+7) ;
        for (int j = 0; j < ls_colors[id+i].size(); j++) {
            auto col = ls_colors[id+i][j] ;
            printf("\033[38;2;%d;%d;%dm█", col.r, col.g, col.b) ;
        }
    }
    if (i == 0) {
        std::cout
            << cursor(view_vpos+4, view_hpos+2)
            << "Empty" ;
    }
    std::cout
        << cursor(view_vpos+entry_nb+7, view_hpos+2)
        << PLAIN
        << "Currently selected:"
        << cursor(view_vpos+entry_nb+8, view_hpos+2) ;
    for (int j = 0; j < curr_map.size(); j++) {
        auto col = curr_map[j] ;
        printf("\033[38;2;%d;%d;%dm█", col.r, col.g, col.b) ;
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
            std::cout << cursor(view_vpos+cnt+2, view_hpos+2) ;
            bool color = false ;
            for (int i = 0; i < line.length(); i++) {
                if (line[i] == '$') {
                    if (!color) {
                        switch (line[++i]) {
                            case 'g': std::cout << GREEN ; break ;
                            case 'r': std::cout << RED ; break ;
                            case 'y': std::cout << YELLOW ; break ;
                            case 'b': std::cout << BLUE ; break ;
                            case 'k': std::cout << BLACK ; break ;
                        }
                        color = true ;
                    } else {
                        color = false ;
                        std::cout << PLAIN ;
                    }
                } else {
                    std::cout << line[i] ;
                }
            }
            std::cout << PLAIN ;
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

enum types_chr {KEYWORD, SELECTOR, INDICATOR, MODIFIER, SYMBOL, UNKNOWN, BLANK, NUMERIC} ;

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
        case '\'':
            return MODIFIER ;
        case '0': case '1': case '2': case '3': case '4':
        case '5': case '6': case '7': case '8': case '9':
            return NUMERIC ;
        case ' ':
            return BLANK ;
        default:
            return UNKNOWN ;
    }
}

bool is_num (char c) {
    return '0' <= c && c <= '9' ;
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
    while (idx < command.length()) {
        //std:: cout << "parsing char " << idx+len << "\n" ;
        //std::cout << "Length:" << len << "\n" ;
        curr = chr_type(command[idx]) ;
        if (curr == KEYWORD) {
            len = 0;
            while (idx+len < command.length() && chr_type(command[idx+(++len)]) == KEYWORD) {}
            tokens.push_back({idx, len}) ;
            idx += len ;
        } else if (curr == SYMBOL || curr == SELECTOR || curr == INDICATOR) {
            tokens.push_back({idx, 1}) ;
            idx++ ;
        } else if (curr == MODIFIER) {
            len = 0 ;
            while (idx+len < command.length() && chr_type(command[idx+(++len)]) != BLANK) {}
            tokens.push_back({idx, len}) ;
            idx += len ;
        } else if (curr == NUMERIC) {
            len = 0 ;
            while (idx+len < command.length() && chr_type(command[idx+(++len)]) == NUMERIC) {}
            tokens.push_back({idx, len}) ;
            idx += len;
        } else if (curr == BLANK) {
            idx++ ;
        } else if (curr == UNKNOWN) {
            tokens.push_back({-1, -1}) ;
            log_info(PARSE, "'" + command.substr(idx, 1) + "' not recognized") ;
            break ;
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
    if (len > num_maxlen) {
        char ans = log_warn(LONGQUANT, command.substr(begin, 5) + "... truncate ? (y/n)") ;
        if (ans == 'n') {
            return -1 ;
        } else {
            len = num_maxlen ;
        }
    }
    for (int i = 0; i < len; i++) {
        if ('0' <= command[begin+i] && command[begin+i] <= '9') {
            n = n*10 + command[begin+i] - '0' ;
        } else {
            return -1 ;
        }
    }
    return n ;
}

bool is_portable_char(char c) {
    return '0' <= c && c <= '9'
        || 'a' <= c && c <= 'z'
        || 'A' <= c && c <= 'Z'
        || c == '.'
        || c == '-'
        || c == '_' ;
}

std::string filename_sanitize (std::string fname) {
    std::ostringstream str ;
    int portable = 0 ;
    for (char c: fname) {
        if (portable == 2 || is_portable_char(c)) {
            str << c ;
        } else if (portable == 0) {
            char ans = log_warn(NPORTABLE, "(" + std::string(1, c) + ") remove from name ? (y/n)") ;
            if (ans == 'n') {
                portable = 2 ;
                str << c ;
            } else {
                portable = 1 ;
            }
        }
    }
    return str.str();
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
    return filename_sanitize(command.substr(begin+1, len-1)) ;
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
                if (ans == 'y') {
                    curr_name = name ;
                } else {
                    continue ;
                }
            } else {
                exec.push_back(STR) ;
                curr_name = name ;
                nameset = true ;
            }
        } else if (is_num(command[tokens[i].beg])) {
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

// std::ostream& operator<< (std::ostream &o, const adjust& a) {
//     return o
//         << "active? "
//         << (a.active ? "true" : "false")
//         << "    dir? "
//         << (a.indicator == ZOOMIN ? "in" : "out")
//         <<  "    quant? "
//         << a.quantifier
//         << std::endl ;
// }

void scope_enter_action (cmd s) {
    switch (s) {
        case NIL:  curr_lspage = -1 ; nil_help_print() ; break ;
        case MAKE: curr_lspage = 0 ;  ls_make_print() ; break ;
        case MAP:  curr_lspage = 0 ;  ls_map_print() ; break ;
        case REC:  curr_lspage = -1 ; focus_adjust() ; preview_redraw() ; break ;
        case SAVE: curr_lspage = 0 ;  ls_save_print() ; break ;
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
                        goto end ;
                }
            case NIL: case REC: case MAP: case MAKE: case SAVE:
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
                double new_lt = view_lt, new_rt = view_rt, new_hi = view_hi, new_lo = view_lo ;
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
                if (new_lo > new_hi) {
                    log_info(FLIP, "Exchanged left/right bounds") ;
                    view_hi = new_lo ;
                    view_lo = new_hi ;
                } else {
                    view_hi = new_hi ;
                    view_lo = new_lo ;
                }
                if (new_lt > new_rt) {
                    log_info(FLIP, "Exchanged up/down bounds") ;
                    view_lt = new_rt ;
                    view_rt = new_rt ;
                } else {
                    view_lt = new_lt ;
                    view_rt = new_rt ;
                }
                focus_adjust() ;
                preview_redraw() ;
                log_info(DONE, "Terminate") ;
                goto end ;
                }
            case NUM:
                switch (curr_scope) {
                    case MAP:
                        map_choose(exec[idx+1]) ;
                        scope_enter_action(MAP) ;
                        log_info(DONE, "Terminate" ) ;
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
                        goto end ;
                    case NIL:
                        meta_input(exec[idx+1]) ;
                        log_info(DONE, "Terminate") ;
                        goto keepls ;
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
                if (idx + 1 <= exec.size() && exec[idx+1] == EXIT) {
                    return 0 ;
                } else {
                    ans = log_warn(QUIT, "(y/n)") ;
                    if (ans == 'y') {
                        return 0 ;
                    } else {
                        goto keepls ;
                    }
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

    screen_clear() ;

    focus_adjust() ;
    preview_redraw() ;

    ls_map_read() ;
    map_choose(0) ;
    log_draw_rect() ;

    do {
        prompt_make() ;
        if (log_hist.size() > 0) {
            log_redraw() ;
        }
        getline(std::cin, command) ;
        parse() ;
    } while (execute()) ;

    screen_clear() ;
    return 0 ;
}
