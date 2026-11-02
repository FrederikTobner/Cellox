# Cellox grammar

This document describes the syntax of Cellox using the Extended Backus-Naur Form
([ISO/IEC 14977](https://www.iso.org/standard/26153.html)). It reflects the lexer
([lexer.c](src/frontend/src/lexical_analysis/lexer.c)), the parsers
([statement_parser.c](src/frontend/src/parsing/statement_parser.c),
[expression_parser.c](src/frontend/src/parsing/expression_parser.c)) and the module
pre-processor ([module_parser.c](src/module-loading/src/module_parser.c)).

## Notation

| Notation        | Meaning                              |
|-----------------|--------------------------------------|
| `a = b ;`       | Definition of the rule `a`           |
| `a , b`         | Concatenation                        |
| `a \| b`        | Alternation                          |
| `[ a ]`         | Optional (zero or one)               |
| `{ a }`         | Repetition (zero or more)            |
| `( a )`         | Grouping                             |
| `"x"` or `'x'`  | Terminal symbol                      |
| `(* ... *)`     | Comment                              |
| `? ... ?`       | Special sequence (informal wording)  |

## Lexical grammar

Whitespace (space, tab, carriage return, line feed) separates tokens and is otherwise ignored.

```ebnf
(* ---------- Comments ---------- *)

line_comment  = "//" , { ? any character except line feed ? } ;
block_comment = "/*" , { ? any character ? | block_comment } , "*/" ;   (* block comments nest *)

(* ---------- Identifiers ---------- *)

identifier = alpha , { alpha | digit } ;
alpha      = "a" | "b" | "c" | "d" | "e" | "f" | "g" | "h" | "i" | "j" | "k" | "l" | "m"
           | "n" | "o" | "p" | "q" | "r" | "s" | "t" | "u" | "v" | "w" | "x" | "y" | "z"
           | "A" | "B" | "C" | "D" | "E" | "F" | "G" | "H" | "I" | "J" | "K" | "L" | "M"
           | "N" | "O" | "P" | "Q" | "R" | "S" | "T" | "U" | "V" | "W" | "X" | "Y" | "Z"
           | "_" ;
digit      = "0" | "1" | "2" | "3" | "4" | "5" | "6" | "7" | "8" | "9" ;

(* ---------- Keywords (reserved, cannot be used as identifiers) ---------- *)

keyword = "and" | "break" | "catch" | "class" | "continue" | "do" | "else" | "error"
        | "false" | "for" | "fun" | "if" | "iferror" | "must" | "null" | "or" | "return"
        | "super" | "this" | "throw" | "true" | "try" | "var" | "while" ;

(* "import", "export" and "from" are handled by the module loader (see "Modules")
   and are not reserved by the lexer. "print" is no keyword: output is done with the
   native function printf. *)

(* ---------- Literals ---------- *)

number        = decimal_number | hex_number | binary_number ;
decimal_number = digit , { digit } , [ "." , digit , { digit } ] ;
hex_number    = "0" , ( "x" | "X" ) , hex_digit , { hex_digit } ;       (* at most 8 hex digits *)
binary_number = "0" , ( "b" | "B" ) , bin_digit , { bin_digit } ;       (* at most 32 binary digits *)
hex_digit     = digit | "a" | "b" | "c" | "d" | "e" | "f"
              | "A" | "B" | "C" | "D" | "E" | "F" ;
bin_digit     = "0" | "1" ;

string          = '"' , { string_char | escape_sequence } , '"' ;       (* may span multiple lines *)
string_char     = ? any character except '"' and "\" ? ;
escape_sequence = "\" , ( "a" | "b" | "f" | "n" | "r" | "t" | "v" | '"' | "'" | "\" | "?"
                        | octal_digit , [ octal_digit , [ octal_digit ] ]
                        | "x" , hex_digit , [ hex_digit ] ) ;           (* octal / hex values <= 255 *)
octal_digit     = "0" | "1" | "2" | "3" | "4" | "5" | "6" | "7" ;

boolean = "true" | "false" ;
null    = "null" ;

(* ---------- Operators and punctuation ---------- *)

(*  (  )  {  }  [  ]  ;  ,  .  ..  :  |
    +  -  *  /  %  **  !  =  ==  !=  <  <=  >  >=
    +=  -=  *=  /=  %=  **=  and  &&  or  ||                               *)
```

## Syntactic grammar

### Program and declarations

```ebnf
program     = { declaration } ;

declaration = class_decl
            | error_decl
            | fun_decl
            | var_decl
            | statement ;

var_decl    = "var" , identifier , [ "=" , ( expression | array_literal_init ) ] , ";" ;
array_literal_init = "{" , [ expression , { "," , expression } ] , "}" ;  (* same as array_literal *)

fun_decl    = "fun" , identifier , function ;
function    = "(" , [ parameters ] , ")" , block ;
parameters  = identifier , { "," , identifier } ;                          (* at most 255 *)

class_decl  = "class" , identifier , [ ":" , identifier ] , "{" , { method } , "}" ;
method      = identifier , function ;                                      (* "init" is the initializer *)

error_decl  = "error" , identifier , "{" , [ error_variants ] , "}" ;
error_variants = identifier , { "," , identifier } , [ "," ] ;             (* trailing comma allowed *)
```

### Statements

```ebnf
statement  = expression_stmt
           | for_stmt
           | if_stmt
           | return_stmt
           | break_stmt
           | continue_stmt
           | while_stmt
           | do_while_stmt
           | iferror_stmt
           | throw_stmt
           | block ;

block      = "{" , { declaration } , "}" ;

expression_stmt = expression , ";" ;

if_stmt    = "if" , "(" , expression , ")" , statement , [ "else" , statement ] ;

while_stmt = "while" , "(" , expression , ")" , statement ;

do_while_stmt = "do" , statement , "while" , "(" , expression , ")" , ";" ;

for_stmt   = "for" , "(" ,
             ( var_decl | expression_stmt | ";" ) ,
             [ expression ] , ";" ,
             [ expression ] ,
             ")" , statement ;

return_stmt   = "return" , [ expression ] , ";" ;   (* not allowed at top level; no value in "init" *)
break_stmt    = "break" , ";" ;                      (* only inside a loop *)
continue_stmt = "continue" , ";" ;                   (* only inside a loop *)

throw_stmt    = "throw" , expression , ";" ;         (* not allowed at top level *)

iferror_stmt  = "iferror" , expression ,
                "|" , identifier , "|" , block ,
                "else" ,
                "|" , identifier , "|" , block ;     (* error branch first, then success branch *)
```

At statement position `iferror` always starts an `iferror_stmt`; the expression form
(see below) is only available where an expression is expected, e.g. `var x = iferror ...;`.

### Expressions

Expressions are listed from the lowest to the highest precedence.

```ebnf
expression  = assignment ;

assignment  = assign_target , assign_op , expression      (* right associative *)
            | catch_expr ;

assign_target = identifier
              | call , "." , identifier                  (* property *)
              | identifier , "[" , expression , "]" ;    (* array element, only "=" *)

assign_op   = "=" | "+=" | "-=" | "*=" | "/=" | "%=" | "**=" ;
              (* compound operators are not available for array elements *)

(* "catch" binds as tightly as "or" *)
catch_expr  = logic_or , { "catch" , catch_handler } ;

catch_handler = logic_or                                  (* fallback value *)
              | bound_handler , [ "else" , bound_handler ] ;
              (* |err| handles the error, |val| (after else) transforms the success value *)

logic_or    = logic_and , { ( "or" | "||" ) , logic_and } ;
logic_and   = equality  , { ( "and" | "&&" ) , equality } ;
equality    = comparison , { ( "==" | "!=" ) , comparison } ;
comparison  = term , { ( "<" | "<=" | ">" | ">=" ) , term } ;
term        = factor , { ( "+" | "-" ) , factor } ;
factor      = unary , { ( "*" | "/" | "%" | "**" ) , unary } ;   (* ** has the same level as * and is left associative *)

unary       = ( "!" | "-" | "try" | "must" ) , unary
            | call ;

call        = primary , { "(" , [ arguments ] , ")"
                        | "." , identifier } ;

arguments   = expression , { "," , expression } ;                 (* at most 255 *)

primary     = number
            | string
            | boolean
            | null
            | identifier , [ "[" , index , "]" ]                       (* see "Arrays, indexing and slices" *)
            | "this"
            | "super" , "." , identifier
            | "(" , expression , ")"
            | array_literal
            | iferror_expr ;
```

### Arrays, indexing and slices

```ebnf
array_literal = "{" , [ expression , { "," , expression } ] , "}" ;    (* at most 255 elements *)

index         = expression                                            (* element access *)
              | expression , ".." , expression ;                      (* slice *)
```

* Arrays are dynamic and are created with `{ ... }` (`var a = {1, 2, 3};`, `var empty = {};`).
  At statement position a leading `{` starts a block, not an array literal.
* `a[i]` reads and `a[i] = v` writes an element. Strings can be indexed as well, but not assigned to.
* `a[from .. to]` creates a slice (a copy) of an array or a string. The upper bound is exclusive.
* Indexing is only supported directly on a variable name (`a[0]`, `a[1 .. 3]`). Indexing the result of
  another expression (`a[0][1]`, `f()[0]`, `this.items[0]`) is a syntax error, assign it to a variable first
  (`var items = this.items; items[0]`).

### Error handling

```ebnf
iferror_expr  = "iferror" , expression ,
                bound_handler , "else" , bound_handler ;

bound_handler = "|" , identifier , "|" , ( handler_block | expression ) ;

handler_block = "{" , [ expression , { ";" , expression } , [ ";" ] ] , "}" ;
```

* `error Name { A, B }` declares an error set, the variants are accessed with `Name.A`.
* `throw Name.A;` returns the error from the current function.
* `try expr` unwraps the success value of `expr` or propagates the error to the caller.
* `must expr` unwraps the success value of `expr` and stops the program with a runtime error if it is an error.
* `expr catch fallback` evaluates to the success value of `expr` or to `fallback` otherwise.
* `expr catch |err| handler [else |val| handler]` runs `handler` with the error bound to `err`
  (and optionally maps the success value bound to `val`).
* `iferror expr |err| ... else |val| ...` branches on the result of `expr`. Used as a statement
  both branches are blocks of declarations. Used as an expression each branch yields a value:
  either a single expression or a block whose value is its last expression (`;` separated). An empty
  block or a trailing `;` yields `null`.
* The interpreter provides the error sets `IoError` (`InvalidArgument`, `OpenFailed`, `AllocFailed`,
  `ReadFailed`, `WriteFailed`) and `StdlibError` (`InvalidArgument`, `TypeError`, `DomainError`,
  `FormatError`, `ReadFailed`, `SystemFailed`).

## Operator precedence

From lowest to highest:

| Level | Operators                                   | Associativity |
|-------|---------------------------------------------|---------------|
| 1     | `=` `+=` `-=` `*=` `/=` `%=` `**=`          | right         |
| 2     | `or` `\|\|` `catch`                         | left          |
| 3     | `and` `&&`                                  | left          |
| 4     | `==` `!=`                                   | left          |
| 5     | `<` `<=` `>` `>=`                           | left          |
| 6     | `+` `-`                                     | left          |
| 7     | `*` `/` `%` `**`                            | left          |
| 8     | `!` `-` (unary) `try` `must`                | right         |
| 9     | `()` `.` `[]`                               | left          |

## Modules

`import` and `export` are not part of the token grammar. The module loader processes the source
**line by line before lexing**, so every directive has to be written on a single line.
Imports are removed from the source and `export` is stripped from the declaration it precedes.

```ebnf
module_line = import_decl | export_decl | ? any other source line ? ;

import_decl = "import" , string , ";"
            | "import" , "{" , identifier , { "," , identifier } , "}" , "from" , string , ";" ;

export_decl = "export" , ( "var" | "fun" | "class" ) , identifier , ? rest of the declaration ? ;

import_path = string ;   (* "./relative.clx", "../relative.clx", "/absolute.clx" or a bare "stdlib/math.clx" *)
```

* Relative paths are resolved from the importing file, bare paths from the standard library directory.
* Every module is loaded once, import cycles and unknown paths or names are reported as module errors.
* A name can only be imported when it is exported by the module (`export var`, `export fun`
  or `export class`). Error sets cannot be exported.

## Built-in functions

`printf` and the other natives are ordinary global functions, not syntax:

`append_to_file`, `array_length`, `asci_to_num`, `class_of`, `clock`, `cosine`, `exit`, `exponential`,
`logarithm`, `logarithm10`, `num_to_asci`, `on_linux`, `on_macOS`, `on_windows`, `printf`, `random`,
`read_file`, `read_key`, `read_line`, `sine`, `size_of`, `string_hash`, `strlen`, `string_replace_at`,
`system`, `tangent`, `wait`, `write_to_file`.

`printf(format, ...)` replaces every `{}` in `format` with the next argument.

## Example

```
import { floor } from "stdlib/math.clx";

error MathError { DivisionByZero }

class Calculator {
    divide(a, b) {
        if (b == 0) throw MathError.DivisionByZero;
        return a / b;
    }
}

var calc = Calculator();
var values = {10, 20, 30, 40};

for (var i = 0; i < array_length(values); i += 1) {
    printf("{}\n", calc.divide(values[i], 4) catch 0);
}

iferror calc.divide(1, 0) |err| {
    printf("failed: {}\n", err);
} else |val| {
    printf("result: {}\n", val);
}

printf("{}\n", values[1 .. 3]);
```
