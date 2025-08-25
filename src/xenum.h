#ifndef XENUM_H_
#define XENUM_H_

#define X_ENUM_VAR(x) x,

#define xenum(name, KINDS_X)                                                   \
    enum name                                                                  \
    {                                                                          \
        KINDS_X(X_ENUM_VAR)                                                    \
    }

#define X_CASE_TO_CSTR(x)                                                      \
    case x:                                                                    \
        return #x;
#define xenum_impl_to_cstr(name, KINDS_X)                                      \
    char const *name##_to_cstr(enum name const val)                            \
    {                                                                          \
        switch (val)                                                           \
        {                                                                      \
            KINDS_X(X_CASE_TO_CSTR)                                            \
        default:                                                               \
            return "(unknown)";                                                \
        }                                                                      \
    }                                                                          \
    static_assert(1, "")

#define X_CASE_DEBUG_PRINT(x)                                                  \
    case x:                                                                    \
        printf(#x);                                                            \
        break;
#define xenum_decl_debug_print(name)                                           \
    void name##_debug_print(enum name const val)
#define xenum_impl_debug_print(name, KINDS_X)                                  \
    xenum_decl_debug_print(name)                                               \
    {                                                                          \
        switch (val)                                                           \
        {                                                                      \
            KINDS_X(X_CASE_DEBUG_PRINT)                                        \
        default:                                                               \
            printf("(invalid %u)", val);                                       \
            break;                                                             \
        }                                                                      \
    }                                                                          \
    static_assert(1, "")

#endif
