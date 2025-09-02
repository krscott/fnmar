#include "krs_cliopt.h"
#include "krs_dynamic_array.h"
#include "krs_log.h"
#include "krs_str.h"
#include "krs_types.h"
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// #define cliopt_devf(...) logf(LL_DEV, "cliopt " __VA_ARGS__)
#ifndef cliopt_devf
#define cliopt_devf(...)
#endif

static nodiscard bool expects_arg(struct cliopt_meta const *meta)
{
    bool out;

    switch (meta->kind)
    {
    case CLIOPT_BOOL:
        out = false;
        break;
    case CLIOPT_STRING:
    case CLIOPT_INT:
        out = true;
        break;
    case CLIOPT_NONE:
    default:
        assert(false);
        out = false;
    }

    return out;
}

static nodiscard bool is_positional_arg(struct cliopt_spec const *const opt)
{
    return opt->name && (opt->name[0] != '-') && (opt->short_name == '\0');
}

static nodiscard bool is_long_arg(struct cliopt_spec const *const opt)
{
    return opt->name && (opt->name[0] == '-') && (opt->name[1] == '-') &&
           (opt->name[2] != '\0');
}

static nodiscard bool parse_arg_value( //
    struct cliopt_meta *const meta,
    char const *const arg
)
{
    assert(arg);
    assert(!meta->used);
    assert(meta->output);

    bool ok;

    switch (meta->kind)
    {
    case CLIOPT_NONE:
        assert(false);
        break;
    case CLIOPT_BOOL:
    {
        *((bool *)meta->output) = true;
        ok = true;
    }
    break;
    case CLIOPT_STRING:
    {
        *((char const **)meta->output) = arg;
        ok = true;
    }
    break;
    case CLIOPT_INT:
    {
        char *tail;

        errno = 0;

        *((i64 *)meta->output) = (i64)strtoll(arg, &tail, 10);

        if (errno)
        {
            logf(LL_ERROR, "%s: %s", strerror(errno), arg);
            ok = false;
        }
        else if (*tail != '\0')
        {
            logf(LL_ERROR, "Not an integer: %s", arg);
            ok = false;
        }
        else
        {
            ok = true;
        }
    }
    break;
    default:
    {
        assert(false);
        ok = false;
    }
    break;
    }

    meta->used = true;
    return ok;
}

static bool nodiscard get_next_positional( //
    struct cliopt_options const opts,
    struct cliopt_meta **const out
)
{
    bool ok = false;

    for (u32 i = 0; i < opts.len; ++i)
    {
        struct cliopt_meta *const meta = &opts.ptr[i];
        if (!meta->used && is_positional_arg(&meta->spec))
        {
            *out = meta;
            ok = true;
            cliopt_devf("Got positional arg '%s'", meta->spec.name);
            break;
        }
    }

    if (!ok)
    {
        logf(LL_ERROR, "Too many positional arguments");
    }
    return ok;
}

static bool get_short( //
    struct cliopt_options const opts,
    char const short_name,
    struct cliopt_meta **const out
)
{
    bool ok = false;

    for (u32 i = 0; i < opts.len; ++i)
    {
        struct cliopt_meta *const meta = &opts.ptr[i];
        if (meta->spec.short_name == short_name)
        {
            *out = meta;
            ok = true;
            cliopt_devf("Got short arg '-%c'", meta->spec.short_name);
            break;
        }
    }

    if (!ok)
    {
        logf(LL_ERROR, "Unrecognized option '-%c'", short_name);
    }
    else if ((*out)->used)
    {
        logf(LL_ERROR, "Option '-%c' already used", short_name);
        ok = false;
    }

    return ok;
}

static bool get_long( //
    struct cliopt_options const opts,
    struct sv const long_name,
    struct cliopt_meta **const out
)
{
    bool ok = false;

    for (u32 i = 0; i < opts.len; ++i)
    {
        struct cliopt_meta *const meta = &opts.ptr[i];
        if (is_long_arg(&meta->spec) &&
            sv_equal_cstr(long_name, meta->spec.name))
        {
            *out = meta;
            ok = true;
            cliopt_devf("Got long arg '%s'", meta->spec.name);
            break;
        }
    }

    if (!ok)
    {
        logf(
            LL_ERROR,
            "Unrecognized option '%.*s'",
            str_format_args(long_name)
        );
    }
    else if ((*out)->used)
    {
        logf(
            LL_ERROR,
            "Option '%.*s' already used",
            str_format_args(long_name)
        );
        ok = false;
    }

    return ok;
}

