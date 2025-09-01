#include "krs_str.h"
#include "krs_dynamic_array.h"

#include <ctype.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct str str_from_cstr(char *const cstr)
{
    return (struct str){
        .ptr = cstr,
        .len = strlen(cstr),
    };
}
struct sv sv_from_cstr(char const *const cstr)
{
    return (struct sv){
        .ptr = cstr,
        .len = strlen(cstr),
    };
}

bool sv_split_at_delims(
    struct sv const s,
    char const *const delims,
    struct sv *const head,
    struct sv *const tail
)
{
    bool success = false;

    for (size_t i = 0; i < s.len; ++i)
    {
        if (strchr(delims, s.ptr[i]))
        {
            *head = (struct sv){
                .ptr = s.ptr,
                .len = i,
            };
            *tail = (struct sv){
                .ptr = &s.ptr[i],
                .len = s.len - i,
            };

            success = true;

            goto done;
        }
    }

    *head = s;
    *tail = (struct sv){0};

done:
    return success;
}

bool sv_split_delims(
    struct sv const s,
    char const *const delims,
    struct sv *const head,
    struct sv *const tail
)
{
    bool const is_split = sv_split_at_delims(s, delims, head, tail);

    if (is_split)
    {
        ++tail->ptr;
        --tail->len;
    }

    return is_split;
}

struct sv sv_trim_left_char(struct sv s, char const c)
{
    while (s.len > 0 && *s.ptr == c)
    {
        ++s.ptr;
        --s.len;
    }

    return s;
}

struct sv sv_trim_left_whitespace(struct sv s)
{
    while (s.len > 0 && isspace(*s.ptr))
    {
        ++s.ptr;
        --s.len;
    }

    return s;
}

struct sv sv_trim_right_whitespace(struct sv s)
{
    while (s.len > 0 && isspace(s.ptr[s.len - 1]))
    {
        --s.len;
    }

    return s;
}

struct sv sv_trim_whitespace(struct sv s)
{
    return sv_trim_right_whitespace(sv_trim_left_whitespace(s));
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

int sv_fprint_repr(FILE *const stream, struct sv const *const s)
{
    return fprintf(stream, "str(\"%.*s\")", str_format_args(*s));
}

//
// cstrbuf
//

void cstrbuf_debug_print(struct cstrbuf const b)
{
    printf("%lu %lu '%s'\n", (unsigned long)b.len, (unsigned long)b.cap, b.ptr);
}

void cstrbuf_deinit(struct cstrbuf *const b)
{
    if (b->ptr)
    {
        free(b->ptr);
    }
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

    size_t const old_len = b->len;
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

    assert(b->ptr[b->len] == '\0');

    return success;
}

bool cstrbuf_extend_cstr(struct cstrbuf *const b, char const *const cstr)
{
    return cstrbuf_extend_cstrn(b, cstr, strlen(cstr));
}

bool cstrbuf_extend_sv(struct cstrbuf *const b, struct sv const s)
{
    return cstrbuf_extend_cstrn(b, s.ptr, s.len);
}
