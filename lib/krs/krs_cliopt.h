#ifndef KRS_CLIOPT_H_
#define KRS_CLIOPT_H_

#include "krs_cc_ext.h"
#include "prexy.h"
#include <stdbool.h>
#include <stddef.h>

enum cliopt_kind
{
    CLIOPT_NONE,
    CLIOPT_BOOL,
    CLIOPT_STRING,
    CLIOPT_INT,
};

struct cliopt
{
    char const *name;
    char const *argname;
    char const *help;
    char short_name;
    bool required;
    bool sufficient;
};

struct cliopt_meta
{
    struct cliopt spec;
    enum cliopt_kind kind;
    char const *ident_name;
    void *output;
    bool used;
};

struct cliopt_prog
{
    char const *name;
};

struct cliopt_options
{
    struct cliopt_meta *ptr;
    size_t len;
};

nodiscard bool cliopt_parse_args( //
    struct cliopt_options const opts,
    int const argc,
    char const *const *argv,
    struct cliopt_prog progopts
);
nodiscard bool cliopt_print_usage( //
    struct cliopt_options opts,
    struct cliopt_prog progopts
);
nodiscard bool cliopt_print_help(struct cliopt_options opts);

prexy_tag(cliopt_from_args);

#define CLIOPT_FROM_ARGS_cliopt(type, varname, ...)                            \
    ((struct cliopt_meta){                                                     \
        .spec =                                                                \
            (struct cliopt){                                                   \
                __VA_ARGS__,                                                   \
            },                                                                 \
        .kind = _Generic(                                                      \
            (cli_data->varname),                                               \
            bool: CLIOPT_BOOL,                                                 \
            char const *: CLIOPT_STRING,                                       \
            i64: CLIOPT_INT                                                    \
        ),                                                                     \
        .ident_name = #varname,                                                \
        .output = &cli_data->varname,                                          \
    })
#define CLIOPT_FROM_ARGS_simple(type, varname)                                 \
    CLIOPT_FROM_ARGS_cliopt(type, varname, 0)

#define CLIOPT_FROM_ARGS_SELECT(fkind, ...)                                    \
    CLIOPT_FROM_ARGS_##fkind(__VA_ARGS__),

#define cliopt_from_args_decl(tname)                                           \
    nodiscard bool tname##_from_args(                                          \
        struct tname *cli_data,                                                \
        int argc,                                                              \
        char const *const *argv,                                               \
        struct cliopt_prog progopts                                            \
    )
#define cliopt_from_args_impl(tname, X)                                        \
    cliopt_from_args_decl(tname)                                               \
    {                                                                          \
        bool help = false;                                                     \
                                                                               \
        struct cliopt_meta opts_arr[] = {                                      \
            ((struct cliopt_meta){                                             \
                .spec =                                                        \
                    (struct cliopt){                                           \
                        .name = "--help",                                      \
                        .short_name = 'h',                                     \
                        .sufficient = true,                                    \
                        .help = "Show help and exit",                          \
                    },                                                         \
                .kind = CLIOPT_BOOL,                                           \
                .ident_name = "help",                                          \
                .output = &help,                                               \
            }),                                                                \
            X(CLIOPT_FROM_ARGS_SELECT)                                         \
        };                                                                     \
                                                                               \
        struct cliopt_options opts = {                                         \
            .ptr = opts_arr,                                                   \
            .len = ARRAY_LENGTH(opts_arr),                                     \
        };                                                                     \
                                                                               \
        bool ok = cliopt_parse_args(opts, argc, argv, progopts);               \
        if (ok && help)                                                        \
        {                                                                      \
            (void)!cliopt_print_usage(opts, progopts);                         \
            (void)!cliopt_print_help(opts);                                    \
            exit(0);                                                           \
        }                                                                      \
        return ok;                                                             \
    }

#endif
