#include "krs_str.h"
#include "krs_dynamic_array.h"

#include <ctype.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct str str(char *const cstr)
{
    return (struct str){
        .ptr = cstr,
        .len = strlen(cstr),
    };
}

bool str_split_at_delims(
    struct str const s,
    char const *const delims,
    struct str *const head,
    struct str *const tail
)
{
    bool success = false;

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

            success = true;

            goto done;
        }
    }

    *head = s;
    *tail = (struct str){0};

done:
    return success;
}

bool str_split_delims(
    struct str const s,
    char const *const delims,
    struct str *const head,
    struct str *const tail
)
{
    bool const is_split = str_split_at_delims(s, delims, head, tail);

    if (is_split)
    {
        ++tail->ptr;
        --tail->len;
    }

    return is_split;
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

int str_fprint_repr(FILE *const stream, struct str const *const s)
{
    return fprintf(stream, "str(\"%.*s\")", str_format_args(*s));
}

//
// cstrbuf
//

void cstrbuf_debug_print(struct cstrbuf const b)
{
    printf("%lu %lu '%s'\n", b.len, b.cap, b.ptr);
}

void cstrbuf_deinit(struct cstrbuf *const b)
{
    if (b->ptr)
    {
        free(b->ptr);
    }
}

void str_revert_into_cstr_unsafe(struct str const s, char const removed_char)
{
    s.ptr[s.len] = removed_char;
}

struct str cstrbuf_to_str(struct cstrbuf const b)
{
    return (struct str){
        .ptr = b.ptr,
        .len = b.len,
    };
}

bool cstrbuf_extend_cstrn(
    struct cstrbuf *const b, char const *cstr, size_t const n
)
{
    char *extension;

    size_t old_len = b->len;
    bool const success = da_extend_uninit(b, n + 1, &extension);

    if (success)
    {
        size_t i = 0;
        for (; i < n && *cstr; ++i)
        {
            *extension++ = *cstr++;
        }
        *extension = '\0';

        b->len = old_len + i;
    }

    return success;
}

bool cstrbuf_extend_cstr(struct cstrbuf *const b, char const *const cstr)
{
    return cstrbuf_extend_cstrn(b, cstr, strlen(cstr));
}

bool cstrbuf_extend_str(struct cstrbuf *const b, struct str const s)
{
    return cstrbuf_extend_cstrn(b, s.ptr, s.len);
}
