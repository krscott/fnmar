#ifndef KRS_LOG_H_
#define KRS_LOG_H_

#include "krs_cc_ext.h"
#include <stdio.h>

enum log_level
{
    LL_DEV,
    LL_FATAL,
    LL_ERROR,
    LL_WARN,
    LL_INFO,
    LL_DEBUG
};

#define klog(level, ...)                                                       \
    do                                                                         \
    {                                                                          \
        if (level <= log_level_printed)                                        \
        {                                                                      \
            fprintf(stderr, "%-5s ", log_level_to_cstr(level));                \
            fprintf(stderr, __VA_ARGS__);                                      \
            fprintf(stderr, "\n");                                             \
        }                                                                      \
    } while (0)

nodiscard char const *log_level_to_cstr(enum log_level const ll);
void log_setup_from_env(void);

extern enum log_level log_level_printed;

#endif
