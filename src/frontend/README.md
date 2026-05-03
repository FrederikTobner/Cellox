# frontend — Lexer, parser, and compiler

This directory builds **two** static library targets:

| Target | Sources | Description |
|--------|---------|-------------|
| `clx_lex` | `lexical_analysis/lexer.c` | Tokenises a Cellox source string into a stream of `token_t` values. |
| `clx_frontend` | `parsing/`, `compilation/` | Single-pass recursive-descent parser + bytecode emitter. |

`clx_lex` depends only on `clx_base`.  
`clx_frontend` depends on `clx_middleend` (to optimise emitted chunks) and `clx_lex`.

---

## clx_lex — Lexical analysis

| File | Description |
|------|-------------|
| `lexical_analysis/lexer.h/.c` | Stores source pointer, current line, and emits the next `token_t` on demand (`lexer_scan_token`). |

---

## clx_frontend — Parser and compiler

The frontend is a single-pass compiler: parsing and code-generation happen
simultaneously without constructing an AST.

| Subdirectory | Description |
|--------------|-------------|
| `parsing/` | Pratt-style expression parser (`expression_parser`) and statement parser (`statement_parser`). Shared parser state lives in `parser_state.h`. |
| `compilation/` | Compilation context (`compilation_context`), bytecode emitter helpers (`bytecode_emitter`), top-level compiler driver (`compiler`). |

### Public API

```c
// Compile a NUL-terminated source string; returns the top-level function or NULL on error.
object_function_t * compiler_compile(char const * source);

// Mark compiler roots for the GC (registered via garbage_collector_set_mark_roots_hook).
void compiler_mark_roots(void);
```
