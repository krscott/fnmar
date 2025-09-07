#ifndef MAIN_XGEN_H_
#define MAIN_XGEN_H_

/* Generated based on main.c */

// enum token_kind
// {
//     TOK_NONE,
//     TOK_COMMENT,
//     TOK_PATTERN,
//     TOK_SEMI,
//     TOK_COLON,
//     TOK_CMD,
//     TOK_EOF,
// };
#define token_kind_x_count 7
#define token_kind_x_variants(X)                                               \
    X(TOK_NONE)                                                                \
    X(TOK_COMMENT)                                                             \
    X(TOK_PATTERN)                                                             \
    X(TOK_SEMI)                                                                \
    X(TOK_COLON)                                                               \
    X(TOK_CMD)                                                                 \
    X(TOK_EOF)

// enum parser_state
// {
//     PS_LINE_START,
//     PS_PATTERN,
//     PS_PATTERN_DELIM,
//     PS_COMMAND,
// };
#define parser_state_x_count 4
#define parser_state_x_variants(X)                                             \
    X(PS_LINE_START)                                                           \
    X(PS_PATTERN)                                                              \
    X(PS_PATTERN_DELIM)                                                        \
    X(PS_COMMAND)

// struct cli
// {
//     char const *filename;
//
//     x_attr(
//         cliopt_attr,
//         .name = "--config",
//         .short_name = 'c',
//         .help = "Config file (default: " DEFAULT_CONFIG_FILENAME ")"
//     );
//     char const *config_filename;
// };
#define cli_x_fields(F)                                                        \
    F(xf_simple, char const *, filename)                                       \
    F(xf_simple, char const *, config_filename)
#define cli_x_cliopt_attr_fields(F)                                            \
    F(xf_simple, char const *, filename)                                       \
    F(xf_simple_attr,                                                          \
      char const *,                                                            \
      config_filename,                                                         \
      .name = "--config",                                                      \
      .short_name = 'c',                                                       \
      .help = "Config file (default: " DEFAULT_CONFIG_FILENAME ")")

#endif
