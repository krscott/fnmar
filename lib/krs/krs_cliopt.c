#include "krs_cliopt.h"
#include "krs_cc_ext.h"
#include "krs_log.h"
#include "krs_str.h"
#include "krs_types.h"
#include <errno.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

enum cliopt_kind
{
    CLIOPT_BOOL,
    CLIOPT_CSTR,
    CLIOPT_INT,
};

struct cliopt_option
{
    char const *name;
    char short_name;
    enum cliopt_kind kind;
    bool required;
};

struct cliopt_meta
{
    struct cliopt_option spec;
    bool used;
    char const *arg;
    void *output;
};

static nodiscard bool expects_arg(struct cliopt_option const *opt)
{
    bool out;

    switch (opt->kind)
    {
    case CLIOPT_BOOL:
        out = false;
        break;
    case CLIOPT_CSTR:
    case CLIOPT_INT:
        out = true;
        break;
    default:
        assert(false);
        out = false;
    }

    return out;
}

static nodiscard bool is_positional(struct cliopt_option const *opt)
{
    return opt->name && (opt->name[0] != '-') && (opt->short_name == '\0');
}

static nodiscard bool parse_arg_value( //
    struct cliopt_meta *const meta,
    char const *const arg
)
{
    assert(!meta->used);

    bool success;

    switch (meta->spec.kind)
    {
    case CLIOPT_BOOL:
    {
        *((bool *)meta->output) = true;
        success = true;
    }
    break;
    case CLIOPT_CSTR:
    {
        *((char const **)meta->output) = arg;
        success = true;
    }
    break;
    case CLIOPT_INT:
    {
        char *tail;

        errno = 0;

        *((i64 *)meta->output) = (i64)strtoll(arg, &tail, 10);

        if (errno)
        {
            logf(LL_FATAL, "%s: %s", strerror(errno), arg);
            success = false;
        }
        else if (*tail != '\0')
        {
            logf(LL_FATAL, "Not an integer: %s", arg);
            success = false;
        }
        else
        {
            success = true;
        }
    }
    break;
    default:
    {
        assert(false);
        success = false;
    }
    break;
    }

    meta->used = true;
    return success;
}

struct cliopt_options
{
    struct cliopt_meta *ptr;
    size_t len;
};

static bool nodiscard get_next_positional( //
    struct cliopt_options const opts,
    struct cliopt_meta **const out
)
{
    bool success = false;

    for (u32 i = 0; i < opts.len; ++i)
    {
        struct cliopt_meta *const meta = &opts.ptr[i];
        if (!meta->used && is_positional(&meta->spec))
        {
            *out = meta;
            success = true;
            break;
        }
    }

    if (!success)
    {
        logf(LL_FATAL, "Too many positional arguments");
    }
    return success;
}

static bool get_short( //
    struct cliopt_options const opts,
    char const short_name,
    struct cliopt_meta **const out
)
{
    bool success = false;

    for (u32 i = 0; i < opts.len; ++i)
    {
        struct cliopt_meta *const meta = &opts.ptr[i];
        if (meta->spec.short_name == short_name)
        {
            *out = meta;
            success = true;
            break;
        }
    }

    if (!success)
    {
        logf(LL_FATAL, "Unrecognized option '-%c'", short_name);
    }
    return success;
}

static bool get_long( //
    struct cliopt_options const opts,
    struct sv const long_name,
    struct cliopt_meta **const out
)
{
    bool success = false;

    for (u32 i = 0; i < opts.len; ++i)
    {
        struct cliopt_meta *const meta = &opts.ptr[i];
        if (strncmp(meta->spec.name, long_name.ptr, long_name.len) == 0)
        {
            *out = meta;
            success = true;
            break;
        }
    }

    if (!success)
    {
        logf(
            LL_FATAL,
            "Unrecognized option '%.*s'",
            str_format_args(long_name)
        );
    }
    return success;
}

struct args
{
    char const *const *ptr;
    size_t len;
};

static nodiscard bool parse_args( //
    struct cliopt_options const opts,
    int const argc,
    char const *const *argv
)
{
    bool success = false;
    bool positional_only = false;

    int i = 0;
    while (i < argc)
    {
        char const *arg = argv[i++];

        if (positional_only || *arg != '-')
        {
            struct cliopt_meta *meta;
            if (get_next_positional(opts, &meta))
            {
                success = parse_arg_value(meta, arg);
            }
            else
            {
                success = false;
                goto done;
            }
        }
        else if (*(++arg) != '-')
        {
            while (*arg != '\0')
            {
                // loop over combined bools, e.g. `-abc123`

                struct cliopt_meta *meta;
                if (get_short(opts, *arg++, &meta))
                {
                    if (expects_arg(&meta->spec))
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
                                    LL_FATAL,
                                    "-%c requires an argument",
                                    meta->spec.short_name
                                );
                                success = false;
                                goto done;
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

                        success = parse_arg_value(meta, arg);
                        break;
                    }
                }
                else
                {
                    success = false;
                    goto done;
                }
            }
        }
        else if (strcmp(arg, "--") == 0)
        {
            positional_only = true;
        }
        else
        {
            struct cliopt_meta *meta;

            struct sv long_arg = sv_from_cstr(arg);

            if (get_long(opts, long_arg, &meta))
            {
                if (expects_arg(&meta->spec))
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
                                LL_FATAL,
                                "%s requires an argument",
                                meta->spec.name
                            );
                            success = false;
                            goto done;
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

                    success = parse_arg_value(meta, arg);
                    break;
                }
            }
            else
            {
                success = false;
                goto done;
            }
        }
    }

done:
    return success;
}
