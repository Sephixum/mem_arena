#ifndef MEM_ARENA_H
#define MEM_ARENA_H

#include "util.h"
#include <string.h>

#define MEM_ARENA_HEADER_SIZE    128
#define MEM_ARENA_USE_FREE_LIST  true
#define MEM_ARENA_USE_MALLO      false

enum 
{
  Mem_Arena_Flag_None,
  Mem_Arena_Flag_No_Chain
};
typedef u32 mem_arena_flags;

/***
 * The arena structure is for the purpose of storing data
 * about the state of a block of memory.
 * Also this is the storage of the linked list of arenas
 * for additional heap memory.
 */
typedef struct mem_arena mem_arena_t;

constexpr u64             mem_arena_default_reserve_size = mb(64);
constexpr u64             mem_arena_default_commit_size  = kb(64);
constexpr mem_arena_flags mem_arena_default_flags        = Mem_Arena_Flag_None;

typedef struct mem_arena_config 
{
  /***
   * Memory in bytes that should be reserved for the arena.
   */
  u64 reserve_size;

  /***
   * Memory in bytes that should be commited for the arena by default.
   */
  u64 commit_size;

  /***
   * Config flags.
   */
  mem_arena_flags flags;
}
mem_arena_config_t;

typedef struct mem_arena_temp
{
  mem_arena_t* arena;
  u64          position;
}
mem_arena_temp_t;

mem_arena_t*
mem_arena_alloc_from_config(const mem_arena_config_t* config);

mem_arena_t*
mem_arena_alloc(void);

void
mem_arena_release(mem_arena_t* arena);

void*
mem_arena_push(mem_arena_t* arena, u64 size, u64 align);

u64 
mem_arena_pos(mem_arena_t* arena);

void 
mem_arena_pop_to(mem_arena_t* arena, u64 pos);

void
mem_arena_clear(mem_arena_t* arena);

void
mem_arena_pop(mem_arena_t* arena, u64 size);

mem_arena_temp_t 
mem_arena_temp_begin(mem_arena_t* arena);

void
mem_arena_temp_end(mem_arena_temp_t temp);

#define mem_arena_push_array_aligned_no_zero(a, T, c, align) (T*)mem_arena_push((a), sizeof(T)*(c), (align))
#define mem_arena_push_array_aligned(a, T, c, align) \
  (T*)memset(mem_arena_push_array_aligned_no_zero(a, T, c, align), 0, sizeof(int) * (c));

#define mem_arena_push_array(a, T, c) \
        mem_arena_push_array_aligned((a), T, (c), max(8, alignof(T)))

#define mem_arena_push_array_no_zero(a, T, c)             \
        mem_arena_push_array_aligned_no_zero((a), T, (c), \
                                         max(8, alignof(T)))

#define mem_arena_push_struct_no_zero(a, T) \
        mem_arena_push_array_aligned_no_zero((a), T, 1, max(8, alignof(T)))

#define mem_arena_push_struct(a, T) \
        mem_arena_push_array_aligned((a), T, 1, max(8, alignof(T)))

void
mem_arena_alloc_scratches(void);

void
mem_arena_release_scratches(void);

typedef struct mem_arena_scratch_conflict_info 
{
  mem_arena_t*const *const conflicts;
  u64           n;
}
mem_arena_scratch_conflict_info_t;

mem_arena_temp_t
mem_arena_scratch_begin_(mem_arena_scratch_conflict_info_t info);

#define mem_arena_scratch_begin(...)                                    \
  mem_arena_scratch_begin_(                                             \
      (mem_arena_scratch_conflict_info_t){                              \
        .conflicts = nullptr,                                           \
        .n         = 0,                                                 \
        __VA_ARGS__                                                     \
      })

void
mem_arena_scratch_end(mem_arena_temp_t);

#endif // MEM_ARENA_H
