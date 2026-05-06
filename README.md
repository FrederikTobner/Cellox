# Cellox

[![Build Interpreter](https://github.com/FrederikTobner/Cellox/actions/workflows/build_compiler.yml/badge.svg)](https://github.com/FrederikTobner/Cellox/actions/workflows/build_compiler.yml)
[![Build Tools](https://github.com/FrederikTobner/Cellox/actions/workflows/build_tools.yml/badge.svg)](https://github.com/FrederikTobner/Cellox/actions/workflows/build_tools.yml)
[![Test](https://github.com/FrederikTobner/Cellox/actions/workflows/tests.yml/badge.svg)](https://github.com/FrederikTobner/Cellox/actions/workflows/tests.yml)
[![Coverage](https://codecov.io/gh/FrederikTobner/Cellox/graph/badge.svg)](https://codecov.io/gh/FrederikTobner/Cellox)
[![Analyze](https://github.com/FrederikTobner/Cellox/actions/workflows/codeql.yml/badge.svg)](https://github.com/FrederikTobner/Cellox/actions/workflows/codeql.yml)
[![Sanitizers](https://img.shields.io/badge/sanitizers-ASan%20%2B%20UBSan-0A7A5C)](https://github.com/FrederikTobner/Cellox/actions/workflows/tests.yml)
[![CTest Labels](https://img.shields.io/badge/ctest-unit%20%7C%20integration%20%7C%20e2e%20%7C%20fuzz-1F6FEB)](https://github.com/FrederikTobner/Cellox/actions/workflows/tests.yml)
[![CI Matrix](https://img.shields.io/badge/ci%20matrix-windows%20%7C%20linux%20%7C%20macOS-7A3EC8)](https://github.com/FrederikTobner/Cellox/actions/workflows/tests.yml)

Bytecode compiler and interpreter based on the book [Crafting Interpreters](https://craftinginterpreters.com/contents.html) for the programming language Cellox.

Cellox is a programming language based on [lox](https://craftinginterpreters.com/the-lox-language.html) from Robert Nystrom.

## Table of Contents

* [Overview](#overview)
* [Building](#building)
* [Usage](#usage)
* [Values](#values)
* [Control structures](#control-structures)
* [Operators](#operators)
* [Objects](#objects)
* [Functions](#functions)
* [Classes](#classes)
* [Modules](#modules)
* [Error handling](#error-handling)
* [Strings](#strings)
* [Arrays](#arrays)
* [Slices](#slices)
* [Benchmarking](#benchmarking)
* [IDE Integration](#ide-integration)
* [How it works](#how-it-works)
* [License](#license)

## Overview

Cellox is a dynamically typed, object-oriented, high-level scripting language.

It is available on Windows, Linux, and macOS. The language is in active development; some features (especially native functions) may change in upcoming releases.

## Building

Prerequisites: CMake ≥ 3.16, a C99 compiler, and a C++14 compiler (for the test suite).

```sh
cmake -S . -B build
cmake --build build
```

To also build the tests:

```sh
cmake -S . -B build -DCLX_BUILD_TESTS=ON
cmake --build build
ctest --test-dir build
```

The resulting interpreter binary is `build/src/Cellox` (Linux/macOS) or `build\src\Cellox.exe` (Windows).

## Usage

```
Cellox ((-h|--help|-v|--version) | ([-c|--compile] [-O0|-O1|-O2|-O3] [--stdlib-dir <path>] [path]))
```

| Flag | Description |
|------|-------------|
| `-h`, `--help` | Print usage information. |
| `-v`, `--version` | Print the version. |
| `-c`, `--compile` | Compile to a `.cxcf` bytecode file instead of executing. |
| `-O0` … `-O3` | Set the bytecode optimisation level (default: `-O0`). |
| `--stdlib-dir <path>` | Override the standard library directory used to resolve bare module imports. |

The standard library directory is resolved in the following order:

1. `--stdlib-dir <path>` command-line flag.
2. `CELLOX_STDLIB_DIR` environment variable.
3. A `stdlib/` directory next to the interpreter executable.
4. The path baked in at build time (`CLX_STDLIB_PATH`).

## Values

In cellox [values](https://github.com/FrederikTobner/Cellox/wiki/Values) are grouped into four different types:

* booleans,
* numbers,
* undefined (null)
* and [cellox objects](https://github.com/FrederikTobner/Cellox#objects) (e.g. a string or a class instance)

## Control structures

The language provides the following control structures:

* Conditional flow structures, with [if/else statements](https://github.com/FrederikTobner/Cellox/wiki/if-else-statements)
* Repetitive flow structures, with [for](https://github.com/FrederikTobner/Cellox/wiki/For) and [while](https://github.com/FrederikTobner/Cellox/wiki/While) loops

## Operators

Cellox features [assignment](https://github.com/FrederikTobner/Cellox/wiki/Operators#assignment-operators), [binary](https://github.com/FrederikTobner/Cellox/wiki/Operators#binary-operators), [logical](https://github.com/FrederikTobner/Cellox/wiki/Operators#logical-operators) and [unary](https://github.com/FrederikTobner/Cellox/wiki/Operators#unary-operators) operators.

## Objects

In Cellox everything besides the three base data types is considered to be a cellox [object](https://github.com/FrederikTobner/Cellox/wiki/Objects).

Even functions and classes are considered to be a cellox object.

This means that you can for example get the reference to a function and assign it to a variable.

## Functions

A [Function](https://github.com/FrederikTobner/Cellox/wiki/Functions) in Cellox is a group of statements, that together perform a task.

Some functions in Cellox also access the enclosing environment of the function to for example change the value stored in variable in the enclosing environment.

These functions are called closures and the values that are accessible for the functions are called upvalues.

Cellox also offers some [native functions](https://github.com/FrederikTobner/Cellox/wiki/Native-Functions) that are implemented in C.

## Classes

Cellox is an object-oriented language that features inheritance and methods bound to a [class](https://github.com/FrederikTobner/Cellox/wiki/Classes) instance.

Classes can also extend the functionality of an already existing class by using inheritance.

## Modules

Cellox supports file-based [modules](https://github.com/FrederikTobner/Cellox/wiki/Modules) with relative imports, named imports, explicit exports, load-once semantics, and diagnostics for cycles or invalid imports.

## Error handling

Cellox provides explicit [error handling](https://github.com/FrederikTobner/Cellox/wiki/Error-Handling) with named error sets, propagation (`try`), recovery (`catch`), strict unwrapping (`must`), branching (`iferror`), and structured stdlib errors.

## Strings

A [string](https://github.com/FrederikTobner/Cellox/wiki/Strings) in cellox is a special type of object.

Strings can contain escape sequences that will be resolved at compile time.

The characters that a string contains can be accessed by the index.

## Arrays

[Arrays](https://github.com/FrederikTobner/Cellox/wiki/Arrays) have a variable-size, meaning they can shrink and grow.

There is no tradional array, with a fixed capacity that is specified at allocation.

## Slices

A [slice](https://github.com/FrederikTobner/Cellox/wiki/Slices) is a subset of an already existing array or string.

Slices are created by using the range operator.

The values stored in slice can be altered without affecting the original array.

## IDE Integration

There are plugins for VS Code, Vim and Neovim. Another alternative is YATE, which has built-in language support.

* [vscode-cellox](https://github.com/FrederikTobner/vscode-cellox) - Visual Studio Code extension for Cellox
* [cellox.vim](https://github.com/FrederikTobner/cellox.vim) - Vim plugin for Cellox syntax support
* [YATE](https://github.com/FrederikTobner/YATE) - Text editor with built-in Cellox language support

## How it works

The language provides automatic memory management to the programmer using it's own [garbage collector](https://github.com/FrederikTobner/Cellox/wiki/Garbage-Collector), that uses the mark-and-sweep algorithm.

The variables defined in a cellox program are stored in a hash table. The variable name is used as the key for the value stored in the hash table.

Source files are resolved into a single compilation unit by the module loader, which handles bare imports (e.g. `import { abs } from "stdlib/math.clx"`) by searching the standard library directory, and relative imports by resolving paths from the importing file.

The program is converted into bytecode and executed by a stack-based [virtual machine](https://github.com/FrederikTobner/Cellox/wiki/Virtual-Machine).

The bytecode can also be stored in a [separate file](https://github.com/FrederikTobner/Cellox/wiki/Chunk-Files) in order to be executed at a later point in time.

More information about the compiler can be found at the [technical documentation](https://frederiktobner.github.io/Cellox/).

## License

This project is licensed under the [GNU General Public License](LICENSE)
