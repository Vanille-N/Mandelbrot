# Grammar

*As presented in the interactive help pages.*

Meta characters: `{}()*`

None of these can appear in an actual command.

Any expression surrounded by other content in the form `prefix{expr}suffix` is to be understood as a single string of characters where all of `{expr}` has been replaced with a valid instance of `expr`.

Examples:
- `scope {scope}` can be `scope nil` or `scope map` or `scope rec` ...
- `'{str}` can become `'abcd` or `'x_y_z` or `'123` ...
- `{scope} {{scope}_cmd}` becomes `nil {nil_cmd}` or `map {map_cmd}` ... which in turn can become `nil ?` or `nil .` ... or `map 10` or `map ls` ...

`{expr}*` is 0 or more of `{expr}`

## Description:

```
universal :=
    | scope .
    | scope {scope}
    | {scope} {{scope}_cmd}
    | ~
    | ~~
    | !
    | =
    |
    | ?
    | ls
    | /
```

```
scope :=
    | nil
    | rec
    | map
    | make
    | save
```

```
nil_cmd :=
    | .
    | #
    | {int}
    | '{str}
    | ls {int}
    | {universal}
```

```
map_cmd :=
    | .
    | {int}
    | ls {int}
    | {universal}
```

```
save_cmd :=
    | #
    | '{str}
    | {int}
    | ls {int}
    | {universal}
```

```
make_cmd :=
    | .
    | #
    | '{str}
    | {int}
    | {int} #
    | {int} '{str}
    | ls {int}
    | {universal}
```

```
rec_cmd :=
    | .
    | ({selec}*{indic}*{int}*)*
    | {universal}
```

```
selec :=
    | L
    | R
    | D
    | U
    | H
    | V
    | A
```

```
indic :=
    | +
    | -
    | <
    | >
    | _
    | ^
```
