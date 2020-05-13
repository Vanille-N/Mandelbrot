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

