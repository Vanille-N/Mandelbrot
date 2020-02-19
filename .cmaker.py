#!/usr/bin/python3
### Colormap creator ###
from math import floor, ceil

DEBUG = False

def verbose(f, legend):
    if DEBUG:
        def wrapper(*args):
            r = f(*args)
            print("[{} --({})--> {}]".format(args, legend, r))
            return r
        return wrapper
    else:
        return f

floor = verbose(floor, "floor")
ceil = verbose(ceil, "ceil")

linelen = 10
maplen = 4
totlen = maplen * linelen

# Pad to 3
def lpad(s):
    return "0"*(max(0, 3-len(s))) + s

def lpadmap(c):
    return [lpad(str(ci)) for ci in c]

def disp(c):
    print("{} {} {} ".format(*lpadmap(c)), end="")

# One color for the set, one for the rest
def printmap(C):
    for i in range(maplen):
        for j in range(linelen):
            disp(C[i*linelen + j])
        print()
    print()

def frac(p, c):
    return tuple(p*ci for ci in c)
frac = verbose(frac, "%")

def rgbval(v):
    return min(255, max(0, int(v)))

def add(*C):
    return tuple(rgbval(sum(c[i] for c in C)) for i in range(3))
add = verbose(add, "+")

# Takes a spread and a certain number of colors and return the mean of all
def mix(spread, *C):
    return add(*[frac(p, c) for p,c in zip(spread, C)])
mix = verbose(mix, "#")

def gradient(n, c):
    def mapspread(f, *C):
        return [(mix(f(k/(totlen-1)), *C) if k%n==0 else c) for k in range(totlen)]
    return mapspread

dirac = lambda p: [floor(1-p), ceil(p)]
power = lambda a: (lambda p: [(1-p)**a, 1-(1-p)**a])
linear = power(1)
quadratic = power(2)
cubic = power(3)
constant = power(0)
three_way = lambda p: [max(0, 1-2*p), 1-abs(2*p-1), max(0, 2*p-1)]
delta_power = lambda a: (lambda p: [1, 0, 0] if p == 0 else [0, (1-p)**a, 1-(1-p)**a])
delta_linear = delta_power(1)
delta_three_way = lambda p: [1, 0, 0, 0] if p == 0 else [0, max(0, 1-2*p), 1-abs(2*p-1), max(0, 2*p-1)]
n_way = lambda n: (lambda p: [max(0, 1-abs((n-1)*p - i)) for i in range(n)])

### Standard colors
BLU = (0, 102, 255)
RED = (255, 0, 0)
GRN = (0, 230, 0)
BLK = (0, 0, 0)
GRY = (128, 128, 128)
WHT = (255, 255, 255)
YLW = (255, 255, 0)
ORG = (255, 153, 0)
PPL = (179, 0, 179)
lBLU = (204, 224, 255)
lRED = (255, 153, 153)
lGRN = (204, 255, 51)
lYLW = (255, 255, 153)
lORG = (255, 204, 128)
lPPL = (255, 51, 204)
dBLU = (0, 20, 51)
dRED = (102, 0, 0)
dGRN = (0, 51, 0)
dYLW = (102, 102, 0)
dORG = (128, 77, 0)
dPPL = (77, 0, 57)

def make(n=1, c=BLK):
    def makemap(f, *C):
        printmap(gradient(n, c)(f, *C))
    return makemap

print(46)








make()(dirac, WHT, BLK)
make()(dirac, BLK, WHT)
make(2, WHT)(constant, BLK)
make(2, BLK)(constant, WHT)
make()(linear, WHT, BLK)
make()(linear, WHT, BLU)
make()(linear, WHT, RED)
make()(linear, WHT, GRN)
make()(linear, WHT, ORG)
make()(linear, WHT, PPL)
make()(linear, WHT, YLW)
make()(delta_linear, WHT, BLU, BLK)
make()(delta_linear, WHT, RED, BLK)
make()(delta_linear, WHT, GRN, BLK)
make()(delta_linear, WHT, ORG, BLK)
make()(delta_linear, WHT, PPL, BLK)
make()(delta_linear, WHT, YLW, BLK)
make()(three_way, WHT, BLU, BLK)
make()(three_way, WHT, RED, BLK)
make()(three_way, WHT, GRN, BLK)
make()(three_way, WHT, ORG, BLK)
make()(three_way, WHT, PPL, BLK)
make()(three_way, WHT, YLW, BLK)
make(2, BLK)(linear, WHT, BLK)
make(2, BLK)(linear, WHT, BLU)
make(2, BLK)(linear, WHT, RED)
make(2, BLK)(linear, WHT, GRN)
make(2, BLK)(linear, WHT, ORG)
make(2, BLK)(linear, WHT, PPL)
make(2, BLK)(linear, WHT, YLW)
make(2, BLK)(delta_linear, WHT, BLU, BLK)
make(2, BLK)(delta_linear, WHT, RED, BLK)
make(2, BLK)(delta_linear, WHT, GRN, BLK)
make(2, BLK)(delta_linear, WHT, ORG, BLK)
make(2, BLK)(delta_linear, WHT, PPL, BLK)
make(2, BLK)(delta_linear, WHT, YLW, BLK)
make(2, BLK)(three_way, WHT, BLU, BLK)
make(2, BLK)(three_way, WHT, RED, BLK)
make(2, BLK)(three_way, WHT, GRN, BLK)
make(2, BLK)(three_way, WHT, ORG, BLK)
make(2, BLK)(three_way, WHT, PPL, BLK)
make(2, BLK)(three_way, WHT, YLW, BLK)
make()(delta_three_way, WHT, YLW, ORG, RED)
make()(delta_three_way, WHT, YLW, GRN, dGRN)
make()(delta_three_way, lPPL, PPL, BLU, RED)
make()(n_way(6), WHT, YLW, GRN, BLU, PPL, RED)
