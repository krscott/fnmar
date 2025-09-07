#include "krs_cliopt.h"
#include "krs_log.h"
#include "krs_str.h"
#include "krs_types.h"
#include "krs_x.h"
#include "main_xgen.h"

#include <assert.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

// Suppress false unused header
#ifndef MAIN_XGEN_H_
#endif

#ifdef _WIN32
#include <shlwapi.h>

static int fnmatch(char const *pattern, char const *string, int flags)
{
    (void)flags;
    return PathMatchSpecA(string, pattern) ? 0 : 1;
}

#else
#include <fnmatch.h>
#endif

#define DEFAULT_CONFIG_FILENAME "fnmar.txt"

enum error
{
    OK = 0,
    ERR_NO_MATCHES,
    ERR_FILESYSTEM,
    ERR_ARGS,
    ERR_OUT_OF_MEMORY,
};

static enum error out_of_memory(void)
{
    klog(LL_FATAL, "Out of memory");
    return ERR_OUT_OF_MEMORY;
}

static enum error cstrbuf_init_from_file( //
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
    size_t const len = (size_t)ftell(file);
    fseek(file, 0, SEEK_SET);

    char *buf = malloc(len + 1);
    if (!buf)
    {
        err = out_of_memory();
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

xgen();
enum token_kind
{
    TOK_NONE,
    TOK_COMMENT,
    TOK_PATTERN,
    TOK_SEMI,
    TOK_COLON,
    TOK_CMD,
    TOK_EOF,
};
static x_enum_to_cstr_impl(token_kind);

struct token
{
    enum token_kind kind;
    struct str str;
};

static enum token_kind get_delim_kind(char const c)
{
    enum token_kind kind;

    switch (c)
    {
    case ';':
        kind = TOK_SEMI;
        break;
    case ':':
        kind = TOK_COLON;
        break;
    default:
        kind = TOK_NONE;
        break;
    }

    return kind;
}

static struct token parse_line_start(struct str input, struct str *const tail)
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

static struct token parse_pattern_delim( //
    struct str input,
    struct str *const tail
)
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
                .ptr = &input.ptr[1],
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

static struct token parse_pattern(struct str input, struct str *const tail)
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

static struct token parse_command(struct str input, struct str *const tail)
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
    u32 line;
    // Zero-indexed column number
    u32 column;
};

static struct file_pos find_file_pos( //
    struct str const text,
    char const *const ptr
)
{
    assert(ptr != NULL);
    assert(text.ptr <= ptr);
    assert((size_t)(ptr - text.ptr) < text.len);

    struct file_pos pos = {0};

    size_t token_index = (size_t)(ptr - text.ptr);

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

xgen();
enum parser_state
{
    PS_LINE_START,
    PS_PATTERN,
    PS_PATTERN_DELIM,
    PS_COMMAND,
};
static x_enum_to_cstr_impl(parser_state);

struct fnmar_parser
{
    enum parser_state state;
    struct token token;
    struct str tail;
    struct str full_text;
    bool unexpected_token;
    bool is_done;
};

static void fnmar_parser_start( //
    struct fnmar_parser *const parser,
    struct str const text
)
{
    *parser = (struct fnmar_parser){
        .full_text = text,
        .tail = str_trim_whitespace(text),
    };
}

static void fnmar_parser_next(struct fnmar_parser *const parser)
{
    if (!parser->is_done)
    {
        switch (parser->state)
        {
        case PS_LINE_START:
            parser->token = parse_line_start(parser->tail, &parser->tail);
            break;

        case PS_PATTERN:
            parser->token = parse_pattern(parser->tail, &parser->tail);
            break;

        case PS_PATTERN_DELIM:
            parser->token = parse_pattern_delim(parser->tail, &parser->tail);
            break;

        case PS_COMMAND:
            parser->token = parse_command(parser->tail, &parser->tail);
            break;
        }

        klog(
            LL_DEBUG,
            "%-19s %-15s %.*s",
            parser_state_to_cstr(parser->state),
            token_kind_to_cstr(parser->token.kind),
            str_format_args(parser->token.str)
        );

        switch (parser->state)
        {
        case PS_LINE_START:
            switch (parser->token.kind)
            {
            case TOK_NONE:
                parser->state = PS_PATTERN;
                break;
            case TOK_COMMENT:
            case TOK_EOF:
                // do nothing
                break;
            case TOK_PATTERN:
            case TOK_SEMI:
            case TOK_COLON:
            case TOK_CMD:
                parser->unexpected_token = true;
                break;
            }
            break;

        case PS_PATTERN:
            switch (parser->token.kind)
            {
            case TOK_PATTERN:
                parser->state = PS_PATTERN_DELIM;
                break;
            case TOK_COMMENT:
            case TOK_NONE:
            case TOK_SEMI:
            case TOK_COLON:
            case TOK_CMD:
            case TOK_EOF:
                parser->unexpected_token = true;
                break;
            }
            break;

        case PS_PATTERN_DELIM:
            switch (parser->token.kind)
            {
            case TOK_NONE:
                parser->state = PS_PATTERN;
                break;
            case TOK_SEMI:
                // do nothing
                break;
            case TOK_COLON:
                parser->state = PS_COMMAND;
                break;
            case TOK_COMMENT:
            case TOK_PATTERN:
            case TOK_CMD:
            case TOK_EOF:
                parser->unexpected_token = true;
                break;
            }
            break;

        case PS_COMMAND:
            switch (parser->token.kind)
            {
            case TOK_CMD:
                parser->state = PS_LINE_START;
                break;
            case TOK_NONE:
            case TOK_COMMENT:
            case TOK_PATTERN:
            case TOK_SEMI:
            case TOK_COLON:
            case TOK_EOF:
                parser->unexpected_token = true;
                break;
            }
            break;
        }

        parser->is_done =
            parser->token.kind == TOK_EOF || parser->unexpected_token;
    }

    if (parser->unexpected_token)
    {
        if (parser->token.kind == TOK_EOF)
        {
            klog(LL_ERROR, "Unexpected end of file");
        }
        else
        {
            struct file_pos pos =
                find_file_pos(parser->full_text, parser->tail.ptr);
            klog(
                LL_ERROR,
                "Unexpected token at line %u col %u: '%.*s'",
                pos.line + 1,
                pos.column + 1,
                str_format_args(parser->token.str)
            );
        }
    }
}

static enum error format_and_run( //
    struct str const cmd_pattern,
    char const *const filename
)
{
    enum error err = OK;

    // Format: Replace "%" with filename

    struct cstrbuf cmd = {0};

    struct str head = {0};
    struct str tail = cmd_pattern;

    bool is_split;

    do
    {
        is_split = str_split_delims(tail, "%", &head, &tail);

        if (!cstrbuf_extend_str(&cmd, head))
        {
            err = out_of_memory();
            goto done;
        }

        if (is_split)
        {
            if (!cstrbuf_extend_cstr(&cmd, filename))
            {
                err = out_of_memory();
                goto done;
            }
        }
    } while (is_split);

    // Run

    klog(LL_INFO, "Running: %s", cmd.ptr);
    int exitcode = system(cmd.ptr);

    if (exitcode != 0)
    {
        klog(LL_WARN, "Command non-zero exit code: %d", exitcode);
    }

done:
    cstrbuf_deinit(&cmd);
    return err;
}

static enum error evaluate( //
    char const *const filename,
    char const *const config_filename
)
{
    assert(filename);
    assert(config_filename);

    struct cstrbuf config_str = {0};
    enum error err = cstrbuf_init_from_file(&config_str, config_filename);
    if (err)
    {
        goto done;
    }

    struct fnmar_parser parser = {0};
    fnmar_parser_start(&parser, cstrbuf_to_str(config_str));

    bool found_match = false;

    while (!parser.is_done)
    {
        fnmar_parser_next(&parser);

        if (!found_match && parser.token.kind == TOK_PATTERN)
        {
            char c;
            char const *pattern = str_into_cstr_unsafe(parser.token.str, &c);

            found_match = 0 == fnmatch(pattern, filename, 0);

            if (found_match)
            {
                klog(LL_DEBUG, "'%s' matched pattern '%s'", filename, pattern);
            }
            else
            {
                klog(
                    LL_DEBUG,
                    "'%s' did not match pattern '%s'",
                    filename,
                    pattern
                );
            }

            str_revert_into_cstr_unsafe(parser.token.str, c);
        }

        if (found_match && parser.token.kind == TOK_CMD)
        {
            err = format_and_run(parser.token.str, filename);
            goto done;
        }
    }

    klog(LL_WARN, "Did not find pattern match for '%s'", filename);
    err = ERR_NO_MATCHES;

done:
    cstrbuf_deinit(&config_str);
    return err;
}

xgen();
struct cli
{
    char const *filename;

    xattr(
        cliopt_attr,
        .name = "--config",
        .short_name = 'c',
        .help = "Config file (default: " DEFAULT_CONFIG_FILENAME ")"
    );
    char const *config_filename;
};
static cliopt_x_from_args_impl(cli);

int main(int const argc, char const *const *const argv)
{
    enum error err = OK;

    log_setup_from_env();

    struct cli cli = {
        .config_filename = DEFAULT_CONFIG_FILENAME,
    };

    struct cliopt_prog const progopts = {
        .name = "fnmar",
    };

    if (!cli_from_args(&cli, argc, argv, progopts))
    {
        err = ERR_ARGS;
        goto done;
    }

    err = evaluate(cli.filename, cli.config_filename);

done:
    return (int)err;
}
