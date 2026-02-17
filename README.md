# mem_arena / sized_string Test Program

This repository contains a small **C23 test program** used to validate a custom
**memory arena allocator (`mem_arena`)**, **sized string utilities (`sized_string`)**,
and minimal **OS abstraction helpers (`os`)**.

The project is intentionally low-level and allocation-explicit, following a
C-style design suitable for systems programming and engine-style codebases.

---

## Supported Platform

- **Linux only**
- Tested with **GCC (C23)**

The OS layer currently assumes a POSIX-compatible environment.

---

## Project Structure

```
.
├── main.c              # Test / entry point
├── mem_arena.c         # Arena allocator implementation
├── mem_arena.h         # Arena allocator interface
├── sized_string.c      # Sized string implementation
├── sized_string.h      # Sized string interface
├── os.c                # OS abstraction (memory, threads, etc.)
├── os.h                # OS interface
├── util.h              # Common utilities and helpers
└── README.md           # This file
```

---

## Example Code

The test program demonstrates scratch arenas and arena-backed string
concatenation without heap allocation.

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

## Key Components

### Memory Arenas (`mem_arena`)
- Linear allocator
- Explicit lifetime management
- No implicit heap allocations
- Scratch arenas for temporary allocation scopes

### Scratch Arenas
- Allocated once at startup
- Borrowed using `mem_arena_scratch_begin`
- Returned using `mem_arena_scratch_end`
- Optional conflict tracking to avoid arena aliasing

### Sized Strings (`sized_string`)
- `(pointer, length)` string representation
- No reliance on null-termination
- Arena-backed concatenation
- Zero heap usage

### OS Layer (`os`)
- Minimal platform abstraction
- Centralized system calls
- Designed to isolate platform-specific code

---

## Build Instructions

Example using GCC:

```bash
gcc -std=c23 -Wall -Wextra -O2 \
  main.c \
  mem_arena.c \
  sized_string.c \
  os.c \
  -o test
```

Adjust include paths if needed.

---

## Design Goals

- Explicit memory management
- Predictable allocation behavior
- No exceptions
- No hidden allocations
- Simple, inspectable C APIs
- Suitable for game engines and systems tools

---

## Notes

- This is **test and development code**, not a finalized library.
- Error handling is intentionally minimal.
- Thread-safety is not guaranteed unless explicitly documented.
- APIs may change freely.

---

## License

Unspecified. Use at your own risk.
