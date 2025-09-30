#ifndef PREXY_H_
#define PREXY_H_

#ifndef PREXY_EXPAND
#include <assert.h>
#endif

#define prexy

#define prexy_count(name) (name##_COUNT + (0 * sizeof((enum name)0)))

#define PREXY_CONCAT(x, y) x##y
#define PREXY_CONCAT3(x, y, z) x##y##z

#ifdef PREXY_EXPAND

#define PREXY_ASSERT_IS_TAG(name) PREXY_EXPAND_REMOVE_SEMI
#define PREXY_ASSERT_IS_NAMESPACE(name) PREXY_EXPAND_REMOVE_SEMI
#define PREXY_ASSERT_IS_STRUCT(name) PREXY_EXPAND_REMOVE_SEMI

#define prexy_tag(name) PREXY_EXPAND_REMOVE_SEMI
#define prexy_namespace(name) PREXY_EXPAND_REMOVE_SEMI

#define px_tag(name) PREXY_EXPAND_REMOVE_SEMI
#define px_attr(name, ...) PREXY_EXPAND_REMOVE_SEMI

#else

#define PREXY_ASSERT_IS_TAG(name)                                              \
    static_assert(sizeof((struct name){.prexy_tag__ = '\0'}), "")
#define PREXY_ASSERT_IS_NAMESPACE(name)                                        \
    static_assert(sizeof((struct name){.prexy_namespace__ = '\0'}), "")
#define PREXY_ASSERT_IS_STRUCT(name)                                           \
    static_assert(sizeof(*(struct name *)0), "")
// #define PREXY_ASSERT_IS_NONEMPTY_STRING(s)                                     \
//     static_assert(sizeof("" s "") > 1, "")

#define prexy_tag(name)                                                        \
    struct name                                                                \
    {                                                                          \
        char prexy_tag__;                                                      \
    }
#define prexy_namespace(name)                                                  \
    struct name                                                                \
    {                                                                          \
        char prexy_namespace__;                                                \
    }

#define px_tag(name) PREXY_ASSERT_IS_TAG(name)
#define px_attr(name, ...) static_assert(sizeof((struct name){__VA_ARGS__}), "")

#endif

// Pre-processor marker macros

// Struct function decl/impl macros

#define prexy_decl(name, macro_name)                                           \
    macro_name##_decl(name);                                                   \
    PREXY_ASSERT_IS_NAMESPACE(macro_name)

// clang-format off
// NOTE: _impl macros do not end with semicolon

#define prexy_impl(name, macro_name)                                           \
    macro_name##_impl(name, name##_X)                                          \
    PREXY_ASSERT_IS_NAMESPACE(macro_name)

#define prexy_impl_attr(name, macro_name, attr)                                \
    macro_name##_impl(name, name##_X_##attr)                                   \
    PREXY_ASSERT_IS_STRUCT(attr)

#define prexy_impl_tag(name, macro_name, tag)                                  \
    macro_name##_impl(name, name##_X_##tag)                                    \
    PREXY_ASSERT_IS_NAMESPACE(macro_name)

// clang-format on

// Generic macros

#define prexy_typeof(struct_name, field_name)                                  \
    PREXY_CONCAT3(struct_name, _FIELDTYPE_, field_name)
#define prexy_is_mut_ptr(struct_name, field_name)                              \
    PREXY_CONCAT3(struct_name, _IS_MUT_PTR_, field_name)
#define prexy_is_const_ptr(struct_name, field_name)                            \
    PREXY_CONCAT3(struct_name, _IS_CONST_PTR_, field_name)
#define prexy_ptr_typeof(struct_name, field_name)                              \
    PREXY_CONCAT3(struct_name, _PTRTYPE_, field_name)

#define prexy_methodname(struct_name, fn_name)                                 \
    PREXY_CONCAT3(struct_name, _, fn_name)

#endif
