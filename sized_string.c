#include "mem_arena.h"
#include "util.h"
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include "sized_string.h"

#if !defined(internal_function)
  #define internal_function static
#endif
#if !defined(local_persist)
  #define local_persist static
#endif
#if !defined(global_variable)
  #define global_variable static
#endif

constexpr global_variable const u8 integer_symbol_reverse[256] =
{
  ['0'] = 0,
  ['1'] = 1,
  ['2'] = 2,
  ['3'] = 3,
  ['4'] = 4,
  ['5'] = 5,
  ['6'] = 6,
  ['7'] = 7,
  ['8'] = 8,
  ['9'] = 9,

  ['A'] = 10, ['a'] = 10,
  ['B'] = 11, ['b'] = 11,
  ['C'] = 12, ['c'] = 12,
  ['D'] = 13, ['d'] = 13,
  ['E'] = 14, ['e'] = 14,
  ['F'] = 15, ['f'] = 15,
};

bool
char_is_space(u8 c)
{
  return (
       c == ' '
    || c == '\n'
    || c == '\t'
    || c == '\r'
    || c == '\f'
    || c == '\v'
  );
}

bool
char_is_upper(u8 c)
{
  return ('A' <= c && c <= 'Z');
}

bool
char_is_lower(u8 c)
{
  return ('a' <= c && c <= 'z');
}

bool
char_is_alpha(u8 c)
{
  return (char_is_lower(c) || char_is_upper(c));
}

bool
char_is_slash(u8 c)
{
  return (c == '/' || c == '\\');
}

bool
char_is_digit(u8 c, u32 base)
{
  bool result = false;
  if (base > 0 && base <= 16)
  {
    u8 val = integer_symbol_reverse[c];
    if (val < base)
    {
      result = true;
    }
  }
  return result;
}

u8
char_to_lower(u8 c)
{
  if (char_is_upper(c))
  {
    c += ('a' - 'A');
  }
  return c;
}

u8
char_to_upper(u8 c)
{
  if (char_is_lower(c))
  {
    c += ('A' - 'a');
  }
  return c;
}

u8
char_to_correct_slash(u8 c)
{
  if (char_is_slash(c))
  {
    c = '/';
  }
  return c;
}

u64 
cstring8_length(u8 *c)
{
  expect(c != nullptr);

  u64 length = 0;
  u8* p = c;
  for (;*p != 0; p += 1);
  length = (u64)(p - c);

  return length;
}


string8_t
string8_cstring(char* c)
{
  expect(c != nullptr);
  string8_t result = {.ptr = (u8*)c, .size = cstring8_length((u8*)c)};
  return result;
}

string8_t
string8(u8 *str, u64 size)
{
  expect(str != nullptr);
  return (string8_t){
    .ptr = str,
    .size = size
  };
}

string8_t
string8_subrange_view(u8* first, u8* one_past_last)
{
  expect(first != nullptr && one_past_last != nullptr);
  return (string8_t){
    .ptr  = first,
    .size = (u64)(one_past_last - first)
  };
}

string8_t
string8_zero(void)
{
  return (string8_t){0};
}

bool
string8_match(string8_t a, string8_t b, string_match_flag_t flags)
{
  bool result = false;
  if (a.size == b.size && flags == 0)
  {
    result = memcmp(a.ptr, b.ptr, b.size);
  }
  else if (a.size == b.size || (flags & StringMatchFlag_RightSideSloppy))
  {
    bool case_insensitive  = (flags & StringMatchFlag_CaseInsensitive);
    bool slash_insensitive = (flags & StringMatchFlag_SlashInsensitive);
    u64  size              = min(a.size, b.size);
    result                 = true;
    for (u64 idx = 0; idx < size; ++idx)
    {
      u8 at = a.ptr[idx];
      u8 bt = b.ptr[idx];
      if (case_insensitive)
      {
        at = char_to_upper(at);
        bt = char_to_upper(bt);
      }
      if (slash_insensitive)
      {
        at = char_to_correct_slash(at);
        bt = char_to_correct_slash(bt);
      }
      if (at != bt)
      {
        result = false;
        break;
      }
    }
  }
  return result;
}

string8_t
upper_from_str8(mem_arena_t* arena, string8_t string)
{
  expect(arena != nullptr);
  string = push_string8_copy(arena, string);
  for(u64 idx = 0; idx < string.size; idx += 1)
  {
    string.ptr[idx] = char_to_upper(string.ptr[idx]);
  }
  return string;
}

string8_t 
push_string8_concat(mem_arena_t* arena, string8_t s1, string8_t s2)
{
  expect(arena != nullptr);
  string8_t str = {0};
  str.size = s1.size + s2.size;
  str.ptr  = mem_arena_push_array_no_zero(arena, u8, str.size + 1);
  memcpy(str.ptr, s1.ptr, s1.size);
  memcpy(str.ptr + s1.size, s2.ptr, s2.size);
  str.ptr[str.size] = '\0';
  return str;
}

string8_t 
push_string8_copy(mem_arena_t* arena, string8_t s)
{
  expect(arena != nullptr);
  string8_t str = {0};
  str.size = s.size;
  str.ptr  = mem_arena_push_array_no_zero(arena, u8, str.size + 1);
  memcpy(str.ptr, s.ptr, s.size);
  str.ptr[str.size] = '\0';
  return str;
}

string8_t 
push_string8fv(mem_arena_t* arena, char *fmt, va_list args)
{
  expect(arena != nullptr);
  va_list args2;
  va_copy(args2, args);
  u32       needed_bytes  = vsnprintf(0, 0, fmt, args) + 1;
  string8_t result        = {0};
  result.ptr              = mem_arena_push_array_no_zero(arena, u8, needed_bytes);
  result.size             = vsnprintf((char*)result.ptr, needed_bytes, fmt, args2);
  result.ptr[result.size] = '\0';
  va_end(args2);
  return result;

}

string8_t 
push_string8f(mem_arena_t* arena, char *fmt, ...)
{
  expect(arena != nullptr);
  va_list args;
  va_start(args, fmt);
  string8_t result = push_string8fv(arena, fmt, args);
  va_end(args);
  return result;
}
