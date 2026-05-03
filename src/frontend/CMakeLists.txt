cellox_add_library(clx_lex
    SOURCES
        lexical_analysis/lexer.c
    DEPENDENCIES
        clx_base
)

cellox_add_library(clx_frontend
    SOURCES
        parsing/expression_parser.c
        parsing/statement_parser.c
        compilation/bytecode_emitter.c
        compilation/compilation_context.c
        compilation/compiler.c
    DEPENDENCIES
        clx_middleend
        clx_lex
)
