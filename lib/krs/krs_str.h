#ifndef KRS_STR_H_
#define KRS_STR_H_

#include "krs_cc_ext.h"
#include <assert.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

// ASCII string (non-null terminated)
struct str
{
    char *ptr;
    size_t len;
};
// ASCII string view (non-null terminated)
struct sv
{
    char const *ptr;
    size_t len;
};
// Invariant: str and sv are bit-compatible
static_assert(offsetof(struct str, ptr) == offsetof(struct sv, ptr), "");
static_assert(offsetof(struct str, len) == offsetof(struct sv, len), "");
static_assert(sizeof(struct str) == sizeof(struct sv), "");

nodiscard static inline struct str str_empty(void)
{
    return (struct str){
        .ptr = "",
        .len = 0,
    };
}
nodiscard static inline struct sv sv_empty(void)
{
    return (struct sv){
        .ptr = "",
        .len = 0,
    };
}

// TODO: rename to sv_*
#define str_format_args(s) ((int)(s).len), ((s).ptr)

nodiscard struct sv sv_from_cstr(char const *cstr);
static inline struct sv sv_from_str(struct str const str)
{
    return (struct sv){
        .ptr = str.ptr,
        .len = str.len,
    };
}

nodiscard bool sv_equal_cstr(struct sv s, char const *cstr);

bool sv_split_at_delims(
    struct sv s, char const *delims, struct sv *head, struct sv *tail
);

bool sv_split_delims(
    struct sv s, char const *delims, struct sv *head, struct sv *tail
);
nodiscard struct sv sv_trim_left_char(struct sv s, char c);
nodiscard struct sv sv_trim_left_whitespace(struct sv s);
nodiscard struct sv sv_trim_right_whitespace(struct sv s);
nodiscard struct sv sv_trim_whitespace(struct sv s);
int sv_fprint_repr(FILE *stream, struct sv const *s);

nodiscard struct str str_from_cstr(char *cstr);

// UNSAFE: Drops const from underlying string
nodiscard static inline struct str str_from_sv_unsafe(struct sv const sv)
{
    return (struct str){
        .ptr = (char *)(uintptr_t)sv.ptr,
        .len = sv.len,
    };
}

static inline bool str_split_at_delims(
    struct str s, char const *delims, struct str *head, struct str *tail
)
{
    struct sv h = sv_empty();
    struct sv t = sv_empty();
    bool split = sv_split_at_delims(sv_from_str(s), delims, &h, &t);
    if (head)
    {
        *head = str_from_sv_unsafe(h);
    }
    if (tail)
    {
        *tail = str_from_sv_unsafe(t);
    }
    return split;
}
static inline bool str_split_delims(
    struct str s, char const *delims, struct str *head, struct str *tail
)
{
    struct sv h = sv_empty();
    struct sv t = sv_empty();
    bool split = sv_split_delims(sv_from_str(s), delims, &h, &t);
    if (head)
    {
        *head = str_from_sv_unsafe(h);
    }
    if (tail)
    {
        *tail = str_from_sv_unsafe(t);
    }
    return split;
}
static inline nodiscard struct str str_trim_left_char(struct str s, char c)
{
    return str_from_sv_unsafe(sv_trim_left_char(sv_from_str(s), c));
}
static inline nodiscard struct str str_trim_left_whitespace(struct str s)
{
    return str_from_sv_unsafe(sv_trim_left_whitespace(sv_from_str(s)));
}
static inline nodiscard struct str str_trim_right_whitespace(struct str s)
{
    return str_from_sv_unsafe(sv_trim_right_whitespace(sv_from_str(s)));
}
static inline nodiscard struct str str_trim_whitespace(struct str s)
{
    return str_from_sv_unsafe(sv_trim_whitespace(sv_from_str(s)));
}
int str_fprint_repr(FILE *stream, struct str const *s);

// UNSAFE: Writes to `s.ptr[s.len]`
char const *str_into_cstr_unsafe(struct str s, char *removed_char);

// UNSAFE: Writes to `s.ptr[s.len]`
void str_revert_into_cstr_unsafe(struct str s, char removed_char);

// Null-terminated ASCII string dynamic buffer
struct cstrbuf
{
    char *ptr;
    size_t len;
    size_t cap;
};

void cstrbuf_deinit(struct cstrbuf *b);
nodiscard struct str cstrbuf_to_str(struct cstrbuf b);
nodiscard bool cstrbuf_extend_cstrn( //
    struct cstrbuf *b,
    char const *cstr,
    size_t n
);
nodiscard bool cstrbuf_extend_cstr(struct cstrbuf *b, char const *cstr);
nodiscard bool cstrbuf_extend_sv(struct cstrbuf *b, struct sv s);
static inline nodiscard bool cstrbuf_extend_str(struct cstrbuf *b, struct str s)
{
    return cstrbuf_extend_sv(b, sv_from_str(s));
}
nodiscard bool cstrbuf_reserve(struct cstrbuf *b, size_t n);

void cstrbuf_debug_print(struct cstrbuf const b);

#define cstrbuf_snprintf(out_success, buf, n, ...)                             \
    do                                                                         \
    {                                                                          \
        bool *const success_ = (out_success);                                  \
        struct cstrbuf *const b_ = (buf);                                      \
        size_t n_ = n;                                                         \
                                                                               \
        *success_ = cstrbuf_reserve(b_, n_);                                   \
        if (*success_)                                                         \
        {                                                                      \
            n_ = (size_t)snprintf(&b_->ptr[b_->len], n_, __VA_ARGS__);         \
            b_->len += n_;                                                     \
            assert(b_->ptr[b_->len] == '\0');                                  \
        }                                                                      \
    } while (0)

#endif
