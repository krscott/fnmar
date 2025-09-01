#ifndef KRS_CLIOPT_H_
#define KRS_CLIOPT_H_

#include "krs_cc_ext.h"
#include <stdbool.h>
#include <stddef.h>

enum cliopt_kind
{
    CLIOPT_BOOL,
    CLIOPT_STRING,
    CLIOPT_INT,
};

struct cliopt_option
{
    char const *name;
    char short_name;
    enum cliopt_kind kind;
    // bool required; // TODO
};

struct cliopt_meta
{
    struct cliopt_option spec;
    bool used;
    char const *arg;
    void *output;
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

#endif
