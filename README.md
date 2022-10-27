# Cellox

[![Build](https://github.com/FrederikTobner/Cellox/actions/workflows/build.yml/badge.svg)](https://github.com/FrederikTobner/Cellox/actions/workflows/build.yml)
[![Build](https://github.com/FrederikTobner/Cellox/actions/workflows/test.yml/badge.svg)](https://github.com/FrederikTobner/Cellox/actions/workflows/test.yml)
<br/>
Interpreter based on the book [Crafting interpreters](https://craftinginterpreters.com/contents.html) for the programming language cellox <br/>
Cellox is a dynamically typed, object oriented, high-level scripting language based on the programming language [lox](https://craftinginterpreters.com/the-lox-language.html) from Robert Nystrom.
<br/>
The language provides automatic memory management using it's own [garbage collector](https://github.com/FrederikTobner/Cellox/wiki/Garbage-Collector) that uses the mark-and-sweep algorithm. <br/>
A cellox program is converted into bytecode and executed by a stack based [virtual machine](https://github.com/FrederikTobner/Cellox/wiki/Virtual-Machine).
