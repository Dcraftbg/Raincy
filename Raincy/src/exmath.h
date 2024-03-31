#pragma once
#include <stdint.h>

#ifndef DEFINE_EXMATH_T_EXTRA
#define DEFINE_EXMATH_T_EXTRA
#endif
// TODO: Support definitions like DEFINE_EXMATH_T_SUPPORT_ABS, ...


#ifndef EXMATH_API
#define EXMATH_API
#endif
#define _min(a,b) a < b ? a : b
#define _max(a,b) a > b ? a : b
#define DEFINE_EXMATH_FLOAT_BASE \
	X(f, float) \
	X(f64, double)
#define DEFINE_EXMATH_INT_BASE \
	X(i64, int64_t ) \
	X(i32, int32_t ) \
	X(i16, int16_t ) \
	X(i8 , int8_t  ) 
#define DEFINE_EXMATH_UINT_BASE \
	X(u64, uint64_t) \
	X(u32, uint32_t) \
	X(u16, uint16_t) \
	X(u8 , uint8_t ) 
//#define X(abr, typ)
#define DEFINE_EXMATH_T \
	DEFINE_EXMATH_FLOAT_BASE \
	DEFINE_EXMATH_INT_BASE \
	DEFINE_EXMATH_UINT_BASE \
	DEFINE_EXMATH_T_EXTRA


#define X(abr, typ) EXMATH_API typ min##abr(typ a, typ b);
DEFINE_EXMATH_T
#define X(abr, typ) EXMATH_API typ max##abr(typ a, typ b);
DEFINE_EXMATH_T
#define X(abr, typ) EXMATH_API typ abs##abr(typ a);
DEFINE_EXMATH_INT_BASE
DEFINE_EXMATH_FLOAT_BASE

#undef X


#ifdef EXMATH_IMPLEMENTATION

#define X(abr, typ) EXMATH_API typ min##abr(typ a, typ b) {return _min(a,b);}
DEFINE_EXMATH_T

#define X(abr, typ) EXMATH_API typ max##abr(typ a, typ b) {return _max(a,b);}
DEFINE_EXMATH_T

#define X(abr, typ) EXMATH_API typ abs##abr(typ a) {return a >= 0 ? a : -a;}
DEFINE_EXMATH_INT_BASE
DEFINE_EXMATH_FLOAT_BASE

#endif


//inline float minf(float a, float b) { return _min(a, b); }