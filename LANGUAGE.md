# Mandelbrot

An overview of the navigation language.

## Scopes

The language was designed to minimize the amount of typing required. This was done by building all of the language around the notion of scopes.
When in a scope, all local commands are accessible directly, and commands from other scopes are available using the universal scope an an intermediary, at the cost of having to specify the name of the target scope.
This has the advantage of allowing us to heavily overload all commands depending on the scope, making it easier for both the language parser and the user.

### List of scopes

The explicit scopes are:
- `nil`, the default scope, can manage i/o of meta-parameters
- `rec`, to fine-tune the frame
- `map`, to choose the color palette
- `make`, to output an image
- `save`, to manage i/o of direct parameters

In addition there exists the `universal` scope, accessible from all other.

### Switching scope

One may change the current scope using the `scope` keyword.
```
(launch program)
(inside scope nil)     cmd> scope map
(inside scope map)     cmd> scope nil
(back to scope nil)    cmd>
```

Anything after `scope` that is not a valid (and explicit, i.e. not `universal`) scope results in an error.

### Accessing other scopes

If one wishes to call a single command from another scope, it can be cumbersome to change scopes.

Instead of
```
cmd> scope map
cmd> 1
cmd> scope nil
```
it is possible to write
```
cmd> map 1
```


More generally, when inside scope `{scope1}`,
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

#### Side note:

Since `{scope3} {cmd2}` is a valid command for any `{cmd2}` that is valid within `{scope3}`, then the following command is absolutely valid:
```
cmd> make nil save map rec A+50
```
and could be translated to (assuming current scope is nil)
```
cmd> scope make
cmd> scope nil
cmd> scope save
cmd> scope map
cmd> scope rec
cmd> A+50
cmd> scope nil
```
It is perfectly equivalent to `rec A+50`.

Note that the above is accurate in the sense that the command in question would specifically NOT be translated to
```
cmd> scope make
cmd> scope nil
cmd> scope save
cmd> scope map
cmd> scope rec
cmd> A+ :50
cmd> scope rec
cmd> scope map
cmd> scope save
cmd> scope nil
cmd> scope make
cmd> scope nil
```
even though it would have the same effect.
The temporary scope changes are implemented sequentially and not recursively, so the scope is indeed changed back to `nil` directly after the end of the command has been reached.


### Universal scope

These expressions are valid anywhere:
```
universal :=
    | scope {scope}
    | {scope} {{scope}_cmd}
    | ~
    |
    | ?
    | ls
    | /
```

That is not to say they will execute successfully: for example if no `ls` page is displayed, `/` is valid but results in an `ERROR: Runtime exception`.

#### `scope {scope}`
We've already seen this one: current scope can be changed from anywhere.

#### `{scope} {{scope}_cmd}`
By specifying the name of the target scope, one can access any command.

#### `~`
Quit (asks for y/n confirmation)

#### ` `
The empty command is always valid

#### `?`
Unlike all of the above, `?` executes differently depending on the current scope.
All scopes provide a `{scope}_help_print()` function that is called when `?` is encountered.

#### `ls`
If the current scope allows `ls {int}`, then `ls` translates to `ls 0`.
In the case of `rec`, `ls` displays the current frame settings (resolution, zone shown)

#### `/`
This one executes the same under all scopes, but depends on the environment.
`/` can only ask to display the *next* page of `ls`. If no `ls` page is currently being displayed it will, as mentioned earlier, result in an exception.
The scope under which `ls` was called is preserved:
```
cmd> scope map
cmd> save ls         <- displays ls page 0 from save (current scope is map)
cmd> /               <- displays ls page 1 from save (current scope is map)
cmd> ls 3            <- displays ls page 3 from map
cmd> scope rec
cmd> /               <- displays ls page 4 from map (current scope is rec)
```


## Parsing
There are some specific parsing details that are useful to know to use this tool correctly.

### Separators
All non-alphabetic commands are a single character long, so these are easy to parse.

Some characters that don't appear in any keyword stop the parsing immediately:
```
cmd> spope nil             <- ERROR: Not a valid keyword
cmd> stope nil             <- ERROR: Unknown character
```
This is mostly an optimization to increase parsing speed, it has no real effect on the user.
All multi-character keywords are read until the first non-alphabetic character before the validity is checked.

### `0-9`
The characters `0-9` are not valid for keywords, so any sequence of them is interpreted as a quantifier
```
cmd> 10                <- tokenized as {..., NUM, 10, ...}
cmd> 10000000000000    <- WARNING: Quantifier too long
```

