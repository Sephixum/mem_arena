#include "os.h"
#include <string.h>

#if PLATFORM_LINUX
  #include <sys/mman.h>
  #include <unistd.h>
  #include <dlfcn.h>
  #include <sys/sysinfo.h>
  #include <dirent.h>
  #include <sys/stat.h>
  #include <fcntl.h>
  #include <errno.h>
#else
  #error "Platform not supported"
#endif

#include <stdio.h>
#include <stdlib.h>

#define align_pow2(x, b) (((x) + (b) - 1) & (~((b) - 1)))
#define clamp_top(a, max) min(a, max)
#define clamp_bottom(a, min) max(a, min)

#if !defined(internal_function)
  #define internal_function static
#endif
#if !defined(local_persist)
  #define local_persist static
#endif
#if !defined(global_variable)
  #define global_variable static
#endif

os_system_info_t* 
os_get_system_info(void)
{
  local_persist os_system_info_t info      = {0};
  local_persist bool             is_cached = false;

  if (!is_cached)
  {
#if PLATFORM_LINUX
    info.logical_processor_count = (u32)get_nprocs();
    info.page_size               = (u64)getpagesize();
    info.allocation_granularity  = info.page_size;
#else
  #error "Platform not supported"
#endif
    is_cached = true;
  }

  return &info;
}


void*
os_memory_reserve(u64 size)
{
  if (size == 0)
  {
    return nullptr;
  }

  #if PLATFORM_LINUX
  void* p_res = mmap(nullptr, size, PROT_NONE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
  if (p_res == MAP_FAILED)
  {
    p_res = nullptr;
  }
  return p_res;
  #else
    #error "Unsupported platform"
  #endif
}


void
os_memory_release(void* addr, u64 size) 
{
#if PLATFORM_LINUX
  munmap(addr, size);
#else
    #error "Unsupported platform"
#endif
}

void
os_memory_commit(void* ptr, u64 size)
{
#if PLATFORM_LINUX
  s32 res = mprotect(ptr, size, PROT_READ | PROT_WRITE);
  if (res < 0)
  {
    fail_msg("mprotect failed to commit memory: %s", strerror(errno));
  }
#else
    #error "Unsupported platform"
#endif
}

void
os_memory_decommit(void* ptr, u64 size)
{
#if PLATFORM_LINUX
  madvise(ptr, size, MADV_DONTNEED);
  mprotect(ptr, size, PROT_NONE);
#else
    #error "Unsupported platform"
#endif
}
