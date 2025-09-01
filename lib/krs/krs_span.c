#include "krs_span.h"
#include <stdint.h>

nodiscard bool span_remove_left_n_(
    void **const ptr, size_t *const len, size_t const elem_size, size_t const n
)
{
    bool can_remove = (*len >= n);

    if (can_remove && n > 0)
    {
        assert(*ptr);
        *ptr = (void *)((uintptr_t)(*ptr) + (n * elem_size));
    }

    return can_remove;
}
