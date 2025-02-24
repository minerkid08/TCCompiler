# A simple compiler for the symphony architecture from Turing Complete Game
## Building
A 'build.sh' script is provided that builds everything with gcc. Any c build system would probably work but it is untested.
## Usage
The compiler builds a modified version of the lua language.
Some of the big changes are:
<li>Every line of code must end with a semicolon</li>
<li>Functions can only return one value and it uses a different syntax: `foo() -> x;`</li>
<li>The not equal operator has been changed to `!=` instead of `~=`</li>
<li>Functions have to be defined before they are used, they can be forward declared with `function foo() end`</li>
To build a file you run the compiler program with the output file as the first argument and the input as the second.
The linking step is not needed but it validates that all functions are defined and adds `call main` to the start of the program.

## Notes
Loops are currently not supported.<br>
This is early in development so expect things to break.
