# cmm
## Introduction
cmm (c minus minus) is a compiler which compiles the source code written in C-- language(which is a subset of C) into aarch64 assembly code.
It's a project of the introductory compiler course in NTU.

## Informal Specification of C--

C-- is based on ANSI C standard, thus only the differences are list below.

### Types
C-- onlys support five types, which are :
* int - signed integer - 4 bytes
* float - single percision floating pointer number - 4 bytes
* string literal - ascii string terminated with NULL byte, 1 byte for each character
* int [] - address of an int array \(only for function parameter\) - 8 bytes
* float [] - address of an float array \(only for function parameter\) - 8 bytes

### Operators
Only support the following operators:
`+`, `-`, `*`, `/`, `!`, `&&`, `||`, `[]`(array subscription), `()`(function call)

### Syntax
* Does not support any type qualifier.
* Does not support assignment expression as rvalue.
* Entry point is `MAIN` instead of `main`.
* Does not support `switch-case`, `do-while`, `break`, `continue`, `goto`.
* Function declaration and definition should not be seperated.
* All declarations should be placed in the front of the block.
* Does not support array initialization.
* No explicit type conversion, but implicity conversion between int and float is allowed.
* Max dimension of array is 10.
* `for`, `while`, `if`, `else` should all followed by a block (`{`, `}`)

### IO functions
There are five functions provided for IO operations:
* `void write(int)`
    print the argument (int) to the standard output.
* `void write(float)`
    print the argument (float) to the standard output.
* `void write(string literal)`
    print the argument (string literal) to the standard output.
* `int read()`
    read and return an integer value from standard inupt.
* `float fread()`
    read and return a floating point value from standard input.

## Application Binary Interface
The cmm ABI is designed roughly based on the Aarch64 ABI, with some slightly diffences:
* Parameter passing using stack instead of register, only write() will use `s0`, `x0`, `w0`.
* compiler has implement the caller/callee register category.

### Stack frame layout
```
|  ...                |
|---------------------|
|  old frame pointer  | <- old x29
|  local variables    |
|  callee saved regs  |
|  caller saved regs  |
|  function args      |
|  return address     |
|---------------------|
|  old frame pointer  | <- x29
|  ...                |
```

## Build
Run
```
$ make
```

## Usage
Run
```
$ ./parser input_file
```
the output file we be named output.s, which contains the aarch64 assembly.

`output.s` isn't self contained, it has to be compiled with `main.S`, which is a wrapper file provding IO rountines and entry loading for the compiled file.

So, then use
```
$ aarch64-linux-gnu-gdb -static main.S
```
to compiler the program, it will pulled in `output.s` and produce an aarch64 binary, then you may test the static binary with qemu-aarch64.
