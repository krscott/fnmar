// Repeatable Trait - no header guards

#ifndef PREXY_EXPAND
#include "krs_cc_ext.h"
#include <assert.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>
#endif

#include "prexy.h"

#ifndef Vec
// For local LSP support
struct develop_vec
{
    int *ptr;
    size_t len;
    size_t cap;
};
#define Vec develop_vec
#define develop_vec_PTRTYPE_ptr int
#define VEC_IMPLEMENTATION
// #define VEC_OPT_INFALLIBLE
#endif

#ifndef vec_realloc
#ifndef PREXY_EXPAND
#include <stdlib.h>
#endif
#define vec_realloc(vec, p, size) realloc(p, size)
#define vec_free(vec, p) free(p)
#endif

#ifdef VEC_OPT_INFALLIBLE
#define alloc_fn void
#else
#define alloc_fn nodiscard bool
#endif

void prexy_methodname(Vec, deinit)(struct Vec *vec);
alloc_fn prexy_methodname(Vec, reserve)( //
    struct Vec *const vec,
    size_t const n
);
alloc_fn prexy_methodname(Vec, append)(
    struct Vec *const vec, prexy_ptr_typeof(Vec, ptr) const *arr, size_t const n
);
alloc_fn prexy_methodname(Vec, push)(
    struct Vec *vec, prexy_ptr_typeof(Vec, ptr) elem
);
nodiscard bool prexy_methodname(Vec, pop)(
    struct Vec *vec, prexy_ptr_typeof(Vec, ptr) * out
);

#ifdef VEC_IMPLEMENTATION

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

void prexy_methodname(Vec, deinit)(struct Vec *const vec)
{
    vec_free(vec, vec->ptr);
}

alloc_fn prexy_methodname(Vec, reserve)( //
    struct Vec *const vec,
    size_t const n
)
{
    assert(vec->ptr || (vec->len == 0 && vec->cap == 0));

    bool success;

    size_t const target_len = vec->len + n;

    if (target_len <= vec->cap)
    {
        success = true;
    }
    else if (vec->cap >= CAP_LIMIT)
    {
        success = false;
    }
    else
    {
        size_t new_cap = vec->cap < MIN_CAP ? MIN_CAP : GROW_CAP(vec->cap);
        while (new_cap < target_len)
        {
            new_cap = GROW_CAP(new_cap);
        }

        void *const new_ptr =
            vec_realloc(vec, vec->ptr, new_cap * sizeof(vec->ptr[0]));

        if (new_ptr)
        {
            success = true;
            vec->ptr = new_ptr;
            vec->cap = new_cap;
        }
        else
        {
            success = false;
        }
    }

#ifdef VEC_OPT_INFALLIBLE
    if (!success)
    {
        // TODO: panic
        assert(success && "vec reserve failed");
        exit(1);
    }
#else
    return success;
#endif
}

alloc_fn prexy_methodname(Vec, append)(
    struct Vec *const vec, prexy_ptr_typeof(Vec, ptr) const *arr, size_t const n
)
{
#ifdef VEC_OPT_INFALLIBLE

    prexy_methodname(Vec, reserve)(vec, n);
    memmove(&vec->ptr[vec->len], arr, n);
    vec->len += n;

#else

    bool const success = prexy_methodname(Vec, reserve)(vec, n);

    if (success)
    {
        memmove(&vec->ptr[vec->len], arr, n);
        vec->len += n;
    }

    return success;

#endif
}

alloc_fn prexy_methodname(Vec, push)(
    struct Vec *const vec, prexy_ptr_typeof(Vec, ptr) const elem
)
{
#ifdef VEC_OPT_INFALLIBLE
    prexy_methodname(Vec, append)(vec, &elem, 1);
#else
    return prexy_methodname(Vec, append)(vec, &elem, 1);
#endif
}

nodiscard bool prexy_methodname(Vec, pop)(
    struct Vec *const vec, prexy_ptr_typeof(Vec, ptr) *const out
)
{
    bool const success = vec->len > 0;

    if (success)
    {
        *out = vec->ptr[--vec->len];
    }

    return success;
}

#endif

#undef vec_free
#undef vec_realloc
#undef Vec
