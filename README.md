# Mandelbrot

C++ script focused on creating images of the Mandelbrot set using a language designed for easy navigation within the Mandelbrot set and many tools for choosing color palettes, image dimensions, and other metaparameters.

Disclaimer: This project relies heavily on ANSI escape codes for interactive rendering. Correct execution has not been tested anywhere other than the default Terminal for Ubuntu 18.04+.

It is recommended to use full screen and reduce the font size for comfort.

How to use:

```
$ g++ -o prog prog.cpp
$ ./prog
```


See `grammar.txt` for an overview of the language. Help can be accessed from within the script, and the explanations inside `.help.txt` are detailed enough that a presentation of the language is unnecessary. Just know that you can use
```
cmd> nil ?
cmd> scope ?
cmd> make ?
cmd> rec ?
cmd> load ?
cmd> save ?
cmd> map ?
```
to get pretty much all the information you need.
