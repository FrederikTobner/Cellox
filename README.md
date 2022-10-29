# Cellox

[![Build](https://github.com/FrederikTobner/Cellox/actions/workflows/build.yml/badge.svg)](https://github.com/FrederikTobner/Cellox/actions/workflows/build.yml)
[![Test](https://github.com/FrederikTobner/Cellox/actions/workflows/tests.yml/badge.svg)](https://github.com/FrederikTobner/Cellox/actions/workflows/tests.yml)
[![Analyze](https://github.com/FrederikTobner/Cellox/actions/workflows/codeql.yml/badge.svg)](https://github.com/FrederikTobner/Cellox/actions/workflows/codeql.yml)
![Lines of code](https://img.shields.io/tokei/lines/github/FrederikTobner/Cellox?style=plastic)
![GitHub repo size](https://img.shields.io/github/repo-size/FrederikTobner/Cellox?style=plastic)

Interpreter based on the book [Crafting interpreters](https://craftinginterpreters.com/contents.html) for the programming language cellox.

Cellox is a scripting language based on the programming language [lox](https://craftinginterpreters.com/the-lox-language.html) from Robert Nystrom.

## Overview

Cellox is a dynamically typed, object oriented, high-level scripting language.

## Data Types

In cellox there exist only 3 different base data types.

* Booleans
* Numbers, these are always 64bit floating point numbers
* Undefiened, the type of null

## Functions

A function call expression looks in cellox the same as it does in C.

```
foo(1, true);
```

Function definitions in cellox use the fun keyword and can return different types by the same function. E.g. a number or a boolean can be returned by the same function

```
fun foo(a, b)
{
    if(a == 0) return true;
    if(b == 0) return "string";
    return a + b;
}
```

Functions are first class in cellox. meaning they are real values that you can get a reference to.

Function declerations are statements, which gives you the ability to declare a local function inside another function.

## Classes

Cellox is a objectoriented language that features inheritance and methods that are bound to a class instance.

A class is defiened using the class keyword followed by an optional parent class that is prefixed with a double dot, like in C++ or C#.

```
class x : y
{
   
}
```

A constructor in cellox is called initializer and must be declared as a method inside the class.

The initializer method that initializes must be called init.

To refer to a method or a field of a cellox instande the this keyword followd by a dot is used.

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

An instance of this class is instanziated like this.

```
Point(1, 2);
```

The super keyword is used to refer to a method of the parent class.
```
class foo
{
   init(x)
   {
        this.x = x;
   }
   printfoo()
   {
        print this.x;
   }
}
class bar : foo
{
   init(y)
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

## How it works

The language provides automatic memory management using it's own [garbage collector](https://github.com/FrederikTobner/Cellox/wiki/Garbage-Collector), that uses the mark-and-sweep algorithm.

A cellox program is converted into bytecode and executed by a stack based [virtual machine](https://github.com/FrederikTobner/Cellox/wiki/Virtual-Machine).

## Dependencies

Cellox is written in C, using the C99 standard and the only depends on the C standard libary.

CMake is used for building within the whole project.

## Testing

The test are written in C++ using the [google-test framework](https://github.com/google/googletest).

For testing a cellox program is executed and the standard output redirected to a string. The string is compared with the expected output after the program was executed.
