#include "mem_arena.h"
#include "os.h"
#include "util.h"

#include <stdio.h>

#if !defined(internal_function)
  #define internal_function static
#endif
#if !defined(local_persist)
  #define local_persist static
#endif
#if !defined(global_variable)
  #define global_variable static
#endif


#define align_pow2(x, b) (((x) + (b) - 1) & (~((b) - 1)))
#define clamp_top(a, max) min(a, max)
#define clamp_bottom(a, min) max(a, min)

/***
 * Primary arena structure.
 *
 * Responsibilities:
 *   - Owns an OS-backed virtual memory region
 *   - Maintains allocation state (bump allocator)
 *   - Optionally supports block chaining and reuse
 */
struct mem_arena 
{
  /***
   * Pointers to head and the previous of linked list.
   */
  mem_arena_t* current;
  mem_arena_t* prev;

#if MEM_ARENA_USE_FREE_LIST
  /***
   * Last block that is deallocated and is free to use again.
   */
  mem_arena_t* free_last;

  /***
   * Size of the reserved memory in the deallocated blocks that can be
   * used again.
   */
  u64 free_size;
#endif

  mem_arena_flags flags;

  /***
   * Reserved memory of the whole arena (not mine we'll see).
   */
  u64 reserved_size;

  /***
   * ...
   */
  u64 committed_size;

  /***
   * Starting byte offset of this block within the arena.
   */
  u64 base_position;

  /***
   * Position up to where of the commited memory has been allocated.
   */
  u64 position;

  /***
   * Position up to where memory has been commited.
   */
  u64 commit;

  /***
   * Position up to where memory has been reserved.
   */
  u64 reserved;
};
static_assert(
    sizeof(mem_arena_t) <= MEM_ARENA_HEADER_SIZE,
    "Expected arena header to be smaller than 128 bytes"
);

mem_arena_t*
mem_arena_alloc_from_config(const mem_arena_config_t* config)
{
  expect(config != nullptr);

  u64 reserve_size = config->reserve_size + MEM_ARENA_HEADER_SIZE;
  u64 commit_size  = config->commit_size;

  

  reserve_size = align_pow2(reserve_size, os_get_system_info()->page_size);
  commit_size  = align_pow2(commit_size , os_get_system_info()->page_size);

  void* base = os_memory_reserve(reserve_size);
  if (nullptr == base)
  {
    fail_msg("Failed to reserve memory for arena");
  }

  os_memory_commit(base, commit_size);

  mem_arena_t* arena = base;
  arena->committed_size = config->commit_size;
  arena->current       = arena;
  arena->reserved_size = config->reserve_size;
  arena->flags         = config->flags;
  arena->base_position = 0;
  arena->position      = MEM_ARENA_HEADER_SIZE;
  arena->commit        = commit_size;
  arena->reserved      = reserve_size;
  arena->free_size     = 0;
  arena->free_last     = nullptr;

  // asan_poison_memory_region(base, commit_size);
  // asan_unpoison_memory_region(base, ARENA_HEADER_SIZE);

  return arena;
}


mem_arena_t*
mem_arena_alloc(void)
{
  return mem_arena_alloc_from_config(&(mem_arena_config_t)
      {
        .reserve_size = mem_arena_default_reserve_size,
        .commit_size  = mem_arena_default_commit_size,
        .flags        = mem_arena_default_flags
      }
  );
}

void
mem_arena_release(mem_arena_t* arena)
{
  for (mem_arena_t *cur  = arena->current,  *prev = nullptr;
       cur != nullptr;
       cur = prev)
  {
    prev = cur->prev;
    os_memory_release(cur, cur->reserved);
  }
}

void*
mem_arena_push(mem_arena_t* arena, u64 size, u64 align)
{
  expect(arena != nullptr);

  if(size == 0)
  {
    return nullptr;
  }

  mem_arena_t* current  = arena->current;
  u64          pos_prev = align_pow2(current->position, align);
  u64          pos_post = pos_prev + size;

  if (current->reserved < pos_post && !(arena->flags & Mem_Arena_Flag_No_Chain))
  {
    mem_arena_t* new_block = nullptr;

#if MEM_ARENA_USE_FREE_LIST
    {
      mem_arena_t* prev_block = nullptr; 
      for (new_block = arena->free_last, prev_block = nullptr;
           new_block != nullptr;
           prev_block = new_block, new_block = new_block->prev)
      {
        if(new_block->reserved >= (align_pow2(new_block->position, align) + size))
        {
          if (prev_block != nullptr)
          {
            prev_block->prev = new_block->prev;
          }
          else
          {
            arena->free_last = new_block->prev;
          }
          break;
        }
      }
    }
#endif // MEM_ARENA_USE_FREE_LIST

    if(new_block == nullptr)
    {
      u64 reserve_size  = current->reserved_size;
      u64 commit_size   = current->committed_size;

      if(size + MEM_ARENA_HEADER_SIZE > reserve_size)
      {
        reserve_size  = align_pow2(size + MEM_ARENA_HEADER_SIZE, align);
        commit_size   = align_pow2(size + MEM_ARENA_HEADER_SIZE, align);
      }

      new_block = mem_arena_alloc_from_config(&(mem_arena_config_t){
          .reserve_size = reserve_size,
          .commit_size  = commit_size,
          .flags        = current->flags
        });
    }

    new_block->base_position = current->base_position + current->reserved;
    new_block->prev          = arena->current;
    arena->current           = new_block;
    current                  = new_block;
    pos_prev                 = align_pow2(current->position, align);
    pos_post                 = pos_prev + size;
    expect(pos_post <= current->reserved);
  }
  
  if (current->commit < pos_post)
  {
    u64 commit_post_aligned = pos_post + current->committed_size - 1;
    commit_post_aligned    -= commit_post_aligned % current->committed_size;

    u64 commit_post_clamped = clamp_top(commit_post_aligned, current->reserved);
    u64 commit_size         = commit_post_clamped - current->commit;
    u64 commit_ptr          = (u64)current + current->commit;

    os_memory_commit((void*)commit_ptr, commit_size);
    current->commit = commit_post_clamped;
  }

  void* result = nullptr;
  if(current->commit >= pos_post)
  {
    result            = (void*)((u64)current + pos_prev);
    current->position = pos_post;
  }

  if(result == nullptr)
  {
    fail_msg("Arena allocation failed");
  }

  return result;
}

