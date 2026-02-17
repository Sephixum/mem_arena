#include "mem_arena.h"
#include "sized_string.h"
#include <stdio.h>

void
test(void)
{
  mem_arena_temp_t temp_a = mem_arena_scratch_begin(.conflicts = nullptr, .n = 0);

  string8_t str1 = string8_lit("This is the first string");
  string8_t str2 = string8_lit("this is the second one.\n");

  string8_t res = push_string8_concat(
    temp_a.arena,
    push_string8_concat(temp_a.arena, str1, string8_lit(" and ")),
    str2
  );

  string8_print(stdout, res);

  mem_arena_scratch_end(temp_a);
}

int
main(void)
{
  mem_arena_alloc_scratches();

  test();

  mem_arena_release_scratches();
}
