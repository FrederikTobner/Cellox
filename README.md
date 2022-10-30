# Cellox

[![Build](https://github.com/FrederikTobner/Cellox/actions/workflows/build.yml/badge.svg)](https://github.com/FrederikTobner/Cellox/actions/workflows/build.yml)
[![Test](https://github.com/FrederikTobner/Cellox/actions/workflows/tests.yml/badge.svg)](https://github.com/FrederikTobner/Cellox/actions/workflows/tests.yml)
[![Analyze](https://github.com/FrederikTobner/Cellox/actions/workflows/codeql.yml/badge.svg)](https://github.com/FrederikTobner/Cellox/actions/workflows/codeql.yml)
![Lines of code](https://img.shields.io/tokei/lines/github/FrederikTobner/Cellox)
![GitHub repo size](https://img.shields.io/github/repo-size/FrederikTobner/Cellox)

Interpreter based on the book [Crafting interpreters](https://craftinginterpreters.com/contents.html) for the programming language cellox.

Cellox is a scripting language based on the programming language [lox](https://craftinginterpreters.com/the-lox-language.html) from Robert Nystrom.

## Overview

Cellox is a dynamically typed, object oriented, high-level scripting language.

Cellox is currently in an experimental state. Some of the language features that are currently included (especially native functions), might change or not be include in the comming versions of the interpreter.

## Data Types

In cellox there exist only 4 different data types.

* Booleans
* Numbers, these are always 64bit floating point numbers
* Undefiened, the type of null
* a cellox object e.g. a string or a class instance

The define a new variable the var keyword is used and the type is infered.

```
var x = 3.14;
```

The name of a variable must consists out of amixture of characters from a to z lower or upper case or a number or a underscores.

But like in other languages it can not start with a number.

## Control flow

Cellox offers the following control flow structures:

* while loops
* for loops
* if statements
* else if statements
* else statements

## Operators

Cellox features the folowing operators

* \*
* \+
* /
* \-
* \*\*
* %
* and / &&
* or // ||
* []
* .

## Objects

The following types are considered to be an object in cellox

* classes
* class instances
* functions
* a method that is bound to a class instance
* native functions
* strings
* upvalues

All these objects are first class in cellox meaning they are real values that you can get a reference to.

## Functions

A function call expression looks in cellox the same as it does in C.

```
foo(1);
```

Function definitions in cellox use the fun keyword and can return different types by the same function. A practical example wourld be a function that returns a number, or a boolean.

```
fun foo(number)
{
    if(a >= 0) return false;
    return number ** 0.5; // Returns the square root the number
}
```

Function declerations are statements, which gives you the ability to declare a local function inside another function.

## Classes

Cellox is a objectoriented language that features inheritance and methods that are bound to a class instance.

A class is defiened using the class keyword followed by an optional parent class that is prefixed with a double dot, like in C++ or C#.

```
class foo : bar
{
   
}
```

A constructor in cellox is called initializer and must be declared as a method inside the class.

The initializer method that initializes must be called init.

To refer to a method or a field of a cellox instance the this keyword followd by a dot is used.

```
class Point
{
    init(x, y)
    {
        this.x = x;
        this.y = y;
    }   
}
```

An instance of this class is created like this:

```
var point = Point(1, 2);
```

The super keyword is used to refer to a method of the parent class.
```
class bar
{
   init(x)
   {
        this.x = x;
   }
   print_foo()
   {
        print this.x;
   }
}
class foo : bar
{
   init(x, y)
   {
        super(x);
        this.y = y;
   }
   print_bar()
   {
        super.print_foo()
        print this.y;
   }
}
```
The fields of the parent class can be accessed by using the this keyword like the field would belong to the child.

Class instances are by default serialized in a format that resembles json, when printed.
```
print foo(x, y);
```

## IDE Integration

Cellox offers an extension for [vscode](https://github.com/FrederikTobner/vscode-cellox) that provides syntax highlighting and code snippets.

## How it works

The language provides automatic memory management using it's own [garbage collector](https://github.com/FrederikTobner/Cellox/wiki/Garbage-Collector), that uses the mark-and-sweep algorithm.

The variables defiened in a cellox program are stored in a hashtable. The variable name is used as the key for the value is the hashtable.

A cellox program is converted into bytecode and executed by a stack based [virtual machine](https://github.com/FrederikTobner/Cellox/wiki/Virtual-Machine).

## Dependencies

Cellox is written in C, using the C99 standard and only depends on the C standard libary.

CMake is used for building within the whole project.

## Testing

The test are written in C++ using the [google-test framework](https://github.com/google/googletest).

For testing a cellox program is executed and the standard output redirected to a string. The string is compared with the expected output after the program was executed.
