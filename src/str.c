
#include "str.h"

#include <ctype.h>
#include <stdlib.h>
#include <string.h>

void str_split_at_delims(
    struct str const s,
    char const *const delims,
    struct str *const head,
    struct str *const tail
)
{
    for (size_t i = 0; i < s.len; ++i)
    {
        if (strchr(delims, s.ptr[i]))
        {
            *head = (struct str){
                .ptr = s.ptr,
                .len = i,
            };
            *tail = (struct str){
                .ptr = &s.ptr[i],
                .len = s.len - i,
            };

            goto done;
        }
    }

    *head = s;
    *tail = (struct str){0};

done:
    return;
}

struct str str_trim_left_char(struct str s, char const c)
{
    while (s.len > 0 && *s.ptr == c)
    {
        ++s.ptr;
        --s.len;
    }

    return s;
}

struct str str_trim_left_whitespace(struct str s)
{
    while (s.len > 0 && isspace(*s.ptr))
    {
        ++s.ptr;
        --s.len;
    }

    return s;
}

struct str str_trim_right_whitespace(struct str s)
{
    while (s.len > 0 && isspace(s.ptr[s.len - 1]))
    {
        --s.len;
    }

    return s;
}

struct str str_trim_whitespace(struct str s)
{
    return str_trim_right_whitespace(str_trim_left_whitespace(s));
}

char const *str_into_cstr_unsafe(struct str const s, char *const removed_char)
{
    if (removed_char)
    {
        *removed_char = s.ptr[s.len];
    }
    s.ptr[s.len] = '\0';

    return s.ptr;
}

void str_revert_into_cstr_unsafe(struct str const s, char const removed_char)
{
    s.ptr[s.len] = removed_char;
}

struct str cstrbuf_to_str(struct cstrbuf const cstrbuf)
{
    return (struct str){
        .ptr = cstrbuf.ptr,
        .len = cstrbuf.len,
    };
}

void cstrbuf_deinit(struct cstrbuf *const cstrbuf)
{
    if (cstrbuf->ptr)
    {
        free(cstrbuf->ptr);
    }
}
