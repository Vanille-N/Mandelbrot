#include <iostream>
#include <vector>
#include <string>
#include <unordered_map>
#include <complex>
#include <fstream>
#include <stdlib.h>

static std::ofstream db ("debug.txt") ;

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
static std::string msg_info = "" ;

static const int num_maxlen = 7 ;
static const int str_maxlen = 10 ;
static const int cmd_maxlen = 50 ;
static const int view_hgt = 60 ;
static const int view_wth = 60 ;
static const int logI = 5 ;
static const int logJ = 85 ;
static const int viewI = 5 ;
static const int viewJ = 5 ;
static const int entry_nb = 20 ;
static const int colors_nb = 40 ;

static double diverge_radius = 5 ;
static int diverge_iter = 200 ;

static cmd curr_scope = NIL ;
static int curr_lspage = -1 ;
static cmd ls_scope = NIL ;

static std::vector<msg_log> hist_log ;

static double vresol = 1000 ;
static double hresol = 1000 ;
static double hdiv = 1 ;
static double vdiv = 1 ;
static double lt = -2.5 ;
static double rt = .5 ;
static double hi = 1 ;
static double lo = -1 ;

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


void refresh () {
    printf("\033[1;1H\n") ;
}


void clear_log () {
    for (int i = 0; i < 15; i++) {
        printf("\033[%d;%dH", logI+i, logJ) ;
        for (int j = 0; j < 50; j++) {
            putchar(' ') ;
        }
    }
}

void clear_prompt () {
    for (int i = 1; i < 5; i++) {
        printf("\033[%d;%dH", i, 1) ;
        for (int j = 0; j < 90; j++) {
            putchar(' ') ;
        }
    }
}

void clear_view () {
    for (int i = 0; i < view_hgt; i++) {
        printf("\033[%d;%dH", viewI+i, viewJ) ;
        for (int j = 0; j < view_wth; j++) {
            putchar(' ') ;
        }
    }
}

// CALCULATIONS

void set_resolution (double resol) {
    vresol = resol ;
    std::cout << vresol ;
}

char random_chr () {
    if (rand() % 2) {
        return (char)('a' + rand()%26) ;
    } else {
        return (char)('A' + rand()%26) ;
    }
}

