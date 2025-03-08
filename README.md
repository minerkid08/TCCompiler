# A simple compiler for the symphony architecture from Turing Complete Game
## Building
A `build.sh` script is provided that builds everything with gcc. Any c build system would probably work but it is untested.

## Usage
### Language
The compiler builds a modified version of the lua language.
Some of the big changes are:
<li>Every line of code must end with a semicolon</li>
<li>Functions can only return one value and it uses a different syntax: `foo() -> x;`</li>
<li>The not equal operator has been changed to `!=` instead of `~=`</li>
<li>Functions have to be defined before they are used, they can be forward declared with `function foo() end`</li>

### Compiler
usage: compiler <opts> <in file>
Options:
<li>-a <arch>  (optional) sets target architecture. Current archs are `symphony` and `symphony-ext`</li> 
<li>-o <file>  (required) sets the output filepath. </li>

### Linker
usage: linker <out file> <in files>

### Interperter
usage: interperter <file>

## Notes
This is early in development so expect things to break.
