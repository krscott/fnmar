#include "krs_log.h"
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>

#define LOG_ENV_VAR "LOG"

enum log_level log_level_printed = LL_WARN;

char const *log_level_to_cstr(enum log_level const ll)
{
    char const *s;

    switch (ll)
    {
    case LL_DEV:
        s = "TEST";
        break;
    case LL_FATAL:
        s = "FATAL";
        break;
    case LL_ERROR:
        s = "ERROR";
        break;
    case LL_WARN:
        s = "WARN";
        break;
    case LL_INFO:
        s = "INFO";
        break;
    case LL_DEBUG:
    default:
        s = "DEBUG";
        break;
    }

    return s;
}

void log_setup_from_env(void)
{
    char const *log_env = getenv(LOG_ENV_VAR);

    if (log_env)
    {
        if (strcmp(log_env, "critical") == 0)
        {
            log_level_printed = LL_FATAL;
        }
        else if (strcmp(log_env, "error") == 0)
        {
            log_level_printed = LL_ERROR;
        }
        else if (strcmp(log_env, "warn") == 0)
        {
            log_level_printed = LL_WARN;
        }
        else if (strcmp(log_env, "info") == 0)
        {
            log_level_printed = LL_INFO;
        }
        else if (strcmp(log_env, "debug") == 0)
        {
            log_level_printed = LL_DEBUG;
        }
        else
        {
            logf(LL_ERROR, "Unknown log level " LOG_ENV_VAR "=%s", log_env);
        }
    }
}
