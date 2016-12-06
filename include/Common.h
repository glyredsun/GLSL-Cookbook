#ifndef _COMMON_H_
#define _COMMON_H_

#include <stdio.h>
#include <assert.h>

#define ASSERT_MSG(cond, format, ...)	do {	\
	if (!(cond)) {								\
		fprintf(stderr, format, __VA_ARGS__);	\
	}											\
	assert(cond);								\
} while(0)

#define RAISE_ERR(format, ...)	do {		\
	fprintf(stderr, format, __VA_ARGS__);	\
	assert(false);							\
} while (0)

#define OFFSET_OF(type, member)	((size_t)(&(((type*)0)->member)))

#endif // !_COMMON_H_
