#ifndef AUGUSTUS_COMMON_H
#define AUGUSTUS_COMMON_H

#include <stdbool.h>
#include <string.h>
#include <limits.h>
#include <stdint.h>

typedef uint8_t  u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef int8_t  i8;
typedef int16_t i16;
typedef int32_t i32;
typedef int64_t i64;

typedef char            byte;
typedef unsigned char   ubyte;

typedef float f32;
typedef long double f64;

typedef size_t usize;

#define U8_MAX  UCHAR_MAX
#define U16_MAX USHRT_MAX
#define U32_MAX ULONG_MAX
#define U64_MAX ULLONG_MAX

#define I8_MAX  CHAR_MAX
#define I16_MAX SHRT_MAX
#define I32_MAX LONG_MAX
#define I64_MAX LLONG_MAX

#endif//AUGUSTUS_COMMON_H
