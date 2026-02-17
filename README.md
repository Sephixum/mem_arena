# mem_arena / sized_string Test Program

This repository contains a small **C23 test program** used to validate a custom
**memory arena allocator (`mem_arena`)** and a **fixed-size string utility (`sized_string`)**.

The code demonstrates:
- Scratch arena allocation
- Temporary arena lifetime management
- String concatenation without heap allocation
- Simple output using standard I/O

---

## Supported Platform

- **Linux only**
- Tested with **GCC (C23)**

No attempt has been made to support non-POSIX platforms at this stage.

---

## Files Overview

```
.
├── mem_arena.h        # Arena allocator API
├── sized_string.h    # Fixed-size string utilities
├── test.c            # Test program
└── README.md         # This file
```

---

## Example Code

The test program exercises scratch arenas and string concatenation:

```c
#include "mem_arena.h"
#include "sized_string.h"
#include <stdio.h>

void
test(void)
{
  mem_arena_temp_t temp_a =
    mem_arena_scratch_begin(.conflicts = nullptr, .n = 0);

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
```

---

## Key Concepts

### Memory Arenas
- No `malloc`/`free` during normal operation
- Linear allocation
- Explicit lifetime control
- Scratch arenas provide temporary allocation scopes

### Scratch Arenas
- Allocated once at startup
- Borrowed temporarily via `mem_arena_scratch_begin`
- Automatically reclaimed with `mem_arena_scratch_end`
- Conflict tracking supported (but optional)

### Sized Strings
- Strings are `(pointer, length)` pairs
- No implicit null-termination requirements
- Concatenation allocates directly into an arena
- No heap usage

---

## Build Instructions

Example using GCC:

```bash
gcc -std=c23 -Wall -Wextra -O2 test.c -o test
```

Adjust include paths as needed if the headers are in separate directories.

---

## Design Goals

- Explicit memory management
- Predictable allocation behavior
- No exceptions
- No hidden allocations
- C-style APIs with minimal abstraction overhead

This code is intended for:
- Low-level systems programming
- Game engines
- Tools where allocation control matters

---

## Notes

- This is **test code**, not a finalized API.
- Error handling is minimal by design.
- Thread-safety is not guaranteed unless explicitly stated in the allocator.

---

## License

Unspecified. Use at your own risk.
