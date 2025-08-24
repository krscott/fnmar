#include <assert.h>
#include <fnmatch.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

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

#define X_CASE_DEBUG_PRINT(x)                                                  \
    case x:                                                                    \
        printf(#x);                                                            \
        break;

#define ENUM_IMPL_DEBUG_PRINT(name, KINDS_X)                                   \
    void name##_debug_print(enum name const x)                                 \
    {                                                                          \
        switch (x)                                                             \
        {                                                                      \
            KINDS_X(X_CASE_DEBUG_PRINT)                                        \
        default:                                                               \
            printf("(invalid %u)", x);                                         \
            break;                                                             \
        }                                                                      \
    }

#define TOKEN_KINDS(X)                                                         \
    X(TOK_NONE)                                                                \
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

    case TOK_EOF:
        printf("EOF");
        break;

    case TOK_NONE:
    case TOK_PATTERN:
    case TOK_SEMI:
    case TOK_COLON:
    case TOK_CMD:
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

struct token parse_delim(struct str const input, struct str *const tail)
{
    struct token token = {0};

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
    struct token token = {
        .kind = TOK_EOF,
        .str.ptr = input.ptr,
    };

    if (input.len > 0)
    {
        token.kind = TOK_NONE;

        while (input.len > 0 && !get_delim_kind(*input.ptr))
        {
            ++input.ptr;
            --input.len;

            token.kind = TOK_PATTERN;
            ++token.str.len;
        }
    }

    *tail = input;
    return token;
}

struct token parse_command(struct str input, struct str *const tail)
{
    struct token token = {
        .kind = TOK_EOF,
        .str = input,
    };

    if (input.len > 0)
    {
        token.kind = TOK_NONE;

        for (size_t i = 0; i < input.len; ++i)
        {
            switch (input.ptr[i])
            {
            case '\0':
            case '\r':
            case '\n':
                input.ptr += i;
                input.len -= i;
                token.str.len = i;
                goto done;
            }

            token.kind = TOK_CMD;
        }
    }

done:
    *tail = input;
    return token;
}

void config_find_match( //
    struct str const haystack,
    char const *const needle,
    struct str *const command
)
{
    struct str remaining = haystack;
    struct token token;

    for (;;)
    {
        for (;;)
        {
            printf("# Pattern\n");
            token = parse_pattern(remaining, &remaining);
            if (token.kind == TOK_EOF)
            {
                goto done;
            }
            token_debug_print(token);

            printf("# Delim\n");
            token = parse_delim(remaining, &remaining);
            if (token.kind == TOK_EOF)
            {
                goto done;
            }
            token_debug_print(token);

            if (token.kind == TOK_COLON)
            {
                break;
            }
        }

        printf("# Command\n");
        token = parse_command(remaining, &remaining);
        if (token.kind == TOK_EOF)
        {
            goto done;
        }
        token_debug_print(token);

        printf("# Newline\n");
        token = parse_delim(remaining, &remaining);
        if (token.kind == TOK_EOF)
        {
            goto done;
        }
        token_debug_print(token);
    }

done:
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