// static void print_usage(void)
// {
//     printf("Usage: fnmar [-h] [-c configfile] file\n");
// }

struct optstrs
{
    struct cstrbuf *ptr;
    size_t len;
    size_t cap;
};

struct helpstrs
{
    char const **ptr;
    size_t len;
    size_t cap;
};

bool cliopt_print_usage(
    struct cliopt_options const opts, struct cliopt_prog const progopts
)
{
    size_t const N = 200;
    bool ok = true;
    bool any_optional_options = false;
    struct cstrbuf usage = {0};

    for (size_t i = 0; i < opts.len; ++i)
    {
        struct cliopt_spec const spec = opts.ptr[i].spec;
        bool const has_arg = expects_arg(&opts.ptr[i]);

        if (is_positional_arg(&spec))
        {
            if (spec.required)
            {
                cstrbuf_snprintf(&ok, &usage, N, "%s ", spec.name);
                if (!ok)
                {
                    goto done;
                }
            }
            else
            {
                cstrbuf_snprintf(&ok, &usage, N, "[%s] ", spec.name);
                if (!ok)
                {
                    goto done;
                }
            }
        }
        else
        {
            if (spec.required)
            {
                if (spec.short_name)
                {
                    cstrbuf_snprintf(&ok, &usage, N, "-%c ", spec.short_name);
                    if (!ok)
                    {
                        goto done;
                    }
                }
                else
                {
                    cstrbuf_snprintf(&ok, &usage, N, "%s ", spec.name);
                    if (!ok)
                    {
                        goto done;
                    }
                }

                if (has_arg)
                {
                    cstrbuf_snprintf(&ok, &usage, N, "%s ", spec.argname);
                    if (!ok)
                    {
                        goto done;
                    }
                }
            }
            else
            {
                any_optional_options = true;
            }
        }
    }

    printf("Usage: %s", progopts.name);
    if (any_optional_options)
    {
        printf(" [options]");
    }
    printf(" %s\n", usage.ptr);

done:
    cstrbuf_deinit(&usage);
    return ok;
}

