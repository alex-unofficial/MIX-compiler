# MIX compiler
Implementation of a simple (60s-70s) style compiler for a
subset of C, targeting the *mythical* MIX computer designed by
Donald Knuth in [The Art of Computer Programming](https://www-cs-faculty.stanford.edu/~knuth/taocp.html).

The compiler accepts a *tiny* subset of C, that supports functions,
branching, while loops, arithmetic expressions, comparison operators, etc.
There is no support for pointers or memory management, no other types
beside int, no I/O support (except printing the return value of `main`),
no structs, ...

You can find some examples of accepted and not accepted code snippets 
in the `example` subdirectory.

At this stage the compiler seems to work as intended but lacks polish. 

## Compiling the Compiler
To compile you can simply run
```bash
make
```
in the project directory. This creates the executable `compiler`.

### Requirements
Compilation requires a version of `gcc`, `bison` and `flex` or
any valid alternatives. Assembling the generated MIXAL code
requires the `mdk` toolkit (see below).

## Using the Compiler
To compile a file you can run
```bash
./compiler file.c
```
which prints the generated MIXAL code to `stdout`

Alternatively run
```bash
./compiler infile.c outfile.mixal
```
to output the generated code to `outfile.mixal`
In case of en error, a message is printed indicating what might have gone wrong.

The compiler generates valid [MIXAL](https://www.gnu.org/software/mdk/manual/html_node/MIXAL.html)
code.  It is recommended to use the [GNU MIX Development Kit (mdk)](https://www.gnu.org/software/mdk/mdk.html)
to assemble and run the generated MIXAL code with the provided MIX virtual machine.

for example:
```bash
./compiler foo.c foo.mixal
mixasm foo.mixal
mixvm -r foo.mix
```

## License
Copyright (C) 2025 Alexandros Athanasiadis

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <https://www.gnu.org/licenses/>.
