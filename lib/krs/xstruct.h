#ifndef XSTRUCT_H_
#define XSTRUCT_H_

#include "prelude.h"
#include <assert.h>

#define XSTRUCT_FIELD_xf_simple(type, varname) type varname
#define XSTRUCT_FIELD_xf_simple_array(type, varname, len) type varname[len]
#define XSTRUCT_FIELD_xf_struct(type, varname) struct type varname
#define XSTRUCT_FIELD_xf_struct_array(type, varname, len)                      \
    struct type varname[len]
#define XSTRUCT_FIELD_xf_enum(type, varname) enum type varname
#define XSTRUCT_FIELD_xf_enum_array(type, varname, len) enum type varname[len]

#define XSTRUCT_FIELD(fkind, ...) XSTRUCT_FIELD_##fkind(__VA_ARGS__);

#define xstruct(name, FIELDS_X)                                                \
    struct name                                                                \
    {                                                                          \
        FIELDS_X(XSTRUCT_FIELD)                                                \
    }

#define XSTRUCT_FPRINT_REPR_xf_simple(type, varname)                           \
    {                                                                          \
        fprintf(stream, "." #varname " = ");                                   \
        fprintf(stream, BASIC_TYPE_FORMAT((x)->varname), (x)->varname);        \
        fprintf(stream, ", ");                                                 \
    }

#define XSTRUCT_FPRINT_REPR_xf_simple_array(type, varname)                     \
    {                                                                          \
        type##_fprint_repr(stream, &(x)->varname);                             \
    }

#define XSTRUCT_FPRINT_REPR_xf_struct(type, varname)                           \
    {                                                                          \
        fprintf(stream, "." #varname " = ");                                   \
        type##_fprint_repr(stream, &(x)->varname);                             \
        fprintf(stream, ", ");                                                 \
    }

#define XSTRUCT_FPRINT_REPR_xf_struct_array(type, varname, len)                \
    {                                                                          \
        fprintf(stream, "." #varname " = { ");                                 \
        for (u32 i = 0; i < len; ++i)                                          \
        {                                                                      \
            type##_fprint_repr(stream, &(x)->varname[i]);                      \
            fprintf(stream, ", ");                                             \
        }                                                                      \
        fprintf(stream, "}");                                                  \
    }

#define XSTRUCT_FPRINT_REPR_xf_enum(type, varname)                             \
    {                                                                          \
        fprintf(stream, "." #varname " = ");                                   \
        fprintf(stream, "%s", type##_to_cstr((x)->varname));                   \
        fprintf(stream, ", ");                                                 \
    }

#define XSTRUCT_FPRINT_REPR_xf_enum_array(type, varname, len)                  \
    {                                                                          \
        fprintf(stream, "." #varname " = { ");                                 \
        for (u32 i = 0; i < len; ++i)                                          \
        {                                                                      \
            fprintf(stream, "%s, ", type##_to_cstr((x)->varname[i]));          \
        }                                                                      \
        fprintf(stream, "}");                                                  \
    }

#define XSTRUCT_FPRINT_REPR(fkind, ...)                                        \
    XSTRUCT_FPRINT_REPR_##fkind(__VA_ARGS__);

#define xstruct_decl_fprint_repr(name)                                         \
    void name##_fprint_repr(FILE *stream, struct name const *x)
#define xstruct_impl_fprint_repr(name, FIELDS_X)                               \
    xstruct_decl_fprint_repr(name)                                             \
    {                                                                          \
        fprintf(stream, "(struct " #name "){ ");                               \
        FIELDS_X(XSTRUCT_FPRINT_REPR)                                          \
        fprintf(stream, "}");                                                  \
    }                                                                          \
    static_assert(1, "")

#endif