std::string random_name () {
    char name [10] ;
    for (int i = 0; i < 10; i++) {
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

void adjust_focus () {
    double factor = (hi-lo) / (rt-lt) ;
    hresol = vresol / factor ;
    hdiv = (rt - lt) / view_wth ;
    vdiv = (hi - lo) / view_hgt ;
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

void display_preview () {
    for (int i = 0; i < view_hgt/2; i++) {
        printf("\033[%d;%dH", viewI+i, viewJ) ;
        for (int j = 0; j < view_wth; j++) {
            int bg = (preview[2*i*view_wth + j] ? 100 : 106) ;
            int fg = (preview[(2*i+1)*view_wth + j] ? 90 : 96) ;
            printf("\033[%d;%dm▄", bg, fg) ;
        }
    }
    printf("\033[0m") ;
}

void preview_redraw () {
    clear_view() ;
    auto hspace = linspace(lt, rt, view_wth) ;
    auto vspace = linspace(lo, hi, view_hgt) ;
    // std::cout << "\n" ;
    // for (int i = 0; i < viewI; i++) {
    //     std::cout << hspace[i] << " " ;
    // } std::cout << '\n' ;
    // for (int i = 0; i < viewI; i++) {
    //     std::cout << vspace[i] << " " ;
    // } std::cout << '\n' ;
    // std::cout << (1 / (2-1-1)) ;
    for (int i = 0; i < view_hgt; i++) {
        for (int j = 0; j < view_wth; j++) {
            preview[i*view_wth + j] = diverge_iter - diverge(std::complex<double> (hspace[j], vspace[i])) ;
        }
    }
    display_preview() ;
}

std::vector<int> round (std::vector<double> orig) {
    std::vector<int> r ;
    for (int i = 0; i < orig.size(); i++) {
        r.push_back((int)std::floor(orig[i])) ;
    }
    return r ;
}

void make_image () {
    adjust_focus() ;
    preview_redraw() ;
    int I = (int)std::ceil(vresol) ;
    int J = (int)std::ceil(hresol) ;
    std::ofstream pic (curr_name + ".ppm") ;
    pic << "P3\n" << J << " " << I << "\n255\n" ;
    auto X = linspace(lt, rt, J) ;
    auto Y = linspace(lo, hi, I) ;
    auto colorspread = round(linspace(0, colors_nb, diverge_iter)) ;
    int indic_i = 0, indic_j = 0 ;
    int corresp = (int)std::ceil((float)(I*J) / (.5 * view_hgt * view_wth)) ;
    int pxdrawn = 0 ;
    for (int i = 0; i < I; i++) {
        for (int j = 0; j < J; j++) {
            int n = colorspread[diverge_iter - diverge(std::complex<double> (X[j], Y[i]))] ;
            //std::cout << (n ? '.' : '@') ;
            rgb C = curr_map[n] ;
            pic << C.R << " " << C.G << " " << C.B << " " ;
            if (++pxdrawn % corresp == 0) {
                printf("\033[%d;%dH\033[102m \033[1;1H\n", viewI+indic_i, viewJ+indic_j) ;
                indic_j++ ;
                if (indic_j == view_wth) {
                  indic_j = 0 ;
                  indic_i++ ;
                }
            }
        }
        pic << "\n" ;
        //std::cout << "\n" ;
    }
    while (indic_j < view_wth) {
        printf("\033[%d;%dH\033[102m \033[1;1H\n", viewI+indic_i, viewJ+indic_j) ;
        indic_j++ ;
    }
}

// GRAPHIC OUTPUT
void clear_screen () {
    std::system("clear") ;
}

void make_prompt () {
    clear_prompt() ;
    printf("\033[2;5Hcmd\033[5m> \033[0m") ;
    printf("\033[3;7HCurrently inside scope ") ;
    for (int i = 0; i < kw[curr_scope].length(); i++) {
        putchar(kw[curr_scope][i]) ;
    }
    refresh() ;
    printf("\033[2;10H\033[0m") ;
}

void print_msg_header(msg_log m) {
    switch(m) {
        case UNKCHR:
            printf("\033[31;1;5mERROR:\033[0m Unknown character") ;
            break ;
        case NOSELEC:
            printf("\033[31;1;5mERROR:\033[0m No selector specified") ;
            break ;
        case NOINDIC:
            printf("\033[31;1;5mERROR:\033[0m No indicator specified") ;
            break ;
        case NOFILE:
            printf("\033[31;1;5mERROR:\033[0m No such file") ;
            break ;
        case PARSE:
            printf("\033[31;1;5mERROR:\033[0m Critical parsing error") ;
            break ;
        case NOSUCHKW:
            printf("\033[31;1;5mERROR:\033[0m Not a valid keyword") ;
            break ;
        case RESELEC:
            printf("\033[33;1;4mWARNING:\033[0m Too many selectors specified") ;
            break ;
        case RENAME:
            printf("\033[33;1;4mWARNING:\033[0m Name already specified") ;
            break ;
        case REINDIC:
            printf("\033[33;1;4mWARNING:\033[0m Too many indicators specified") ;
            break ;
        case LONGQUANT:
            printf("\033[33;1;4mWARNING:\033[0m Quantifier too long") ;
            break ;
        case LONGCMD:
            printf("\033[33;1;4mWARNING:\033[0m Command too long") ;
            break ;
        case LONGNAME:
            printf("\033[33;1;4mWARNING:\033[0m String literal too long") ;
            break ;
        case FEXISTS:
            printf("\033[33;1;4mWARNING:\033[0m File already exists") ;
            break ;
        case QUIT:
            printf("\033[33;1;4mWARNING:\033[0mQuit ?") ;
            break ;
        case DEFQUANT:
            printf("\033[34;1mINFO:\033[0m Used default quantifier") ;
            break ;
        case DONE:
            printf("\033[32;1mSUCCESS\033[0m") ;
            break ;
        case NEWSCOPE:
            printf("\033[34;1mINFO:\033[0m Changed scope") ;
            break ;
        case LOADED:
            printf("\033[34;1mINFO:\033[0m Loaded save file") ;
            break ;
        case SAVED:
            printf("\033[34;1mINFO:\033[0m Current settings saved") ;
            break ;
        case NEWMAP:
            printf("\033[34;1mINFO:\033[0m Changed color map") ;
            break ;
        case BUILT:
            printf("\033[34;1mINFO:\033[0m Done building image") ;
            break ;
        case EMPTY:
            printf("\033[34;1mINFO:\033[0m Blank expression") ;
            break ;
        case SIGLS:
            printf("\033[34;1mINFO:\033[0m Asked for listing") ;
            break ;
        case SIGRESET:
            printf("\033[34;1mINFO:\033[0m Asked for reset") ;
            break ;
        case SIGHELP:
            printf("\033[34;1mINFO:\033[0m Asked for help") ;
            break ;
        case NEWFOCUS:
            printf("\033[34;1mINFO:\033[0m Adjusting focus") ;
            break ;
    }
}

void print_msg_info() {
    for (int i = 0; i < msg_info.size(); i++) {
        putchar(msg_info[i]) ;
    }
}

void log_redraw () {
    int L = logI, C = logJ ;
    clear_log() ;
    printf("\033[%d;%dH", L, C) ;
    print_msg_header(hist_log[hist_log.size()-1]) ;
    printf("\033[%d;%dH", L+1, C) ;
    print_msg_info() ;
    for (int i = hist_log.size()-2; i >= 0; i--) {
        printf("\033[%d;%dH", L-i+(int)hist_log.size(), C) ;
        print_msg_header(hist_log[i]) ;
    }
    make_prompt() ;
}

void shift_hist () {
    if (hist_log.size() <= 10) return ;
    for (int i = 1; i < hist_log.size(); i++) {
        hist_log[i-1] = hist_log[i] ;
    }
}

void log_err (msg_log e) {
    shift_hist() ;
    if (hist_log.size() <= 10) {
        hist_log.push_back(e) ;
    } else {
        hist_log[hist_log.size()-1] = e ;
    }
    log_redraw() ;
}

char log_warn (msg_log w) {
    //std::cout << "Got here before shift hist\n" ;
    shift_hist() ;
    //std::cout << "Hist shifted \n" ;
    if (hist_log.size() <= 10) {
        hist_log.push_back(w) ;
    } else {
        hist_log[hist_log.size()-1] = w ;
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

void log_info (msg_log i) {
    shift_hist() ;
    if (hist_log.size() <= 10) {
        hist_log.push_back(i) ;
    } else {
        hist_log[hist_log.size()-1] = i ;
    }
    log_redraw() ;
}

void read_ls_load () {
    ls_text.clear() ;
    system("ls *.sv 1>.x.txt 2>/dev/null") ;
    std::ifstream x (".x.txt") ;
    std::string line ;
    while (getline(x, line)) {
        ls_text.push_back(line) ;
    }
}

void read_ls_make () {
    ls_text.clear() ;
    system("ls *.ppm 1>.x.txt 2>/dev/null") ;
    std::ifstream x (".x.txt") ;
    std::string line ;
    while (getline(x, line)) {
        ls_text.push_back(line) ;
    }
}

void read_ls_map () {
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

int set_map (int id) {
    read_ls_map() ;
    if (id < ls_colors.size()) {
        curr_map = ls_colors[id] ;
    } else {
        msg_info = "Map does not exist" ;
        log_err(NOFILE) ;
        return 1 ;
    }
    return 0 ;
}

void reset_map () {
    set_map(0) ;
}

void reset_make () {
    set_resolution(1) ;
}

void reset_rec () {
    lt = -2.5 ;
    rt = .5 ;
    hi = 1 ;
    lo = -1 ;
}

void reset_nil () {
    reset_map() ;
    reset_make() ;
    reset_rec() ;
}

void change_focus (cmd side, int indic, int quant) {
    if (quant == -1) quant = 1 ;
    switch (side) {
        case LSIDE: lt += hdiv * quant * (indic==ZOOMIN ? +1 : -1) ; break ;
        case RSIDE: rt += hdiv * quant * (indic==ZOOMIN ? -1 : +1) ; break ;
        case USIDE: hi += vdiv * quant * (indic==ZOOMIN ? -1 : +1) ; break ;
        case DSIDE: lo += vdiv * quant * (indic==ZOOMIN ? +1 : -1) ; break ;
    }
    if (lo > hi) {
        double tmp = hi ;
        hi = lo ;
        lo = tmp ;
    }
    if (lt > rt) {
        double tmp = lt ;
        lt = rt ;
        rt = tmp ;
    }
    msg_info = "Side " + kw[side] ;
    log_info(NEWFOCUS) ;
}

void print_ls_load () {
    ls_scope = LOAD ;
    clear_view() ;
    read_ls_load() ;
    int id = curr_lspage * entry_nb ;
    printf("\033[%d;%dHShowing page %d for LOAD", viewI, viewJ, curr_lspage) ;
    int i ;
    for (i = 0; i < std::min((int)ls_text.size()-id, entry_nb); i++) {
        printf("\033[%d;%dH\033[1m%d: \033[0m", viewI+2+i, viewJ, id+i) ;
        for (int j = 0; j < ls_text[id+i].length(); j++) {
            putchar(ls_text[id+i][j]) ;
        }
    }
    if (i == 0) {
        printf("\033[%d;%dHEmpty", viewI+2, viewJ) ;
    }
}

void print_ls_save () {
    ls_scope = SAVE ;
    clear_view() ;
    read_ls_load() ;
    int id = curr_lspage * entry_nb ;
    printf("\033[%d;%dHShowing page %d for SAVE", viewI, viewJ, curr_lspage) ;
    int i ;
    for (i = 0; i < std::min((int)ls_text.size()-id, entry_nb); i++) {
        printf("\033[%d;%dH\033[1m%d: \033[0m", viewI+2+i, viewJ, id+i) ;
        for (int j = 0; j < ls_text[id+i].length(); j++) {
            putchar(ls_text[id+i][j]) ;
        }
    }
    if (i == 0) {
        printf("\033[%d;%dHEmpty", viewI+2, viewJ) ;
    }
}

void print_ls_rec () {
    ls_scope = REC ;
    clear_view() ;
    adjust_focus() ;
    printf("\033[%d;%dHCurrent settings for REC", viewI, viewJ) ;
    printf("\033[%d;%dHSize (in pixels): vertical %d ; horizontal %d", viewI+2, viewJ, (int)std::ceil(vresol), (int)std::ceil(hresol)) ;
    printf("\033[%d;%dHShowing complex plane", viewI+3, viewJ) ;
    printf("\033[%d;%dHfrom %f+%fi", viewI+4, viewJ, lt, lo) ;
    printf("\033[%d;%dHto %f+%fi", viewI+5, viewJ, rt, hi) ;
}

void print_ls_make () {
    ls_scope = MAKE ;
    clear_view() ;
    read_ls_make() ;
    int id = curr_lspage * entry_nb ;
    printf("\033[%d;%dHShowing page %d for MAKE", viewI, viewJ, curr_lspage) ;
    int i ;
    for (i = 0; i < std::min((int)ls_text.size()-id, entry_nb); i++) {
        printf("\033[%d;%dH\033[1m%d: \033[0m", viewI+2+i, viewJ, id+i) ;
        for (int j = 0; j < ls_text[id+i].length(); j++) {
            putchar(ls_text[id+i][j]) ;
        }
    }
    if (i == 0) {
        printf("\033[%d;%dHEmpty", viewI+2, viewJ) ;
    }
}

void print_ls_map () {
    ls_scope = MAP ;
    clear_view() ;
    read_ls_map() ;
    int id = curr_lspage * entry_nb ;
    printf("\033[%d;%dHShowing page %d for MAP", viewI, viewJ, curr_lspage) ;
    int i ;
    for (i = 0; i < std::min((int)ls_colors.size()-id, entry_nb); i++) {
        printf("\033[%d;%dH\033[0m\033[1m%d:", viewI+2+i, viewJ, id+i) ;
        printf("\033[%d;%dH\033[0m", viewI+2+i, viewJ+5) ;
        for (int j = 0; j < ls_colors[id+i].size(); j++) {
            auto col = ls_colors[id+i][j] ;
            printf("\033[38;2;%d;%d;%dm█", col.R, col.G, col.B) ;
        }
    }
    if (i == 0) {
        printf("\033[%d;%dHEmpty", viewI+2, viewJ) ;
    }
}

void print_help (std::string indic, std::string term) {
    clear_view() ;
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
            printf("\033[%d;%dH", viewI+cnt, viewJ) ;
            for (int i = 0; i < line.length(); i++) {
                putchar(line[i]) ;
            }
            cnt++ ;
        }
    }
}

void print_scope_help () {
    print_help("<SCOPE>", "<END>") ;
}

void print_map_help () {
    print_help("<MAP>", "<END>") ;
}

void print_save_help () {
    print_help("<SAVE>", "<END>") ;
}

void print_load_help () {
    print_help("<LOAD>", "<END>") ;
}

void print_rec_help () {
    print_help("<REC>", "<END>") ;
}

void print_make_help () {
    print_help("<MAKE>", "<END>") ;
}

void print_nil_help () {
    print_help("<NIL>", "<END>") ;
}

// TEXT IO

void output_save () {
    std::ofstream sv (curr_name + ".sv") ;
    sv << hresol << " " << vresol << "\n" ;
    sv << lt << " " << rt << "\n" ;
    sv << lo << " " << hi << "\n" ;
}

void input_save () {
    std::ifstream sv (curr_name + ".sv") ;
    sv >> hresol ;
    sv >> vresol ;
    sv >> lt ;
    sv >> rt ;
    sv >> lo ;
    sv >> hi ;
}

void input_save (int id) {
    read_ls_load() ;
    if (id < ls_text.size()) {
        curr_name = ls_text[id] ;
    } else {
        msg_info = "Quantifier too big" ;
        log_err(NOFILE) ;
        return ;
    }
    input_save() ;
}

enum types_chr {KEYWORD, SELECTOR, INDICATOR, MODIFIER, SYMBOL, UNKNOWN, BLANK} ;

types_chr chr_type (char c) {
    switch (c) {
        case 'a': case 'c': case 'e': case 'i': case 'k':
        case 'l': case 'm': case 'n': case 'o': case 'p':
        case 'r': case 's': case 'v':
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
            msg_info = "'" + command.substr(idx+len, 1) + "' not recognized" ;
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
int parse_int (std::string command, int begin, int len) {
    int n = 0 ;
    // First chr is a ':', ignore it
    if (len-1 > num_maxlen) {
        msg_info = command.substr(begin+1, 5) + "... truncate ? (y/n)" ;
        char ans = log_warn(LONGQUANT) ;
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
std::string parse_name (int begin, int len) {
    if (len-1 > str_maxlen) {
        msg_info = command.substr(begin+1, 5) + "... truncate ? (y/n)" ;
        char ans = log_warn(LONGNAME) ;
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
        msg_info = command.substr(0, 5) + "... truncate ? (y/n)" ;
        //std::cout << msg_info << "\n" ;
        char ans = log_warn(LONGCMD) ;
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
            log_err(UNKCHR) ;
            exec.push_back(ABORT) ;
        } else if (command[tokens[i].beg] == '\'') {
            auto name = parse_name(tokens[i].beg, tokens[i].len) ;
            if (name.length() == 0) {
                msg_info = "Process aborted : no name specified" ;
                log_err(PARSE) ;
                exec.push_back(ABORT) ;
            } else if (nameset) {
                msg_info = "Overwrite ? (y/n)" ;
                char ans = log_warn(RENAME) ;
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
            int n = parse_int(command, tokens[i].beg, tokens[i].len) ;
            if (n == -1) {
                msg_info = command.substr(tokens[i].beg, 5) + "... not a valid quantifier" ;
                log_err(PARSE) ;
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
                msg_info = atom.substr(0, std::min(5, (int)atom.length())) + (atom.length() > 5 ? "..." : "") ;
                log_err(NOSUCHKW) ;
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
        case NIL: curr_lspage = -1 ; print_nil_help() ; break ;
        case MAKE: curr_lspage = 0 ; print_ls_make() ; break ;
        case MAP: curr_lspage = 0 ; print_ls_map() ; break ;
        case REC: curr_lspage = -1 ; adjust_focus() ; preview_redraw() ; break ;
        case LOAD: curr_lspage = 0 ; print_ls_load() ; break ;
        case SAVE: curr_lspage = -1 ; print_save_help() ; break ;
    }
}


int execute () {
    cmd scope_restore = (cmd)curr_scope ;
    int idx = 0 ;
    for (;;) {
        if (idx >= exec.size()) {
            msg_info = "Nothing left to do" ;
            log_info(EMPTY) ;
            goto end ;
        }
        switch (exec[idx]) {
            case SCOPE:
                if (idx+1 >= exec.size()) {
                    msg_info = "No scope specified" ;
                    log_err(PARSE) ;
                    goto end ;
                }
                switch (exec[idx+1]) {
                    case HELP:
                        print_scope_help() ;
                        msg_info = "Terminate" ;
                        log_info(DONE) ;
                        goto end ;
                    case NIL: case REC: case MAP: case MAKE: case LOAD: case SAVE:
                        msg_info = "Switching to " + kw[exec[idx+1]] ;
                        log_info(NEWSCOPE) ;
                        curr_scope = exec[idx+1] ;
                        msg_info = "Terminate" ;
                        log_info(DONE) ;
                        scope_enter_action(exec[idx+1]) ;
                        return 1 ;
                    case RESET:
                        msg_info = "Reset scope : NIL" ;
                        log_info(NEWSCOPE) ;
                        curr_scope = NIL ;
                        msg_info = "Terminate" ;
                        log_info(DONE) ;
                        return 1 ;
                    default:
                        msg_info = kw[exec[idx+1]]  + " not expected after keyword scope" ;
                        log_err(PARSE) ;
                        goto end ;
                }
            case NIL: case REC: case MAP: case MAKE: case LOAD: case SAVE:
                msg_info = "Temporary scope change" ;
                log_info(NEWSCOPE) ;
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
                        print_ls_map() ;
                        msg_info = "Terminate" ;
                        log_info(SIGLS) ;
                        log_info(DONE) ;
                        goto keepls ;
                    case LOAD:
                        if (idx+1 < exec.size() && exec[idx+1]==NUM) {
                            curr_lspage = exec[idx+2] ;
                        } else {
                            curr_lspage = 0 ;
                        }
                        print_ls_load() ;
                        msg_info = "Terminate" ;
                        log_info(SIGLS) ;
                        log_info(DONE) ;
                        goto keepls ;
                    case SAVE:
                        if (idx+1 < exec.size() && exec[idx+1]==NUM) {
                            curr_lspage = exec[idx+2] ;
                        } else {
                            curr_lspage = 0 ;
                        }
                        print_ls_save() ;
                        msg_info = "Terminate" ;
                        log_info(SIGLS) ;
                        log_info(DONE) ;
                        goto keepls ;
                    case MAKE:
                        if (idx+1 < exec.size() && exec[idx+1]==NUM) {
                            curr_lspage = exec[idx+2] ;
                        } else {
                            curr_lspage = 0 ;
                        }
                        print_ls_make() ;
                        msg_info = "Terminate" ;
                        log_info(SIGLS) ;
                        log_info(DONE) ;
                        goto keepls ;
                    case REC:
                        print_ls_rec() ;
                        msg_info = "Terminate" ;
                        log_info(SIGLS) ;
                        log_info(DONE) ;
                        goto end ;
                    default:
                        msg_info = "LS not expected in current scope" ;
                        log_err(PARSE) ;
                        goto end ;
                }
            case HELP:
                switch (curr_scope) {
                    case MAP:
                        print_map_help() ; break ;
                    case SAVE:
                        print_save_help() ; break ;
                    case LOAD:
                        print_load_help() ; break ;
                    case REC:
                        print_rec_help() ; break ;
                    case MAKE:
                        print_make_help() ; break ;
                    case NIL:
                        print_nil_help() ; break ;
                    }
                msg_info = "Terminate" ;
                log_info(SIGHELP) ;
                log_info(DONE) ;
                goto end ;
            case RESET:
                switch (curr_scope) {
                    case MAP:
                        reset_map() ;
                        scope_enter_action(MAP) ;
                        break ;
                    case REC:
                        reset_rec() ;
                        scope_enter_action(REC) ;
                        break ;
                    case MAKE:
                        reset_make() ;
                        scope_enter_action(MAKE) ;
                        break ;
                    case NIL:
                        reset_nil() ;
                        scope_enter_action(NIL) ;
                        break ;
                    default:
                        msg_info = "No reset for scope " + kw[curr_scope] ;
                        log_err(PARSE) ;
                        goto end ;
                }
                msg_info = "Terminate" ;
                log_info(SIGRESET) ;
                log_info(DONE) ;
                goto end ;
            case HASH:
                switch (curr_scope) {
                    case SAVE:
                        hash_name() ;
                        output_save() ;
                        goto end ;
                    case MAKE:
                        hash_name() ;
                        make_image() ;
                        goto end ;
                    default:
                        msg_info = kw[HASH] + " not expected in scope" + kw[curr_scope] ;
                        log_err(PARSE) ;
                        goto end ;
                }
            case NEXT:
                if (curr_lspage == -1) {
                    msg_info = "No ls page displayed" ;
                    log_err(PARSE) ;
                    goto end ;
                }
                curr_lspage++ ;
                switch (ls_scope) {
                    case MAP:
                        print_ls_map() ;
                        break ;
                    case LOAD:
                        print_ls_load() ;
                        break ;
                    case SAVE:
                        print_ls_save() ;
                        break ;
                    case MAKE:
                        print_ls_make() ;
                        break ;
                    case REC:
                        print_ls_rec() ;
                        break ;
                }
                msg_info = "Terminate" ;
                log_info(DONE) ;
                goto keepls ;
            case LSIDE: case RSIDE: case USIDE: case DSIDE: case HSIDE: case ASIDE:
            case VSIDE: case ZOOMIN: case ZOOMOUT: case LSHIFT: case RSHIFT: case USHIFT: case DSHIFT:
                {
                if (curr_scope != REC) {
                    msg_info = "View specifications not expected here" ;
                    log_err(PARSE) ;
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
                adjust_focus() ;
                if (chL.active && chL.indicator != -1) change_focus(LSIDE, chL.indicator, chL.quantifier) ;
                if (chR.active && chR.indicator != -1) change_focus(RSIDE, chR.indicator, chR.quantifier) ;
                if (chU.active && chU.indicator != -1) change_focus(USIDE, chU.indicator, chU.quantifier) ;
                if (chD.active && chD.indicator != -1) change_focus(DSIDE, chD.indicator, chD.quantifier) ;
                adjust_focus() ;
                preview_redraw() ;
                msg_info = "Terminate" ;
                log_info(DONE) ;
                goto end ;
                }
            case NUM:
                switch (curr_scope) {
                    case MAP:
                        set_map(exec[idx+1]) ;
                        msg_info = "Terminate" ;
                        log_info(DONE) ;
                        goto end ;
                    case MAKE:
                        set_resolution(exec[idx+1]) ;
                        idx += 2 ;
                        continue ;
                    case LOAD:
                        input_save(exec[idx+1]) ;
                        msg_info = "Terminate" ;
                        log_info(DONE) ;
                        goto end ;
                    default:
                        msg_info = "Quantifier not expected here" ;
                        log_err(PARSE) ;
                        goto end ;
                }
            case STR:
                switch (curr_scope) {
                    case SAVE:
                        input_save() ;
                        msg_info = "Terminate" ;
                        log_info(DONE) ;
                        goto end ;
                    case MAKE:
                        make_image() ;
                        msg_info = "Terminate" ;
                        log_info(DONE) ;
                        goto end ;
                    default:
                        msg_info = "String literal not expected here" ;
                        log_err(PARSE) ;
                        goto end ;
                }
                goto end ;
            case EXIT:
                return 0 ;
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
    clear_screen() ;
    // adjust_focus() ;
    //
    // change_focus(LSIDE, ZOOMOUT, 10) ;
    // change_focus(USIDE, ZOOMOUT, 20) ;
    // change_focus(DSIDE, ZOOMOUT, 10) ;
    // change_focus(RSIDE, ZOOMOUT, 10) ;

    adjust_focus() ;
    preview_redraw() ;

    read_ls_map() ;
    set_map(0) ;

    do {
        //db << "OK\n" ;
        make_prompt() ;
        if (hist_log.size() > 0) {
            log_redraw() ;
        }
        getline(std::cin, command) ;
        //std::cout << "read line\n" ;
        //std::cout << "<<" << command << ">>\n" ;
        parse() ;
        //std::cout << command ;
    } while (execute()) ;

    db.close() ;
    clear_screen() ;
    return 0 ;
}
