#ifndef clox_scanner_h
#define clox_scanner_h

// Tokens of the language
typedef enum
{
#pragma region SingleCharacterTokens
  // (
  TOKEN_LEFT_PAREN,
  // )
  TOKEN_RIGHT_PAREN,
  // {
  TOKEN_LEFT_BRACE,
  // }
  TOKEN_RIGHT_BRACE,
  // ,
  TOKEN_COMMA,
  // .
  TOKEN_DOT,
  // -
  TOKEN_MINUS,
  // +
  TOKEN_PLUS,
  // ;
  TOKEN_SEMICOLON,
  // /
  TOKEN_SLASH,
  // *
  TOKEN_STAR,
  // !
  TOKEN_BANG,
  // >
  TOKEN_GREATER,
  // <
  TOKEN_LESS,
  // =
  TOKEN_EQUAL,
#pragma endregion SingleCharacterTokens
#pragma region TwoCharacterTokens
  // !=
  TOKEN_BANG_EQUAL,
  // ==
  TOKEN_EQUAL_EQUAL,
  // >=
  TOKEN_GREATER_EQUAL,
  // <=
  TOKEN_LESS_EQUAL,
#pragma endregion TwoCharacterTokens
#pragma region Literals
  // An identifier e.g. for the variable named x
  TOKEN_IDENTIFIER,
  // A string literal e.g. "Hello World!"
  TOKEN_STRING,
  // A number literal e.g. 5.3
  TOKEN_NUMBER,
  // Boolean literal true
  TOKEN_TRUE,
  // Boolean literal false
  TOKEN_FALSE,
  // literal nil-value/null-value/undefiened value
  TOKEN_NIL,
#pragma endregion Literals
#pragma region Keywords
  // and
  TOKEN_AND,
  // class
  TOKEN_CLASS,
  // else
  TOKEN_ELSE,
  // for
  TOKEN_FOR,
  // fun
  TOKEN_FUN,
  // if
  TOKEN_IF,
  // or
  TOKEN_OR,
  // print
  TOKEN_PRINT,
  // return
  TOKEN_RETURN,
  // super
  TOKEN_SUPER,
  // this
  TOKEN_THIS,
  // var
  TOKEN_VAR,
  // while
  TOKEN_WHILE,
#pragma endregion Keywords
#pragma region InternalTokens
  // Used to mark an error in the parsing process
  TOKEN_ERROR,
  /// Used to mark the end of the file
  TOKEN_EOF
#pragma endregion InternalTokens
} TokenType;

// Type definition of the scanner
typedef struct
{
  TokenType type;
  const char *start;
  int length;
  int line;
} Token;

// Initializes the scanner
void initScanner(const char *source);

// Scans the next token in the sourcecode and saves it in a linear sequence of tokens
Token scanToken();

#endif