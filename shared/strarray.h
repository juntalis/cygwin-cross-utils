/**
 * @file strarray.h
 * @author Charles Grunwald <cgrunwald@gmail.com>
 * @brief TODO: Description for strarray.h
 */
#ifndef STRARRAY_H
#define STRARRAY_H

#include "shared.h"

#define OFFSET_OF(type, member) ((size_t)(&((type*)0)->member))

#if defined(__cplusplus)
extern "C" {
#endif

/* string array */
typedef struct string_array
{
	size_t maxlen;
	size_t len;
	char* ptr[];
} string_array;

static ALWAYS_INLINE string_array* string_array_alloc(size_t maxlen)
{
	string_array* array = malloc(OFFSET_OF(string_array, ptr) + maxlen * sizeof(char*));
	array->maxlen = 0;
	array->len = 0;
	return array;
}

static ALWAYS_INLINE string_array* string_array_push(string_array* array, char* str)
{
	if (!array) {
		array = string_array_alloc(1);
		if (!array)
			return array;
	}
	if (array->len + 1 > array->maxlen) {
		size_t newmaxlen = (array->len + 1) > (array->maxlen * 2) ? (array->len + 1) : (array->maxlen * 2);
		array = realloc(array, OFFSET_OF(string_array, ptr) + newmaxlen * sizeof(char*));
		if (!array)
			return array;
		array->maxlen = newmaxlen;
	}
	array->ptr[array->len++] = str;
	return array;
}

static ALWAYS_INLINE void string_array_free(string_array* array)
{
	size_t i;
	char** p;
	if (!array)
		return;
	for (i = 0, p = array->ptr; i < array->len; ++i, ++p) {
		if (*p)
			free(*p);
	}
	free(array);
}

#ifdef __cplusplus
};
#endif

#endif /* STRARRAY_H */
