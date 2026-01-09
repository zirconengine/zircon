#pragma once

typedef unsigned char      u8;
typedef unsigned short     u16;
typedef unsigned int       u32;
typedef unsigned long long u64;
typedef unsigned long int  ul32;

typedef signed char      i8;
typedef signed short     i16;
typedef signed int       i32;
typedef signed long long i64;

typedef float  f32;
typedef double f64;

typedef void*       VoidPtr;
typedef const void* ConstPtr;
typedef char*       CharPtr;
typedef const char* ConstChr;

#if defined(_WIN64)
#define ZI_WIN 1
#define ZI_API __declspec(dllexport)
#elif defined(__EMSCRIPTEN__)
#define ZI_EMSCRIPTEN 1
#define ZI_API
#elif defined(__linux__)
#define ZI_LINUX 1
#define ZI_API
#elif defined(__APPLE__)
#define ZI_MACOS 1
#define ZI_API
#else
#define ZI_API
#endif

typedef f32 Float;

#define ZI_HANDLER(StructName)                                                 \
  typedef struct StructName {                                                  \
    VoidPtr handler;                                                               \
  } StructName

#if defined _MSC_VER
// unsigned int MAX
#define U8_MAX 0xffui8
#define U16_MAX 0xffffui16
#define U32_MAX 0xffffffffui32
#define U64_MAX 0xffffffffffffffffui64

// signed int MIN
#define I8_MIN (-127i8 - 1)
#define I16_MIN (-32767i16 - 1)
#define I32_MIN (-2147483647i32 - 1)
#define I64_MIN (-9223372036854775807i64 - 1)

// signed int MAX
#define I8_MAX 127i8
#define I16_MAX 32767i16
#define I32_MAX 2147483647i32
#define I64_MAX 9223372036854775807i64

#define F32_MAX 3.402823466e+38F
#define F64_MAX 1.7976931348623158e+308

#define F32_MIN 1.175494351e-38F
#define F64_MIN 2.2250738585072014e-308

#define F32_LOW (-(F32_MAX))
#define F64_LOW (-(F64_MAX))

#elif defined __GNUC__
#define I8_MIN (-128)
#define I16_MIN (-32767 - 1)
#define I32_MIN (-2147483647 - 1)
#define I64_MIN INT64_MIN

#define I8_MAX (127)
#define I16_MAX (32767)
#define I32_MAX (2147483647)
#define I64_MAX INT64_MAX

/* Maximum of unsigned integral types.  */
#define U8_MAX (255)
#define U16_MAX (65535)
#define U32_MAX (4294967295U)
#define U64_MAX 18446744073709551615UL

#define F32_MAX __FLT_MAX__
#define F64_MAX __DBL_MAX__

#define F32_MIN __FLT_MIN__
#define F64_MIN __DBL_MIN__

#define F32_LOW (-(F32_MAX))
#define F64_LOW (-(F64_MAX))
#else
static_assert(false, "not implemented");
#endif
