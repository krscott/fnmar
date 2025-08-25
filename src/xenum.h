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
#define xenum_decl_to_cstr(name) char const *name##_to_cstr(enum name const val)
#define xenum_impl_to_cstr(name, KINDS_X)                                      \
    xenum_decl_to_cstr(name)                                                   \
    {                                                                          \
        switch (val)                                                           \
        {                                                                      \
            KINDS_X(X_CASE_TO_CSTR)                                            \
        default:                                                               \
            return "?";                                                        \
        }                                                                      \
    }                                                                          \
    static_assert(1, "")

#endif
