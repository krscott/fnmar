#ifndef XENUM_H_
#define XENUM_H_

#define X_ENUM_VAR(x) x,

#define ENUM_DEF(name, KINDS_X)                                                \
    enum name                                                                  \
    {                                                                          \
        KINDS_X(X_ENUM_VAR)                                                    \
    }

#define X_CASE_TO_CSTR(x)                                                      \
    case x:                                                                    \
        return #x;
#define ENUM_IMPL_TO_CSTR(name, KINDS_X)                                       \
    char const *name##_to_cstr(enum name const val)                            \
    {                                                                          \
        switch (val)                                                           \
        {                                                                      \
            KINDS_X(X_CASE_TO_CSTR)                                            \
        default:                                                               \
            return "(unknown)";                                                \
        }                                                                      \
    }

#define X_CASE_DEBUG_PRINT(x)                                                  \
    case x:                                                                    \
        printf(#x);                                                            \
        break;
#define ENUM_IMPL_DEBUG_PRINT(name, KINDS_X)                                   \
    void name##_debug_print(enum name const val)                               \
    {                                                                          \
        switch (val)                                                           \
        {                                                                      \
            KINDS_X(X_CASE_DEBUG_PRINT)                                        \
        default:                                                               \
            printf("(invalid %u)", val);                                       \
            break;                                                             \
        }                                                                      \
    }

#endif
