#ifndef KRS_SPAN_H_
#define KRS_SPAN_H_

#include "krs_cc_ext.h"
#include <assert.h>
#include <stdbool.h>
#include <stddef.h>

nodiscard bool span_remove_left_n_( //
    void **ptr,
    size_t *len,
    size_t elem_size,
    size_t n
);
#define span_remove_left(span, data)                                           \
    span_remove_left_n_(                                                       \
        (void **)&(span)->ptr,                                                 \
        &(span)->len,                                                          \
        sizeof(*(span)->ptr),                                                  \
        1                                                                      \
    )
#define span_remove_left_n(span, data, n)                                      \
    span_remove_left_n_(                                                       \
        (void **)&(span)->ptr,                                                 \
        &(span)->len,                                                          \
        sizeof(*(span)->ptr),                                                  \
        n                                                                      \
    )

#endif
