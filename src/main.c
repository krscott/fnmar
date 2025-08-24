#include <assert.h>
#include <ctype.h>
#include <fnmatch.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

enum error
{
    OK = 0,
    ERR_FILESYSTEM,
    ERR_ARGS,
    ERR_OUT_OF_MEMORY,
};

static char const *const DEFAULT_CONFIG_FILENAME = "fnmar.txt";

#define str_debug_print(s) printf("%.*s", (int)(s).len, (s).ptr)
#define str_debug_println(s) printf("%.*s\n", (int)(s).len, (s).ptr)

struct str
{
    char *ptr;
    size_t len;
};

void str_split_at_delims(
    struct str const s,
    char const *const delims,
    struct str *const head,
    struct str *const tail
)
{
    for (size_t i = 0; i < s.len; ++i)
    {
        if (strchr(delims, s.ptr[i]))
        {
            *head = (struct str){
                .ptr = s.ptr,
                .len = i,
            };
            *tail = (struct str){
                .ptr = &s.ptr[i],
                .len = s.len - i,
            };

            goto done;
        }
    }

    *head = s;
    *tail = (struct str){0};

done:
    return;
}

struct str str_trim_left_char(struct str s, char const c)
{
    while (s.len > 0 && *s.ptr == c)
    {
        ++s.ptr;
        --s.len;
    }

    return s;
}

struct str str_trim_left_whitespace(struct str s)
{
    while (s.len > 0 && isspace(*s.ptr))
    {
        ++s.ptr;
        --s.len;
    }

    return s;
}

struct str str_trim_right_whitespace(struct str s)
{
    while (s.len > 0 && isspace(s.ptr[s.len - 1]))
    {
        --s.len;
    }

    return s;
}

struct str str_trim_whitespace(struct str s)
{
    return str_trim_right_whitespace(str_trim_left_whitespace(s));
}

struct cstrbuf
{
    char *ptr;
    size_t len;
    size_t cap;
};

struct str cstrbuf_to_str(struct cstrbuf const cstrbuf)
{
    return (struct str){
        .ptr = cstrbuf.ptr,
        .len = cstrbuf.len,
    };
}

enum error cstrbuf_init_from_file( //
    struct cstrbuf *const cstrbuf,
    char const *const filepath
)
{
    enum error err = OK;

    FILE *file = fopen(filepath, "rb");
    if (!file)
    {
        perror(filepath);
        err = ERR_FILESYSTEM;
        goto done;
    }

    fseek(file, 0, SEEK_END);
    size_t const len = ftell(file);
    fseek(file, 0, SEEK_SET);

    char *buf = malloc(len + 1);
    if (!buf)
    {
        printf("Out of memory\n");
        err = ERR_OUT_OF_MEMORY;
        goto done;
    }

    size_t bytes_read = fread(buf, 1, len, file);
    assert(bytes_read == len);
    buf[bytes_read] = '\0';

    *cstrbuf = (struct cstrbuf){
        .ptr = buf,
        .len = len,
        .cap = len,
    };

done:
    if (file)
    {
        fclose(file);
    }
    return err;
}

void cstrbuf_deinit(struct cstrbuf *const cstrbuf)
{
    if (cstrbuf->ptr)
    {
        free(cstrbuf->ptr);
    }
}

#define X_ENUM_VAR(x) x,

#define ENUM_DEF(name, KINDS_X)                                                \
    enum name                                                                  \
    {                                                                          \
        KINDS_X(X_ENUM_VAR)                                                    \
    }

#define X_CASE_TO_CSTR(x)                                                      \
    case x:                                                                    \
        return #x;
#define ENUM_IMPL_TO_CSTR(name, KINDS_X)                                       \
    char const *name##_to_cstr(enum name const val)                            \
    {                                                                          \
        switch (val)                                                           \
        {                                                                      \
            KINDS_X(X_CASE_TO_CSTR)                                            \
        default:                                                               \
            return "(unknown)";                                                \
        }                                                                      \
    }

