#ifndef KRS_X_H_
#define KRS_X_H_

#include <assert.h>

#define X_ENUM_VAR(x) x,

#define x_enum(name, KINDS_X)                                                  \
    enum name                                                                  \
    {                                                                          \
        KINDS_X(X_ENUM_VAR)                                                    \
    }

#define X_CASE_TO_CSTR(x)                                                      \
    case x:                                                                    \
        return #x;
#define x_enum_to_cstr_decl(name)                                              \
    char const *name##_to_cstr(enum name const val)
#define x_enum_to_cstr_impl(name)                                              \
    x_enum_to_cstr_decl(name)                                                  \
    {                                                                          \
        switch (val)                                                           \
        {                                                                      \
            name##_x_variants(X_CASE_TO_CSTR) default                          \
                : assert(false && "Invalid enum " #name " value");             \
            return "?";                                                        \
        }                                                                      \
    }                                                                          \
    static_assert(1, "")

#define X_STRUCT_FIELD_xf_simple(type, varname) type varname
#define X_STRUCT_FIELD_xf_simple_attr(type, varname, ...) type varname
#define X_STRUCT_FIELD_xf_simple_array(type, varname, len) type varname[len]
#define X_STRUCT_FIELD_xf_struct(type, varname) struct type varname
#define X_STRUCT_FIELD_xf_struct_array(type, varname, len)                     \
    struct type varname[len]
#define X_STRUCT_FIELD_xf_enum(type, varname) enum type varname
#define X_STRUCT_FIELD_xf_enum_array(type, varname, len) enum type varname[len]

#define X_STRUCT_FIELD(fkind, ...) X_STRUCT_FIELD_##fkind(__VA_ARGS__);

#define x_struct(name, FIELDS_X)                                               \
    struct name                                                                \
    {                                                                          \
        FIELDS_X(X_STRUCT_FIELD)                                               \
    }

// xgen markers

#define xgen() static_assert(1, "")
#define xattr(name, ...) static_assert(sizeof((struct name){__VA_ARGS__}), "")

#endif