### `'`
The `'` symbol indicates the beginning of an identifier, i.e. a string.
No checks are performed concerning the validity of the string as a filename, and it continues until either the end of the command or a space.
```
cmd> 'a                <- tokenized as {..., STR, ...}, "a" is stored in another variable
cmd> '                 <- ERROR: Critical parsing error
cmd> 'a 'b             <- WARNING: Name already specified
cmd> 'aaaaaaaaaaaaaaa  <- WARNING: String literal too long
```

### Whitespace
Tabs are not recognized as valid characters, spaced are allowed everywhere between tokens.
There is no restriction on the maximum amount of spacing in a command.
Spaces are required only to end `'` groups and to separate alphabetic keywords:
```
cmd>     scope     nil    <- no problem
cmd> A+10                 <- spaces are allowed but not required between A/+ and +/10
cmd> ls1                  <- spaces are allowed but not required between ls/1
cmd> make122'a            <- spaces are allowed but not required between make/122 and 122/'a
```

### Character limit
The following are arbitrary character limits:
```c++
static const int num_maxlen = 7 ;
static const int str_maxlen = 20 ;
static const int cmd_maxlen = 50 ;
```
These are quite self-explanatory.
Going over any of these will trigger a `WARNING: {} too long... truncate ? (y/n)`.

The graphical interface was made to support commands longer than 50 characters, but not too much longer: commands over 82 characters will not be fully erased when the interface is refreshed.

### End of a command
The tokenizer will ask to truncate commands over 50 characters, but there is no guarantee that the parser will read until the end. In fact, the parser has full control over when a command ends and there is no way to force it to read more.

Example: `cmd> scope rec :10 #`, the tokenizer returns `{SCOPE, REC, NUM, 10, HASH}`, but the parser will read `SCOPE`, `REC`, then stop there. No warning is raised.


## Commands
Explanations of all commands/keywords/constructs available:

### `{int}` (Quantifier)
Valid in: `nil`, `map`, `save`, `make`

#### `nil {int}`
Load corresponding `.meta` file (may trigger `ERROR: No such file`)

#### `map {int}`
Select corresponding color map (may trigger `ERROR: No such file`)

#### `save {int}`
Load corresponding `.save` file (may trigger `ERROR: No such file`)

#### `make {int}`
Set height for output image

### `'{str}`
Valid in: `nil`, `save`, `make`

#### `nil '{str}`
Save current meta-settings in a new file with given name

#### `save '{str}`
Save current frame in a new file with given name

#### `make '{str}`
Launch calculations to create a new image with given name.

### `#`
Valid in: same as `'{str}`

A new pseudo-random name is created and used as if it were provided by the user.

This is safe to use as it will check that another file with the same name does not already exist.

### `.`
Valid in: `nil`, `map`, `make`, `rec`

#### `nil .`
Full reset: equivalent to
```
cmd> map .
cmd> make .
cmd> rec .
```

#### `map .`
Reset map: equivalent to `cmd> map :0`

#### `make .`
Reset resolution: equivalent to `cmd> make :100`

#### `rec .`
Reset frame to initial values: from -2.5 to 0.5 and from -i to i.

### ls {int}
Valid in: `nil`, `map`, `save`, `make`

Display the given page of:
- `nil` -> list of `.meta` files
- `map` -> list of color maps
- `save` -> list of `.save` files
- `make` -> list of `.ppm` files

Although `ls {int}` seems valid in scope `rec`, that is only because when the scope is rec and the parser reads `LS`, it immediately ends the command as discussed previously.


### `{int} #` and `{int} '{str}`
Valid in: `make`

It is possible to execute both instructions sequentially in a single command: `{int}` will change resolution and `#` or `'{str}` will make a name and create the image immediately afterwards.

Note that `cmd> make {int} '{str}` is equivalent to
```
cmd> make {int}
cmd> make '{str}
```
In particular, the new resolution affects also future images.

`make '{str} {int}` is also accepted, but the image will be created immediately afterwards `'{str}` and `{int} will be ignored.

### `({selec}*{indic}*{int}*)*`
Valid in: `rec`

Used to navigate the image:

`{selec}` is one of
- `L` (left)
- `R` (right)
- `U` (up)
- `D` (down)
- `H` (horizontal)
- `V` (vertical)
- `A` (all)
It determines which sides of the image are affected
Several can be selected at once, modifications will apply to all sides specified.

`{indic} is one of
- `<` (move left)
- `>` (move right)
- `^` (move up)
- `_` (move down)
- `-` (zoom out)
- `+` (zoom in)
It determines in which direction the selected sides are moved and affects all previous sides that do not have a direction yet.
The first takes precedence.

`{int}` indicates the amount by which the selected sides will be moved (translated internally as a proportion of the total size of the frame). Defaults to 1 if absent. Applied to all sides that do not have a quantifier yet.

See `EXAMPLES.pdf` for a somewhat comprehensive list of examples of this mechanism.
