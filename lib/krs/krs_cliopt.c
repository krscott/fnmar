#include "krs_cliopt.h"
#include "krs_log.h"
#include "krs_str.h"
#include "krs_types.h"
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define cliopt_devf(...) logf(LL_DEV, "cliopt " __VA_ARGS__)

static nodiscard bool expects_arg(struct cliopt_option const *opt)
{
    bool out;

    switch (opt->kind)
    {
    case CLIOPT_BOOL:
        out = false;
        break;
    case CLIOPT_STRING:
    case CLIOPT_INT:
        out = true;
        break;
    default:
        assert(false);
        out = false;
    }

    return out;
}

static nodiscard bool is_positional_arg(struct cliopt_option const *const opt)
{
    return opt->name && (opt->name[0] != '-') && (opt->short_name == '\0');
}

static nodiscard bool is_long_arg(struct cliopt_option const *const opt)
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

    bool success;

    switch (meta->spec.kind)
    {
    case CLIOPT_BOOL:
    {
        *((bool *)meta->output) = true;
        success = true;
    }
    break;
    case CLIOPT_STRING:
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
            logf(LL_ERROR, "%s: %s", strerror(errno), arg);
            success = false;
        }
        else if (*tail != '\0')
        {
            logf(LL_ERROR, "Not an integer: %s", arg);
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

static bool nodiscard get_next_positional( //
    struct cliopt_options const opts,
    struct cliopt_meta **const out
)
{
    bool success = false;

    for (u32 i = 0; i < opts.len; ++i)
    {
        struct cliopt_meta *const meta = &opts.ptr[i];
        if (!meta->used && is_positional_arg(&meta->spec))
        {
            *out = meta;
            success = true;
            cliopt_devf("Got positional arg '%s'", meta->spec.name);
            break;
        }
    }

    if (!success)
    {
        logf(LL_ERROR, "Too many positional arguments");
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
            cliopt_devf("Got short arg '-%c'", meta->spec.short_name);
            break;
        }
    }

    if (!success)
    {
        logf(LL_ERROR, "Unrecognized option '-%c'", short_name);
    }
    else if ((*out)->used)
    {
        logf(LL_ERROR, "Option '-%c' already used", short_name);
        success = false;
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
        if (is_long_arg(&meta->spec) &&
            strncmp(meta->spec.name, long_name.ptr, long_name.len) == 0)
        {
            *out = meta;
            success = true;
            cliopt_devf("Got long arg '%s'", meta->spec.name);
            break;
        }
    }

    if (!success)
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
        success = false;
    }

    return success;
}

bool cliopt_parse_args( //
    struct cliopt_options const opts,
    int const argc,
    char const *const *argv
)
{
    cliopt_devf("skipping arg 0: %s", argv[0]);
    cliopt_devf("parsing %d args", argc - 1);

#ifndef NDEBUG
    for (size_t i = 0; i < opts.len; ++i)
    {
        struct cliopt_meta const *const meta = &opts.ptr[i];

        if (meta->spec.name && meta->spec.short_name != '\0')
        {
            // If an arg has a short flag, it must not use a positional name
            if (!is_long_arg(&meta->spec))
            {
                logf(
                    LL_ERROR,
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

    bool success = true;
    bool positional_only = false;

    for (int i = 1; success && i < argc;)
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
                success = parse_arg_value(meta, full_arg);
            }
            else
            {
                success = false;
            }
            cliopt_devf("positional success=%d", success);
        }
        else if (full_arg[1] != '-')
        {
            // short arg
            cliopt_devf("short");

            char const *arg = &full_arg[1];

            while (success && *arg != '\0')
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
                                    LL_ERROR,
                                    "-%c requires an argument",
                                    meta->spec.short_name
                                );
                                success = false;
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

                        success = success && parse_arg_value(meta, arg);
                        break;
                    }
                    else
                    {
                        success = parse_arg_value(meta, "");
                    }
                }
                else
                {
                    success = false;
                }
            }
            cliopt_devf("short success=%d", success);
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
                if (expects_arg(&meta->spec))
                {
                    if (using_equals)
                    {
                        // equals arg e.g. `--arg=123`

                        // NOTE: This is sound because tail was split from cstr
                        assert(tail.ptr[tail.len] == '\0');
                        success = parse_arg_value(meta, tail.ptr);
                    }
                    else
                    {
                        // separate arg e.g. `--arg 123`
                        if (i < argc)
                        {
                            success = parse_arg_value(meta, argv[i++]);
                        }
                        else
                        {
                            logf(
                                LL_ERROR,
                                "-%c requires an argument",
                                meta->spec.short_name
                            );
                            success = false;
                        }
                    }
                }
                else
                {
                    success = parse_arg_value(meta, "");
                }
            }
            else
            {
                success = false;
            }

            cliopt_devf("long success=%d", success);
        }
    }

    {
        // Missing argument check. Pass if either condition is true:
        // - Any "sufficient" arg is used
        // - Any "required" arg is not used

        bool any_sufficient = false;

        for (size_t i = 0; success && i < opts.len; ++i)
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
            for (size_t i = 0; success && i < opts.len; ++i)
            {
                struct cliopt_meta const *const meta = &opts.ptr[i];

                if (meta->spec.required && !meta->used)
                {
                    logf(
                        LL_ERROR,
                        "Missing required argument '%s'",
                        meta->spec.name
                    );
                    success = false;
                }
            }
        }
    }

    return success;
}
