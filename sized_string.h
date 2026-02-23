#ifndef SIZED_STRING_H
#define SIZED_STRING_H

#include "os.h"
#include "mem_arena.h"
#include <stdarg.h>

typedef struct string8 
{
  u8* ptr;
  u64 size;
}
string8_t;

typedef struct string8_node string8_node_t;
struct string8_node
{
   string8_node_t* next;
   string8_t       string;

};

typedef struct string8_list
{
  string8_node_t* first;
  string8_node_t* last;

  u64 node_count;
  u64 total_size;
}
string8_list_t;

typedef struct string8_array 
{
  string8_t* ptr; 
  u64        size;
}
string8_array_t;

enum 
{
  StringMatchFlag_CaseInsensitive   = bit(0),
  StringMatchFlag_SlashInsensitive  = bit(1),
  StringMatchFlag_RightSideSloppy   = bit(2)
};
typedef u32 string_match_flag_t;

enum
{
  StringPathStyle_Relative,
  StringPathStyle_WindowsAbsolute,
  StringPathStyle_UnixAbsolute,
#if PLATFORM_LINUX
  StringPathStyle_SystemAbsolute = StringPathStyle_UnixAbsolute,
#elif PLATFORM_WINDOWS
  StringPathStyle_SystemAbsolute = StringPathStyle_WindowsAbsolute,
#else
  #error "Unsupported platform"
#endif
};
typedef u32 string_path_style_t;

typedef struct string_join 
{
  string8_t pre;
  string8_t sep;
  string8_t post;
}
string_join_t;

bool
char_is_space(u8 c);

bool
char_is_upper(u8 c);

bool
char_is_lower(u8 c);

bool
char_is_alpha(u8 c);

bool
char_is_slash(u8 c);

bool
char_is_digit(u8 c, u32 base);

u8
char_to_lower(u8 c);

u8
char_to_upper(u8 c);

u8
char_to_correct_slash(u8 c);

u64 
cstring8_length(u8 *c);

string8_t
string8(u8 *str, u64 size);

#define string8_lit(s) string8((u8*)(s), sizeof((s)) - 1)
#define string8_lit_comp(s) {((u8*)(s), sizeof((s)) - 1)}

string8_t
string8_subrange_view(u8* first, u8* one_past_last);

string8_t
string8_zero(void);

string8_t 
upper_from_string8(mem_arena_t* arena, string8_t string);

string8_t 
lower_from_string8(mem_arena_t* arena, string8_t string);

string8_t 
backslashed_from_string8(mem_arena_t* arena, string8_t string);

string8_t
string8_cstring(char* c);

bool
string8_match(string8_t a, string8_t b, string_match_flag_t flags);

string8_t 
push_string8_concat(mem_arena_t* arena, string8_t s1, string8_t s2);

string8_t 
push_string8_copy(mem_arena_t* arena, string8_t s);

string8_t 
push_string8fv(mem_arena_t* arena, char *fmt, va_list args);

string8_t 
push_string8f(mem_arena_t* arena, char *fmt, ...);

#define string8_print(fd, S) \
  fwrite((S).ptr, sizeof *(S).ptr, (S).size, (fd))

#endif // SIZED_STRING_H
