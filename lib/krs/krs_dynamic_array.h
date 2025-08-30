#ifndef KRS_DYNAMIC_ARRAY_H_
#define KRS_DYNAMIC_ARRAY_H_

#include <assert.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

void *da_at_unchecked_(void *const *restrict ptr, size_t elem_size, size_t i);
#define da_at_unchecked(arr)                                                   \
    da_at_unchecked_((void *const *)&(arr)->ptr, sizeof(*(arr)->ptr), i)

bool da_at_(
    void *const *restrict ptr,
    size_t *restrict len,
    size_t elem_size,
    size_t i,
    void **out
);
#define da_at(arr, out)                                                        \
    da_at_((void *const *)&(arr)->ptr, &(arr)->len, sizeof(*(arr)->ptr), i, out)

bool da_reserve_(
    void **restrict const ptr,
    size_t *restrict const len,
    size_t *restrict const cap,
    size_t const elem_size,
    size_t const n
);
#define da_reserve(arr, n)                                                     \
    da_reserve_(                                                               \
        (void **)&(arr)->ptr,                                                  \
        &(arr)->len,                                                           \
        &(arr)->cap,                                                           \
        sizeof(*(arr)->ptr),                                                   \
        n                                                                      \
    )

bool da_extend_uninit_(
    void **restrict const ptr,
    size_t *restrict const len,
    size_t *restrict const cap,
    size_t const elem_size,
    size_t const n,
    void **restrict const out_ptr
);
#define da_extend_uninit(arr, n, out_ptr)                                      \
    da_extend_uninit_(                                                         \
        (void **)&(arr)->ptr,                                                  \
        &(arr)->len,                                                           \
        &(arr)->cap,                                                           \
        sizeof(*(arr)->ptr),                                                   \
        n,                                                                     \
        (void **)out_ptr                                                       \
    )

bool da_extend_(
    void **ptr,
    size_t *restrict len,
    size_t *restrict cap,
    size_t elem_size,
    void const *data,
    size_t n
);
#define da_extend(arr, data, n)                                                \
    da_extend_(                                                                \
        (void **)&(arr)->ptr,                                                  \
        &(arr)->len,                                                           \
        &(arr)->cap,                                                           \
        sizeof(*(arr)->ptr),                                                   \
        data,                                                                  \
        n                                                                      \
    )

#endif
