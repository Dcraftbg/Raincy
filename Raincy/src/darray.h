#pragma once
#include <stddef.h>
#define darray_of(T) struct {T* data; size_t len; size_t cap;}

#ifndef DA_INIT_CAP
#define DA_INIT_CAP 256
#endif
#ifndef DA_REALLOC
#   include <stdlib.h>
#   define DA_REALLOC(ptr, oldSize, newSize) realloc(ptr, newSize)
#endif
#ifndef DA_FREE
#   include <stdlib.h>
#   define DA_FREE(ptr, size) free(ptr)
#endif



#ifndef DA_ASSERT
#   include <assert.h>
#   define DA_ASSERT(x) assert(x)
#endif


#define darray_drop(da) \
	do { \
	if((da)->data) DA_FREE((da)->data, (da)->cap); \
	} while (0)
#define darray_push(da, v) \
	do { \
		if ((da)->len + 1 > (da)->cap) { \
			size_t old = (da)->cap; \
			void* oldp = (da)->data; \
			(da)->cap = (da)->cap ? (da)->cap * 2 : DA_INIT_CAP; \
			\
			(da)->data = DA_REALLOC((da)->data, old * sizeof(*(da)->data), (da)->cap * sizeof(*(da)->data)); \
			if ((da)->data == NULL) { \
				DA_FREE(oldp, old * sizeof(*(da)->data)); \
				DA_ASSERT(false && "Ran out of memory :("); \
				TraceLog(LOG_FATAL, "RAN OUT OF MEMORY"); \
			} \
		} \
		(da)->data[(da)->len] = v; \
		(da)->len++;\
	} while(0)

	