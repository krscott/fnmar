#ifndef STR_H_
#define STR_H_

#include <stdbool.h>
#include <stddef.h>
#include <string.h>

struct str
{
    char *ptr;
    size_t len;
};

#define str_format_args(s) ((int)(s).len), ((s).ptr)

void str_split_at_delims(
    struct str s, char const *delims, struct str *head, struct str *tail
);
struct str str_trim_left_char(struct str s, char c);
struct str str_trim_left_whitespace(struct str s);
struct str str_trim_right_whitespace(struct str s);
struct str str_trim_whitespace(struct str s);

// UNSAFE: Writes to `s.ptr[s.len]`
char const *str_into_cstr_unsafe(struct str s, char *removed_char);

// UNSAFE: Writes to `s.ptr[s.len]`
void str_revert_into_cstr_unsafe(struct str s, char removed_char);

struct cstrbuf
{
    char *ptr;
    size_t len;
    size_t cap;
};

void cstrbuf_deinit(struct cstrbuf *b);
struct str cstrbuf_to_str(struct cstrbuf b);
bool cstrbuf_extend_cstrn(struct cstrbuf *b, char const *cstr, size_t n);
bool cstrbuf_extend_cstr(struct cstrbuf *b, char const *cstr);

#endif
