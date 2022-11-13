# Cellox

[![Build](https://github.com/FrederikTobner/Cellox/actions/workflows/build.yml/badge.svg)](https://github.com/FrederikTobner/Cellox/actions/workflows/build.yml)
[![Test](https://github.com/FrederikTobner/Cellox/actions/workflows/tests.yml/badge.svg)](https://github.com/FrederikTobner/Cellox/actions/workflows/tests.yml)
[![Analyze](https://github.com/FrederikTobner/Cellox/actions/workflows/codeql.yml/badge.svg)](https://github.com/FrederikTobner/Cellox/actions/workflows/codeql.yml)

Interpreter based on the book [Crafting interpreters](https://craftinginterpreters.com/contents.html) for the programming language cellox.

Cellox is a scripting language based on the programming language [lox](https://craftinginterpreters.com/the-lox-language.html) from Robert Nystrom.

## Overview

Cellox is a dynamically typed, object oriented, high-level scripting language.

Cellox is currently in an experimental state. Some of the language features that are currently included (especially native functions), might change or not be include in the upcoming versions of the interpreter.

## Values

In cellox [values](https://github.com/FrederikTobner/Cellox/wiki/Values) are grouped into four different types:

* booleans,
* numbers,
* undefiened (null)
* and [cellox objects](https://github.com/FrederikTobner/Cellox#objects) (e.g. a string or a class instance)

## Control structures

Cellox offers the following control structures:

* Conditional flow structures, with [if/else statements](https://github.com/FrederikTobner/Cellox/wiki/if-else-statements)
* Repetitive flow structures, with [for](https://github.com/FrederikTobner/Cellox/wiki/For) and [while](https://github.com/FrederikTobner/Cellox/wiki/While) loops

## Operators

Cellox features [assignment](https://github.com/FrederikTobner/Cellox/wiki/Operators#assignment-operators), [binary](https://github.com/FrederikTobner/Cellox/wiki/Operators#binary-operators), [logical](https://github.com/FrederikTobner/Cellox/wiki/Operators#logical-operators) and [unary](https://github.com/FrederikTobner/Cellox/wiki/Operators#unary-operators) operators.

## Objects

In Cellox everything besides the 3 base data types is considered to be a cellox [object](https://github.com/FrederikTobner/Cellox/wiki/Objects).
This means that you can for example get the reference to a function and assign it to a variable.

## Functions

A [Function](https://github.com/FrederikTobner/Cellox/wiki/Functions) in Cellox is a group of statements, that together perform a task.
Cellox also offers some [native functions](https://github.com/FrederikTobner/Cellox/wiki/Native-Functions) that are implemented in C.

## Classes

Cellox is a objectoriented language that features inheritance and methods that are bound to a [class](https://github.com/FrederikTobner/Cellox/wiki/Classes) instance.

## Strings

A string in cellox is a special type of object.
Strings can contain escape sequences that will be resolved at compile time.
The characters that a string contains can be accessed and altered by the index.

## IDE Integration

Cellox offers an extension for [vscode](https://github.com/FrederikTobner/vscode-cellox) that provides syntax highlighting and code snippets.

## How it works

The language provides automatic memory management using it's own [garbage collector](https://github.com/FrederikTobner/Cellox/wiki/Garbage-Collector), that uses the mark-and-sweep algorithm.

The variables defiened in a cellox program are stored in a hashtable. The variable name is used as the key for the value stored in the hashtable.

A cellox program is converted into bytecode and executed by a stack based [virtual machine](https://github.com/FrederikTobner/Cellox/wiki/Virtual-Machine).

## Dependencies

Cellox is written in C, using the C99 standard and [CMake](https://cmake.org/) is used for building within the whole project.

Under windows:

* conio.h
* windows.h

Under linux:

* curses.h
* unistd.h

## Testing

The test are written in C++ using the [google-test framework](https://github.com/google/googletest) (version 1.12.1).

For testing a cellox program is executed and the standard output is redirected to a string. The string is compared with the expected output after the program was executed.

A small benchmarking suite to measure the performance of cellox programs is also included.