bool cliopt_print_help(struct cliopt_options const opts)
{
    size_t const N = 200;
    char const *SEP = "\1";

    bool ok = true;
    struct optstrs pos_lines = {0};
    struct optstrs opt_lines = {0};

    for (size_t i = 0; i < opts.len; ++i)
    {
        struct cliopt_spec const spec = opts.ptr[i].spec;
        bool const has_arg = expects_arg(&opts.ptr[i]);

        if (is_positional_arg(&spec))
        {
            struct cstrbuf *line;
            ok = da_emplace_uninit(&pos_lines, &line);
            if (!ok)
            {
                goto done;
            }
            *line = (struct cstrbuf){0};

            ok = cstrbuf_extend_cstr(line, spec.name);
            if (!ok)
            {
                goto done;
            }

            cstrbuf_snprintf(&ok, line, N, "%s%s", SEP, spec.help);
            if (!ok)
            {
                goto done;
            }
        }
        else
        {
            struct cstrbuf *line;
            ok = da_emplace_uninit(&opt_lines, &line);
            if (!ok)
            {
                goto done;
            }
            *line = (struct cstrbuf){0};

            if (spec.short_name)
            {
                cstrbuf_snprintf(&ok, line, N, "-%c", spec.short_name);
                if (!ok)
                {
                    goto done;
                }
            }

            if (spec.short_name && spec.name)
            {
                ok = cstrbuf_extend_cstr(line, ", ");
                if (!ok)
                {
                    goto done;
                }
            }

            if (spec.name)
            {
                ok = cstrbuf_extend_cstr(line, spec.name);
                if (!ok)
                {
                    goto done;
                }
            }

            if (has_arg)
            {
                cstrbuf_snprintf(&ok, line, N, " %s", spec.argname);
                if (!ok)
                {
                    goto done;
                }
            }

            cstrbuf_snprintf(&ok, line, N, "%s%s", SEP, spec.help);
            if (!ok)
            {
                goto done;
            }
        }
    }

    int max_len = 0;
    for (size_t i = 0; i < pos_lines.len; ++i)
    {
        struct str head = str_empty();
        if (str_split_delims(
                cstrbuf_to_str(pos_lines.ptr[i]),
                SEP,
                &head,
                NULL
            ))
        {
            max_len = MAX(max_len, (int)head.len);
        }
        else
        {
            assert(false);
        }
    }
    for (size_t i = 0; i < opt_lines.len; ++i)
    {
        struct str head = str_empty();
        if (str_split_delims(
                cstrbuf_to_str(opt_lines.ptr[i]),
                SEP,
                &head,
                NULL
            ))
        {
            max_len = MAX(max_len, (int)head.len);
        }
        else
        {
            assert(false);
        }
    }

    for (size_t i = 0; i < pos_lines.len; ++i)
    {
        if (i == 0)
        {
            printf("\n");
            printf("Positional arguments:\n");
        }

        struct str head = str_empty();
        struct str tail = str_empty();
        if (str_split_delims(
                cstrbuf_to_str(pos_lines.ptr[i]),
                SEP,
                &head,
                &tail
            ))
        {
            printf(
                "  %-*.*s    %.*s\n",
                max_len,
                str_format_args(head),
                str_format_args(tail)
            );
        }
        else
        {
            assert(false);
        }
    }
    for (size_t i = 0; i < opt_lines.len; ++i)
    {
        if (i == 0)
        {
            printf("\n");
            printf("Options:\n");
        }

        struct str head = str_empty();
        struct str tail = str_empty();
        if (str_split_delims(
                cstrbuf_to_str(opt_lines.ptr[i]),
                SEP,
                &head,
                &tail
            ))
        {
            printf(
                "  %-*.*s    %.*s\n",
                max_len,
                str_format_args(head),
                str_format_args(tail)
            );
        }
        else
        {
            assert(false);
        }
    }

done:
    for (size_t i = 0; i < pos_lines.len; ++i)
    {
        cstrbuf_deinit(&pos_lines.ptr[i]);
    }
    da_deinit(&pos_lines);
    for (size_t i = 0; i < opt_lines.len; ++i)
    {
        cstrbuf_deinit(&opt_lines.ptr[i]);
    }
    da_deinit(&opt_lines);

    return ok;
}

