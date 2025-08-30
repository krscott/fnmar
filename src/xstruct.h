#ifndef XSTRUCT_H_
#define XSTRUCT_H_

#include "prelude.h"
#include <assert.h>

#define XSTRUCT_DECL_FIELD(varname, type) type varname;
#define XSTRUCT_DECL_STRUCT(varname, name) struct name varname;
#define XSTRUCT_DECL_FIELD_ARRAY(varname, type, len) type varname[len];
#define XSTRUCT_DECL_STRUCT_ARRAY(varname, name, len) struct name varname[len];

#define xstruct(name, FIELDS_X)                                                \
    struct name                                                                \
    {                                                                          \
        FIELDS_X(                                                              \
            XSTRUCT_DECL_FIELD,                                                \
            XSTRUCT_DECL_STRUCT,                                               \
            XSTRUCT_DECL_FIELD_ARRAY,                                          \
            XSTRUCT_DECL_STRUCT_ARRAY                                          \
        )                                                                      \
    }

#define XSTRUCT_PRINT_F(varname, type)                                         \
    {                                                                          \
        printf("." #varname " = ");                                            \
        printf(BASIC_TYPE_FORMAT((x)->varname), (x)->varname);                 \
        printf(", ");                                                          \
    }

#define XSTRUCT_PRINT_S(varname, name)                                         \
    {                                                                          \
        name##_print(&(x)->varname);                                           \
    }

#define XSTRUCT_PRINT_FA(varname, type, len)                                   \
    {                                                                          \
        printf("." #varname " = { ");                                          \
        for (u32 i = 0; i < len; ++i)                                          \
        {                                                                      \
            printf(BASIC_TYPE_FORMAT((x)->varname[i]), (x)->varname[i]);       \
            printf(", ");                                                      \
        }                                                                      \
        printf("}");                                                           \
    }

#define XSTRUCT_PRINT_SA(varname, name, len)                                   \
    {                                                                          \
        printf("." #varname " = { ");                                          \
        for (u32 i = 0; i < len; ++i)                                          \
        {                                                                      \
            name##_print(&(x)->varname[i]);                                    \
        }                                                                      \
        printf("}");                                                           \
    }

#define xstruct_decl_print(name) void name##_print(struct name const *const x)
#define xstruct_impl_print(name, FIELDS_X)                                     \
    xstruct_decl_print(name)                                                   \
    {                                                                          \
        printf("(struct " #name "){ ");                                        \
        FIELDS_X(                                                              \
            XSTRUCT_PRINT_F,                                                   \
            XSTRUCT_PRINT_S,                                                   \
            XSTRUCT_PRINT_FA,                                                  \
            XSTRUCT_PRINT_SA                                                   \
        )                                                                      \
        printf("}");                                                           \
    }                                                                          \
    static_assert(1, "")

#endif