u64 
mem_arena_pos(mem_arena_t* arena)
{
  expect(arena != nullptr);

  mem_arena_t* current = arena->current;
  u64          pos     = current->base_position + current->position;

  return pos;
}

void 
mem_arena_pop_to(mem_arena_t* arena, u64 pos)
{
  expect(arena != nullptr);

  u64          big_posiiton = clamp_bottom(MEM_ARENA_HEADER_SIZE, pos);
  mem_arena_t* current      = arena->current;

#if MEM_ARENA_USE_FREE_LIST
  for (mem_arena_t* prev = nullptr; current->base_position >= big_posiiton; current = prev)
  {
    prev              = current->prev;
    current->position = MEM_ARENA_HEADER_SIZE;
    arena->free_size  = current->reserved_size;
    current->prev     = arena->free_last;
    arena->free_last  = current;
  }
#else
  for (mem_arena_t* prev = nullptr; current->base_position >= big_posiiton; current = prev)
  {
    prev = current->prev;
    os_memory_release(current, current->reserved);
  }
#endif

  arena->current   = current;
  u64 new_position = big_posiiton - current->base_position;

  expect(new_position <= current->position);

  current->position = new_position;
}

void
mem_arena_clear(mem_arena_t* arena)
{
  expect(arena != nullptr);
  mem_arena_pop_to(arena, 0);
}

void
mem_arena_pop(mem_arena_t* arena, u64 size)
{
  expect(arena != nullptr);

  u64 position_old = mem_arena_pos(arena);
  u64 position_new = position_old;

  if (size < position_old)
  {
    position_new = position_old - size;
  }

  mem_arena_pop_to(arena, position_new);
}

mem_arena_temp_t 
mem_arena_temp_begin(mem_arena_t* arena)
{
  expect(arena != nullptr);
  u64 position = mem_arena_pos(arena);
  return (mem_arena_temp_t){
          .arena     = arena,
          .position = position
  };
}

void
mem_arena_temp_end(mem_arena_temp_t temp)
{
  mem_arena_pop_to(temp.arena, temp.position);
}

global_variable thread_local mem_arena_t* scratch_arenas[2] = {0};

internal_function void
set_scratch_arena_at(u32 idx, mem_arena_t* arena)
{
  expect(idx >= 0 && idx < array_size(scratch_arenas));
  scratch_arenas[idx] = arena;
}

internal_function mem_arena_t*
get_scratch_arena_at(u32 idx)
{
  expect(idx >= 0 && idx < array_size(scratch_arenas));
  return scratch_arenas[idx];
}

void
mem_arena_alloc_scratches(void)
{
  for (u64 idx = 0; idx < array_size(scratch_arenas); idx++)
  {
    if (nullptr == get_scratch_arena_at(idx))
    {
      set_scratch_arena_at(idx, mem_arena_alloc());
    }
  }
}

void
mem_arena_release_scratches(void)
{
  for (u64 idx = 0; idx < array_size(scratch_arenas); idx++)
  {
    if (nullptr != get_scratch_arena_at(idx))
    {
      mem_arena_release(get_scratch_arena_at(idx));
      set_scratch_arena_at(idx, nullptr);
    }
  }
}

mem_arena_temp_t
mem_arena_scratch_begin_(mem_arena_scratch_conflict_info_t info)
{
  mem_arena_temp_t scratch = {0};

  for (u64 i = 0; i < array_size(scratch_arenas); ++i)
  {
    bool is_conflicting = false;

    for (u64 j = 0; j < info.n; ++j)
    {
      mem_arena_t* conflict = info.conflicts[j];
      if(get_scratch_arena_at(i) == conflict)
      {
        is_conflicting = true;
        break;
      }
    }

    if (is_conflicting == false)
    {
      scratch.arena    = get_scratch_arena_at(i);
      scratch.position = scratch.arena->position;
      break;
    }
  }

  return scratch;
}

void
mem_arena_scratch_end(mem_arena_temp_t temp) 
{
  mem_arena_temp_end(temp);
}
