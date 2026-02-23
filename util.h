#ifndef UITL_H
#define UITL_H

#include <stdint.h>

typedef int8_t    s8;
typedef int16_t   s16;
typedef int32_t   s32;
typedef int64_t   s64;
typedef uint8_t   u8;
typedef uint16_t  u16;
typedef uint32_t  u32;
typedef uint64_t  u64;

#define bit(n) (1u << (n))

#define kb(n) (((u64)(n)) << 10) 
#define mb(n) (((u64)(n)) << 20) 
#define gb(n) (((u64)(n)) << 30) 
#define tb(n) (((u64)(n)) << 40) 

#define min(a, b) ((a) < (b) ? (a) : (b))
#define max(a, b) ((a) > (b) ? (a) : (b))

#define array_size(array) (u64)((sizeof((array))) / sizeof((array)[0]))

#define fail_msg(...)                       \
  do                                        \
  {                                         \
    fprintf(stderr, "[FATAL] " __VA_ARGS__);\
    fprintf(stderr, "\nAt %s:%d in %s()\n", \
        __FILE__, __LINE__, __func__);      \
  }                                         \
  while(0)

#define expect(expr)                              \
  do                                              \
  {                                               \
    if (!(expr))                                  \
    {                                             \
      fail_msg("Expression '%s' failed.", #expr); \
    }                                             \
  }                                               \
  while(0)

#define for_each(name, L) \
    for (typeof((L).ptr) (name) = (L).ptr, _end__ = (L).ptr + (L).size; (name) != _end__; ++(name))

#endif // UITL_H
