#ifndef STR_H_
#define STR_H_

#include <stddef.h>

struct str
{
    char *ptr;
    size_t len;
};

#define str_debug_print(s) printf("%.*s", (int)(s).len, (s).ptr)
#define str_debug_println(s) printf("%.*s\n", (int)(s).len, (s).ptr)

void str_split_at_delims(
    struct str const s,
    char const *const delims,
    struct str *const head,
    struct str *const tail
);
struct str str_trim_left_char(struct str s, char const c);
struct str str_trim_left_whitespace(struct str s);
struct str str_trim_right_whitespace(struct str s);
struct str str_trim_whitespace(struct str s);

// UNSAFE: Writes to `s.ptr[s.len]`
char const *str_into_cstr_unsafe(struct str const s, char *const removed_char);

// UNSAFE: Writes to `s.ptr[s.len]`
void str_revert_into_cstr_unsafe(struct str const s, char const removed_char);

struct cstrbuf
{
    char *ptr;
    size_t len;
    size_t cap;
};

void cstrbuf_deinit(struct cstrbuf *const cstrbuf);

struct str cstrbuf_to_str(struct cstrbuf const cstrbuf);

enum error cstrbuf_init_from_file( //
    struct cstrbuf *const cstrbuf,
    char const *const filepath
);

struct str cstrbuf_to_str(struct cstrbuf const cstrbuf);

#endif
