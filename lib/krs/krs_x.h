#ifndef KRS_X_H_
#define KRS_X_H_

#include "prexy.h"
#include <assert.h>

prexy_tag(x_enum_to_cstr);

#define X_CASE_TO_CSTR(x)                                                      \
    case x:                                                                    \
        return #x;

#define x_enum_to_cstr_decl(name)                                              \
    char const *name##_to_cstr(enum name const val)

#define x_enum_to_cstr_impl(name, X)                                           \
    x_enum_to_cstr_decl(name)                                                  \
    {                                                                          \
        switch (val)                                                           \
        {                                                                      \
            X(X_CASE_TO_CSTR)                                                  \
        default:                                                               \
            assert(false && "Invalid enum " #name " value");                   \
            return "?";                                                        \
        }                                                                      \
    }

#endif
