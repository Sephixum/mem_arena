#include "mem_arena.h"
#include "sized_string.h"
#include <stdio.h>

void
test(void)
{
  mem_arena_temp_t temp_a = mem_arena_scratch_begin();

  string8_t str1 = string8_lit("This is \\ the first string");
  string8_t str2 = string8_lit("this is / the second one.\n");

  string8_t res = push_string8_concat(
    temp_a.arena,
    push_string8_concat(
      temp_a.arena,
      str1,
      string8_lit(" and ")
    ),
    str2
  );

  string8_print(stdout, res);

  string8_t to_cap = upper_from_string8(
      temp_a.arena,
      push_string8_copy(temp_a.arena, res)
  );

  string8_print(stdout, to_cap);

  bool pred = string8_match(
      res,
      to_cap,
      StringMatchFlag_CaseInsensitive | StringMatchFlag_SlashInsensitive | StringMatchFlag_RightSideSloppy
  );
  string8_print(
      stdout,
      pred ? string8_lit("true\n") : string8_lit("false\n")
  );

  pred = string8_match(
      to_cap,
      res,
      StringMatchFlag_CaseInsensitive | StringMatchFlag_SlashInsensitive
  );
  string8_print(
      stdout,
      pred ? string8_lit("true\n") : string8_lit("false\n")
  );

  pred = string8_match(
      to_cap,
      res,
      StringMatchFlag_CaseInsensitive
  );
  string8_print(
      stdout,
      pred ? string8_lit("true\n") : string8_lit("false\n")
  );

  mem_arena_scratch_end(temp_a);
}

void
match_test()
{
  string8_t a = string8_lit("this Is \\ a string");
  string8_t b = string8_lit("thiS is / A string");

  printf("[INFO] size of a : %lu\n", a.size);
  printf("[INFO] size of b : %lu\n", b.size);

  bool pred = string8_match(
    a,
    b,
    StringMatchFlag_CaseInsensitive | StringMatchFlag_SlashInsensitive | StringMatchFlag_RightSideSloppy
  );
  string8_print(
      stdout,
      pred ? string8_lit("true\n") : string8_lit("false\n")
  );
}

int
main(void)
{
  mem_arena_alloc_scratches();

  // test();
  match_test();

  mem_arena_release_scratches();
}
