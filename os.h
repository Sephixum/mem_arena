#ifndef OS_H
#define OS_H

#include "util.h"

#if defined(__STDC_VERSION__) && __STDC_VERSION__ < 202311L
  #include <stdalign.h>
  #include <threads.h> // for thread_local keyword
  #include <stdbool.h>
  #define nullptr NULL
  #define static_assert _Static_assert;
#endif

#if !defined (PLATFORM_WINDOWS)
  #if defined (_WIN32)
    #define PLATFORM_WINDOWS true
  #else 
    #define PLATFORM_WINDOWS false
  #endif
#endif // !defined (PLATFORM_WINDOWS)

#if !defined (PLATFORM_LINUX)
  #if defined (__linux__)
    #define PLATFORM_LINUX true
  #else 
    #define PLATFORM_LINUX false
  #endif
#endif // !defined (PLATFORM_WINDOWS)

#if !defined(COMPILER_MSVC)
  #if defined(_MSC_VER)
    #define COMPILER_MSVC true
  #else
    #define COMPILER_MSVC false
  #endif
#endif

#if !defined(COMPILER_CLANG)
  #if defined(__clang__)
    #define COMPILER_CLANG true
  #else
    #define COMPILER_CLANG false
  #endif
#endif

#if !defined(COMPILER_GCC)
  #if defined(__GNUC__)
    #define COMPILER_GCC true
  #else
    #define COMPILER_GCC false
  #endif
#endif

typedef struct os_system_info
{
  u32 logical_processor_count; 
  u64 page_size;
  u64 allocation_granularity;
}
os_system_info_t;

os_system_info_t*
os_get_system_info(void);

void*
os_memory_reserve(u64 size);


void
os_memory_release(void* addr, u64 size);

void
os_memory_commit(void* ptr, u64 size);

void
os_memory_decommit(void* ptr, u64 size);

#endif // OS_H