#define X_CASE_DEBUG_PRINT(x)                                                  \
    case x:                                                                    \
        printf(#x);                                                            \
        break;
#define ENUM_IMPL_DEBUG_PRINT(name, KINDS_X)                                   \
    void name##_debug_print(enum name const val)                               \
    {                                                                          \
        switch (val)                                                           \
        {                                                                      \
            KINDS_X(X_CASE_DEBUG_PRINT)                                        \
        default:                                                               \
            printf("(invalid %u)", val);                                       \
            break;                                                             \
        }                                                                      \
    }

#define TOKEN_KINDS(X)                                                         \
    X(TOK_NONE)                                                                \
    X(TOK_COMMENT)                                                             \
    X(TOK_PATTERN)                                                             \
    X(TOK_SEMI)                                                                \
    X(TOK_COLON)                                                               \
    X(TOK_CMD)                                                                 \
    X(TOK_NEWLINE)                                                             \
    X(TOK_EOF)
ENUM_DEF(token_kind, TOKEN_KINDS);
ENUM_IMPL_DEBUG_PRINT(token_kind, TOKEN_KINDS);

struct token
{
    enum token_kind kind;
    struct str str;
};

void token_debug_print(struct token const token)
{
    printf("token { ");
    token_kind_debug_print(token.kind);
    printf(", `");
    switch (token.kind)
    {
    case TOK_NEWLINE:
        if (token.str.ptr[0] == '\n')
        {
            printf("\\n");
        }
        else
        {
            printf("\\r");
        }
        break;

    case TOK_NONE:
    case TOK_PATTERN:
    case TOK_SEMI:
    case TOK_COLON:
    case TOK_CMD:
    case TOK_EOF:
    default:
        printf("%.*s", (int)token.str.len, token.str.ptr);
        break;
    }
    printf("` }\n");
}

enum token_kind get_delim_kind(char const c)
{
    enum token_kind kind;

    switch (c)
    {
    case '\0':
        kind = TOK_EOF;
        break;
    case ';':
        kind = TOK_SEMI;
        break;
    case ':':
        kind = TOK_COLON;
        break;
    case '\r':
    case '\n':
        kind = TOK_NEWLINE;
        break;
    default:
        kind = TOK_NONE;
        break;
    }

    return kind;
}

struct token parse_line_start(struct str input, struct str *const tail)
{
    struct token token = {0};

    input = str_trim_left_whitespace(input);

    if (input.len == 0)
    {
        token.kind = TOK_EOF;
        *tail = input;
    }
    else if (input.ptr[0] == '#')
    {
        token.kind = TOK_COMMENT;
        str_split_at_delims(input, "\r\n", &token.str, tail);
    }
    else
    {
        token.kind = TOK_NONE;
        *tail = input;
    }

    return token;
}

struct token parse_pattern_delim(struct str input, struct str *const tail)
{
    struct token token = {0};

    input = str_trim_left_whitespace(input);

    if (input.len == 0)
    {
        token.kind = TOK_EOF;
        *tail = input;
    }
    else
    {
        token.kind = get_delim_kind(input.ptr[0]);
        if (token.kind)
        {
            token.str = (struct str){
                .ptr = input.ptr,
                .len = 1,
            };
            *tail = (struct str){
                .ptr = input.ptr + 1,
                .len = input.len - 1,
            };
        }
        else
        {
            *tail = input;
        }
    }

    return token;
}

struct token parse_pattern(struct str input, struct str *const tail)
{
    struct token token = {0};

    input = str_trim_left_whitespace(input);

    if (input.len == 0)
    {
        token.kind = TOK_EOF;
        *tail = input;
    }
    else
    {
        str_split_at_delims(input, ";:\r\n", &token.str, tail);
        token.str = str_trim_whitespace(token.str);
        if (token.str.len > 0)
        {
            token.kind = TOK_PATTERN;
        }
        else
        {
            token = parse_pattern_delim(input, tail);
        }
    }

    return token;
}

struct token parse_command(struct str input, struct str *const tail)
{
    struct token token = {0};

    input = str_trim_left_char(input, ' ');

    if (input.len == 0)
    {
        token.kind = TOK_EOF;
        *tail = input;
    }
    else
    {
        str_split_at_delims(input, "\r\n", &token.str, tail);
        token.str = str_trim_whitespace(token.str);
        if (token.str.len > 0)
        {
            token.kind = TOK_CMD;
        }
        else
        {
            token.kind = TOK_NONE;
        }
    }

    return token;
}

struct file_pos
{
    // Zero-indexed line number
    size_t line;
    // Zero-indexed column number
    size_t column;
};

struct file_pos find_token_pos(struct str const text, struct token const token)
{
    assert(text.ptr <= token.str.ptr);
    assert(token.str.ptr - text.ptr < text.len);

    struct file_pos pos = {0};

    size_t token_index = token.str.ptr - text.ptr;

    for (size_t i = 0; i < token_index; ++i)
    {
        if (text.ptr[i] == '\n')
        {
            ++pos.line;
            pos.column = 0;
        }
        else
        {
            ++pos.column;
        }
    }

    return pos;
}

#define PARSER_STATES(X)                                                       \
    X(PS_LINE_START)                                                           \
    X(PS_PATTERN)                                                              \
    X(PS_PATTERN_DELIM)                                                        \
    X(PS_COMMAND)
ENUM_DEF(parser_state, PARSER_STATES);
ENUM_IMPL_TO_CSTR(parser_state, PARSER_STATES);

void config_find_match( //
    struct str const haystack,
    char const *const needle,
    struct str *const command
)
{
    struct str remaining = str_trim_whitespace(haystack);
    struct token token;

    enum parser_state state = (enum parser_state)0;

    bool unexpected_token = false;

    while (token.kind != TOK_EOF && !unexpected_token)
    {
        printf("%-18s ", parser_state_to_cstr(state));

        switch (state)
        {
        case PS_LINE_START:
            token = parse_line_start(remaining, &remaining);
            break;

        case PS_PATTERN:
            token = parse_pattern(remaining, &remaining);
            break;

        case PS_PATTERN_DELIM:
            token = parse_pattern_delim(remaining, &remaining);
            break;

        case PS_COMMAND:
            token = parse_command(remaining, &remaining);
            break;
        }

        token_debug_print(token);

        switch (state)
        {
        case PS_LINE_START:
            switch (token.kind)
            {
            case TOK_NONE:
                state = PS_PATTERN;
                break;
            case TOK_COMMENT:
                // do nothing
                break;
            case TOK_PATTERN:
            case TOK_SEMI:
            case TOK_COLON:
            case TOK_CMD:
            case TOK_NEWLINE:
            case TOK_EOF:
                unexpected_token = true;
                break;
            }
            break;

        case PS_PATTERN:
            switch (token.kind)
            {
            case TOK_PATTERN:
                // TODO
                state = PS_PATTERN_DELIM;
                break;
            case TOK_COMMENT:
            case TOK_NONE:
            case TOK_SEMI:
            case TOK_COLON:
            case TOK_CMD:
            case TOK_NEWLINE:
            case TOK_EOF:
                unexpected_token = true;
                break;
            }
            break;

        case PS_PATTERN_DELIM:
            switch (token.kind)
            {
            case TOK_NONE:
                state = PS_PATTERN;
                break;
            case TOK_NEWLINE:
            case TOK_SEMI:
                // do nothing
                break;
            case TOK_COLON:
                state = PS_COMMAND;
                break;
            case TOK_COMMENT:
            case TOK_PATTERN:
            case TOK_CMD:
            case TOK_EOF:
                unexpected_token = true;
                break;
            }
            break;

        case PS_COMMAND:
            switch (token.kind)
            {
            case TOK_NEWLINE:
                state = PS_LINE_START;
                break;
            case TOK_CMD:
                // TODO
                state = PS_LINE_START;
                break;
            case TOK_NONE:
            case TOK_COMMENT:
            case TOK_PATTERN:
            case TOK_SEMI:
            case TOK_COLON:
            case TOK_EOF:
                unexpected_token = true;
                break;
            }
            break;
        }
    }

    if (unexpected_token)
    {
        struct file_pos pos = find_token_pos(haystack, token);
        printf(
            "Unexpected token at line %lu col %lu: '%.*s'\n",
            pos.line + 1,
            pos.column + 1,
            (int)token.str.len,
            token.str.ptr
        );
    }

    return;
}

int main(int const argc, char const *const *const argv)
{
    enum error err = OK;
    struct cstrbuf config_str = {0};

    if (argc != 2)
    {
        printf("Usage: %s file\n", argv[0]);
        err = ERR_ARGS;
        goto done;
    }

    err = cstrbuf_init_from_file(&config_str, DEFAULT_CONFIG_FILENAME);
    if (err)
    {
        goto done;
    }

    struct str cmd = {0};
    config_find_match(cstrbuf_to_str(config_str), argv[1], &cmd);

done:
    cstrbuf_deinit(&config_str);
    return err;
}
