#include "krs_dynamic_array.h"
#include <stdint.h>
#include <string.h>

#ifndef GROW_NUMERATOR
#ifdef GROW_DENOMINATOR
#error "Define both GROW_NUMERATOR and GROW_DENOMINATOR"
#endif
#define GROW_NUMERATOR 3
#define GROW_DENOMINATOR 2
#endif

#define MIN_CAP 8
#define GROW_CAP(cap) ((cap) / GROW_DENOMINATOR * GROW_NUMERATOR)
#define CAP_LIMIT (SIZE_MAX / GROW_NUMERATOR * GROW_DENOMINATOR)

void *da_at_unchecked_(
    void *const *restrict const ptr, size_t const elem_size, size_t const i
)
{
    return (void *)((uintptr_t)(*ptr) + (i * elem_size));
}

bool da_at_(
    void *const *restrict const ptr,
    size_t *restrict const len,
    size_t const elem_size,
    size_t const i,
    void **const out
)
{
    assert(*len == 0 || ptr);

    bool const success = i < *len;

    if (success)
    {
        *out = da_at_unchecked_(ptr, elem_size, i);
    }

    return success;
}

bool da_reserve_(
    void **restrict const ptr,
    size_t *restrict const len,
    size_t *restrict const cap,
    size_t const elem_size,
    size_t const n
)
{
    assert(*len == 0 && *cap == 0 || ptr);

    bool success;

    size_t const target_len = *len + n;

    if (target_len <= *cap)
    {
        success = true;
    }
    else if (*cap >= CAP_LIMIT)
    {
        success = false;
    }
    else
    {
        size_t new_cap = *cap < MIN_CAP ? MIN_CAP : GROW_CAP(*cap);
        while (new_cap < target_len)
        {
            new_cap = GROW_CAP(new_cap);
        }

        void *const new_ptr = realloc(*ptr, new_cap * elem_size);

        if (new_ptr)
        {
            success = true;
            *ptr = new_ptr;
            *cap = new_cap;
        }
        else
        {
            success = false;
        }
    }

    return success;
}

bool da_extend_uninit_(
    void **restrict const ptr,
    size_t *restrict const len,
    size_t *restrict const cap,
    size_t const elem_size,
    size_t const n,
    void **restrict const out_ptr
)
{
    assert(*len == 0 && *cap == 0 || ptr);

    bool const success = da_reserve_(ptr, len, cap, elem_size, n);

    if (success)
    {
        *out_ptr = (void *)((uintptr_t)(*ptr) + elem_size * *len);
        // memset(*out_ptr, 0, elem_size * n);
        *len += n;
    }

    return success;
}

bool da_extend_(
    void **const ptr,
    size_t *restrict const len,
    size_t *restrict const cap,
    size_t const elem_size,
    void const *const data,
    size_t const n
)
{
    assert(*len == 0 && *cap == 0 || ptr);

    void *extension;
    bool const success =
        da_extend_uninit_(ptr, len, cap, elem_size, n, &extension);

    if (success)
    {
        memmove(extension, data, elem_size * n);
    }

    return success;
}
