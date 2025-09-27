#ifndef PREXY_H_
#define PREXY_H_

#include <assert.h>

// Pre-processor marker macros

#define prexy
#define prexy_tag(name)                                                        \
    struct name                                                                \
    {                                                                          \
        char prexy_tag__;                                                      \
    }

#define px_tag(name)                                                           \
    static_assert(sizeof((struct name){.prexy_tag__ = '\0'}), "")
#define px_attr(name, ...) static_assert(sizeof((struct name){__VA_ARGS__}), "")

// Helper macros

#define prexy_count(name) (name##_COUNT + (0 * sizeof((enum name)0)))

#define PREXY_ASSERT_IS_TAG(name) px_tag(name)

// Struct function decl/impl macros

#define prexy_decl(name, macro_name)                                           \
    macro_name##_decl(name);                                                   \
    PREXY_ASSERT_IS_TAG(macro_name)

// clang-format off
// NOTE: _impl macros do not end with semicolon

#define prexy_impl(name, macro_name)                                           \
    macro_name##_impl(name, name##_X)                                          \
    PREXY_ASSERT_IS_TAG(macro_name)

#define prexy_impl_attr(name, macro_name, attr)                                \
    macro_name##_impl(name, name##_X_##attr)                                   \
    static_assert(sizeof(*(struct attr *)0), "")

#define prexy_impl_tag(name, macro_name, tag)                                  \
    macro_name##_impl(name, name##_X_##tag)                                    \
    PREXY_ASSERT_IS_TAG(tag)

// clang-format on

#endif
