#ifndef KRS_X_FPRINTF_REPR_H_
#define KRS_X_FPRINTF_REPR_H_

#include "krs_types.h"
#include "krs_x.h"
#include <stdio.h>

#define X_STRUCT_FPRINT_REPR_xf_simple(type, varname)                          \
    {                                                                          \
        fprintf(stream, "." #varname " = ");                                   \
        fprintf(stream, BASIC_TYPE_FORMAT((x)->varname), (x)->varname);        \
        fprintf(stream, ", ");                                                 \
    }

#define X_STRUCT_FPRINT_REPR_xf_simple_array(type, varname)                    \
    {                                                                          \
        type##_fprint_repr(stream, &(x)->varname);                             \
    }

#define X_STRUCT_FPRINT_REPR_xf_struct(type, varname)                          \
    {                                                                          \
        fprintf(stream, "." #varname " = ");                                   \
        type##_fprint_repr(stream, &(x)->varname);                             \
        fprintf(stream, ", ");                                                 \
    }

#define X_STRUCT_FPRINT_REPR_xf_struct_array(type, varname, len)               \
    {                                                                          \
        fprintf(stream, "." #varname " = { ");                                 \
        for (u32 i = 0; i < len; ++i)                                          \
        {                                                                      \
            type##_fprint_repr(stream, &(x)->varname[i]);                      \
            fprintf(stream, ", ");                                             \
        }                                                                      \
        fprintf(stream, "}");                                                  \
    }

#define X_STRUCT_FPRINT_REPR_xf_enum(type, varname)                            \
    {                                                                          \
        fprintf(stream, "." #varname " = ");                                   \
        fprintf(stream, "%s", type##_to_cstr((x)->varname));                   \
        fprintf(stream, ", ");                                                 \
    }

#define X_STRUCT_FPRINT_REPR_xf_enum_array(type, varname, len)                 \
    {                                                                          \
        fprintf(stream, "." #varname " = { ");                                 \
        for (u32 i = 0; i < len; ++i)                                          \
        {                                                                      \
            fprintf(stream, "%s, ", type##_to_cstr((x)->varname[i]));          \
        }                                                                      \
        fprintf(stream, "}");                                                  \
    }

#define X_STRUCT_FPRINT_REPR(fkind, ...)                                       \
    X_STRUCT_FPRINT_REPR_##fkind(__VA_ARGS__);

#define x_struct_fprint_repr_decl(name)                                        \
    void name##_fprint_repr(FILE *stream, struct name const *x)
#define x_struct_fprint_repr_impl(name, FIELDS_X)                              \
    x_struct_fprint_repr_decl(name)                                            \
    {                                                                          \
        fprintf(stream, "(struct " #name "){ ");                               \
        FIELDS_X(X_STRUCT_FPRINT_REPR)                                         \
        fprintf(stream, "}");                                                  \
    }                                                                          \
    static_assert(1, "")

#endif