bool cliopt_parse_args( //
    struct cliopt_options opts,
    int const argc,
    char const *const *argv,
    struct cliopt_prog progopts
)
{
    cliopt_devf("skipping arg 0: %s", argv[0]);
    cliopt_devf("parsing %d args", argc - 1);

    // Fixup defaults
    if (!progopts.name)
    {
        progopts.name = argv[0];
    }

    for (size_t i = 0; i < opts.len; ++i)
    {
        struct cliopt_meta *const meta = &opts.ptr[i];

        // If no spec provided, make positional arg based on ident_name
        if (!meta->spec.name && !meta->spec.short_name)
        {
            meta->spec.name = meta->ident_name;
            meta->spec.required = true;
        }

        if (!meta->spec.help)
        {
            meta->spec.help = "";
        }

        if (!meta->spec.argname)
        {
            meta->spec.argname = "VAL";
        }
    }

    // Debug checking
#ifndef NDEBUG
    for (size_t i = 0; i < opts.len; ++i)
    {
        struct cliopt_meta const *const meta = &opts.ptr[i];

        if (!meta->spec.name && !meta->spec.short_name)
        {
            logf(
                LL_FATAL,
                "Option must define at least one of 'name', 'short_name', "
                "or define meta 'ident_name'"
            );
            assert(false);
        }

        char short_name_cstr[] = "-x";
        short_name_cstr[1] = meta->spec.short_name;

        char const *const opt_name =
            meta->spec.name ? meta->spec.name : short_name_cstr;

        if (meta->kind == CLIOPT_NONE)
        {
            logf(LL_FATAL, "Option '%s' has no kind set", opt_name);
            assert(false);
        }

        if (!meta->output)
        {
            logf(LL_FATAL, "Option '%s' output ptr is NULL", opt_name);
            assert(false);
        }

        if (meta->spec.name && meta->spec.short_name != '\0')
        {
            // If an arg has a short flag, it must not use a positional name
            if (!is_long_arg(&meta->spec))
            {
                logf(
                    LL_FATAL,
                    "Short option '-%c' with positional name '%s' (change to "
                    "'--%s')",
                    meta->spec.short_name,
                    meta->spec.name,
                    meta->spec.name
                );
                assert(false);
            }
        }
    }
#endif

    bool ok = true;
    bool positional_only = false;

    for (int i = 1; ok && i < argc;)
    {
        char const *const full_arg = argv[i++];

        cliopt_devf("arg: %s", full_arg);

        if (positional_only || full_arg[0] != '-')
        {
            // positional arg
            cliopt_devf("positional");

            struct cliopt_meta *meta;
            if (get_next_positional(opts, &meta))
            {
                ok = parse_arg_value(meta, full_arg);
            }
            else
            {
                ok = false;
            }
            cliopt_devf("positional ok=%d", ok);
        }
        else if (full_arg[1] != '-')
        {
            // short arg
            cliopt_devf("short");

            char const *arg = &full_arg[1];

            while (ok && *arg != '\0')
            {
                // loop over combined bools, e.g. `-abc123`

                struct cliopt_meta *meta;
                if (get_short(opts, *arg++, &meta))
                {
                    if (expects_arg(meta))
                    {
                        if (*arg == '\0')
                        {
                            // separate arg e.g. `-a 123`
                            if (i < argc)
                            {
                                arg = argv[i++];
                            }
                            else
                            {
                                logf(
                                    LL_ERROR,
                                    "-%c requires an argument",
                                    meta->spec.short_name
                                );
                                ok = false;
                            }
                        }
                        else if (*arg == '=')
                        {
                            // equals arg e.g. `-a=123`
                            ++arg;
                        }
                        else
                        {
                            // smooshed arg e.g. `-a123`
                        }

                        ok = ok && parse_arg_value(meta, arg);
                        break;
                    }
                    else
                    {
                        ok = parse_arg_value(meta, "");
                    }
                }
                else
                {
                    ok = false;
                }
            }
            cliopt_devf("short ok=%d", ok);
        }
        else if (strcmp(full_arg, "--") == 0)
        {
            // positional split arg `--`

            positional_only = true;

            cliopt_devf("switch to positional-only");
        }
        else
        {
            // long arg
            cliopt_devf("long");

            struct cliopt_meta *meta;

            struct sv long_arg;
            struct sv tail;
            bool const using_equals =
                sv_split_delims(sv_from_cstr(full_arg), "=", &long_arg, &tail);

            if (get_long(opts, long_arg, &meta))
            {
                if (expects_arg(meta))
                {
                    if (using_equals)
                    {
                        // equals arg e.g. `--arg=123`

                        // NOTE: This is sound because tail was split from cstr
                        assert(tail.ptr[tail.len] == '\0');
                        ok = parse_arg_value(meta, tail.ptr);
                    }
                    else
                    {
                        // separate arg e.g. `--arg 123`
                        if (i < argc)
                        {
                            ok = parse_arg_value(meta, argv[i++]);
                        }
                        else
                        {
                            logf(
                                LL_ERROR,
                                "-%c requires an argument",
                                meta->spec.short_name
                            );
                            ok = false;
                        }
                    }
                }
                else
                {
                    ok = parse_arg_value(meta, "");
                }
            }
            else
            {
                ok = false;
            }

            cliopt_devf("long ok=%d", ok);
        }
    }

    {
        // Missing argument check. Pass if either condition is true:
        // - Any "sufficient" arg is used
        // - Any "required" arg is not used

        bool any_sufficient = false;

        for (size_t i = 0; ok && i < opts.len; ++i)
        {
            struct cliopt_meta const *const meta = &opts.ptr[i];

            if (meta->spec.sufficient && meta->used)
            {
                cliopt_devf("provided opt index %u is sufficient", (unsigned)i);
                any_sufficient = true;
                break;
            }
        }

        if (!any_sufficient)
        {
            for (size_t i = 0; ok && i < opts.len; ++i)
            {
                struct cliopt_meta const *const meta = &opts.ptr[i];

                if (meta->spec.required && !meta->used)
                {
                    logf(
                        LL_ERROR,
                        "Missing required argument '%s'",
                        meta->spec.name
                    );
                    ok = false;
                }
            }
        }
    }

    if (!ok)
    {
        (void)!cliopt_print_usage(opts, progopts);
    }

    return ok;
}
