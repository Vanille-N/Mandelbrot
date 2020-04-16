# Mandelbrot

C++ script focused on creating images of the Mandelbrot set using a language designed for easy navigation within the Mandelbrot set and many tools for choosing color palettes, image dimensions, and other metaparameters.

Disclaimer: This project relies heavily on ANSI escape codes for interactive rendering. Correct execution has not been tested anywhere other than the default Terminal for Ubuntu 18.04.

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

I will just add a short note:

Due to the design of the language, when inside scope `{scope1}`,
```
cmd> scope {scope2}
cmd> {cmd1}
cmd> scope {scope1}
```
is absolutely equivalent to
```
cmd> {scope2} {cmd1}
```
for any `{cmd1}` that is valid within `{scope2}`.

In particular, since `{scope2} {cmd2}` is a valid command, then the following command is absolutely valid:
```
cmd> make nil load save map rec A+ :50
```
and would be translated to
```
cmd> scope make
cmd> scope nil
cmd> scope load
cmd> scope save
cmd> scope map
cmd> scope rec
cmd> A+ :50
cmd> scope make
```
It is perfectly equivalent to `rec A+ :50` and there is no limit other than the arbitrary 50-character limit for commands.
