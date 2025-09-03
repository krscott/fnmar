#ifndef XGEN_MAIN_H_
#define XGEN_MAIN_H_

/* Generated based on main.c */

#define xgen_token_kind(X) \
    X(TOK_NONE) \
    X(TOK_COMMENT) \
    X(TOK_PATTERN) \
    X(TOK_SEMI) \
    X(TOK_COLON) \
    X(TOK_CMD) \
    X(TOK_EOF)

#define xgen_parser_state(X) \
    X(PS_LINE_START) \
    X(PS_PATTERN) \
    X(PS_PATTERN_DELIM) \
    X(PS_COMMAND)

#endif
