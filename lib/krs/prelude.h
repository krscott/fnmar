#ifndef PRELUDE_H_
#define PRELUDE_H_

#include <assert.h>
#include <stdint.h>

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef int8_t i8;
typedef int16_t i16;
typedef int32_t i32;
typedef int64_t i64;

typedef float f32;
typedef double f64;

#define UNINIT_BYTE (0xCB)
#define uninitialize(ptr) memset((ptr), UNINIT_BYTE, sizeof(*(ptr)));

#define ARRAY_LENGTH(arr) (sizeof(arr) / sizeof((arr)[0]))

// Get printf format string for a type.
// Does *not* assume a `char *` is a `%s`.
#define BASIC_TYPE_FORMAT(x)                                                   \
    (_Generic(                                                                 \
        (x),                                                                   \
         short: "%hd",                                                         \
         unsigned short: "%hu",                                                \
         int: "%d",                                                            \
         unsigned int: "%u",                                                   \
         long: "%ld",                                                          \
         unsigned long: "%lu",                                                 \
         long long: "%lld",                                                    \
         unsigned long long: "%llu",                                           \
         float: "%f",                                                          \
         double: "%lf",                                                        \
         char: "'%c'",                                                         \
         void *: "%p"                                                          \
    ))

#endif
