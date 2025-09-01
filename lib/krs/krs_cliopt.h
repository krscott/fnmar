#ifndef KRS_CLIOPT_H_
#define KRS_CLIOPT_H_

#include "krs_cc_ext.h"
#include "krs_x.h"
#include <stdbool.h>
#include <stddef.h>

enum cliopt_kind
{
    CLIOPT_NONE,
    CLIOPT_BOOL,
    CLIOPT_STRING,
    CLIOPT_INT,
};

struct cliopt_spec
{
    char const *name;
    char short_name;
    bool required;
    bool sufficient;
};

struct cliopt_meta
{
    struct cliopt_spec spec;
    enum cliopt_kind kind;
    char const *ident_name;
    void *output;
    bool used;
};

struct cliopt_options
{
    struct cliopt_meta *ptr;
    size_t len;
};

nodiscard bool cliopt_parse_args( //
    struct cliopt_options const opts,
    int const argc,
    char const *const *argv
);

#define CLIOPT_X_FIELD_xf_simple_attr(type, varname, ...)                      \
    (struct cliopt_meta)                                                       \
    {                                                                          \
        .spec = (struct cliopt_spec){__VA_ARGS__},                             \
        .kind = _Generic(                                                      \
            (cli_data->varname),                                               \
            bool: CLIOPT_BOOL,                                                 \
            char const *: CLIOPT_STRING,                                       \
            i64: CLIOPT_INT                                                    \
        ),                                                                     \
        .ident_name = #varname, .output = &cli_data->varname,                  \
    }
#define CLIOPT_X_FIELD_xf_simple(type, varname)                                \
    CLIOPT_X_FIELD_xf_simple_attr(type, varname, 0)

#define CLIOPT_X_FIELD(fkind, ...) CLIOPT_X_FIELD_##fkind(__VA_ARGS__),

#define cliopt_x_from_args_decl(name)                                          \
    nodiscard bool name##_from_args(                                           \
        struct name *cli_data,                                                 \
        int argc,                                                              \
        char const *const *argv                                                \
    )
#define cliopt_x_from_args_impl(name, FIELDS_X)                                \
    cliopt_x_from_args_decl(name)                                              \
    {                                                                          \
        struct cliopt_meta opts_arr[] = {FIELDS_X(CLIOPT_X_FIELD)};            \
        struct cliopt_options opts = {                                         \
            .ptr = opts_arr,                                                   \
            .len = ARRAY_LENGTH(opts_arr),                                     \
        };                                                                     \
        return cliopt_parse_args(opts, argc, argv);                            \
    }                                                                          \
    static_assert(1, "")

#endif
