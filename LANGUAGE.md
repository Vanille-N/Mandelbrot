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
cmd> :1
cmd> scope nil
```
it is possible to write
```
cmd> map :1
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
cmd> make nil save map rec A+ :50
```
and could be translated to (assuming current scope is nil)
```
cmd> scope make
cmd> scope nil
cmd> scope save
cmd> scope map
cmd> scope rec
cmd> A+ :50
cmd> scope nil
```
It is perfectly equivalent to `rec A+ :50`.

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
If the current scope allows `ls :{int}`, then `ls` translates to `ls :0`.
In the case of `rec`, `ls` displays the current frame settings (resolution, zone shown)

#### `/`
This one executes the same under all scopes, but depends on the environment.
`/` can only ask to display the *next* page of `ls`. If no `ls` page is currently being displayed it will, as mentioned earlier, result in an exception.
The scope under which `ls` was called is preserved:
```
cmd> scope map
cmd> save ls         <- displays ls page 0 from save (current scope is map)
cmd> /               <- displays ls page 1 from save (current scope is map)
cmd> ls :3           <- displays ls page 3 from map
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

### `:`
The `:` symbol indicates the beginning of a quantifier, i.e. a positive integer.
Checks are performed to verify beforehand that no non-numeric characters are encountered before the end, signaled by a blank space.
```
cmd> :10                <- tokenized as {..., NUM, 10, ...}
cmd> :10000000000000    <- WARNING: Quantifier too long
cmd> :                  <- tokenized as {..., NUM, 0, ...}
cmd> :a                 <- ERROR: Critical parsing error
cmd> :/                 <- ERROR: Critical parsing error
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
Spaces are required only to end `'` and `:` groups and to separate alphabetic keywords:
```
cmd>     scope     nil    <- no problem
cmd> A+:10                <- spaces are allowed but not required between A/+ and +/:10
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


