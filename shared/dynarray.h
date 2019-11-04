/**
 * @file dyn-array.h
 * 
 * TODO: Description
 */
#ifndef _ARRAY_H_
#define _ARRAY_H_
#pragma once

#include <stdlib.h>
#include <stdint.h>
#include <memory.h>

#ifdef __cplusplus
extern "C" {
#endif

struct dynarray
{
	void *base;
	size_t elements;
};

int dynarray_init(struct dynarray *a);
int dynarray_reset(struct dynarray *a);
void *dynarray_append(struct dynarray *a, size_t element_size);
void dynarray_sort(struct dynarray *a, size_t element_size, int (*cmp)(const void *a, const void *b));

#define ARRAY_FOREACH(array_, iter_)                                      \
	for (iter_ = (array_)->base.base;                                          \
		 iter_ <                                                               \
		 ((typeof(iter_))(array_)->base.base + (array_)->base.elements);       \
		 iter_++)

#define ARRAY_FOREACH_REVERSE(array_, iter_)                              \
	for (iter_ = ((typeof(iter_))(array_)->base.base +                         \
				  (array_)->base.elements - 1);                                \
		 iter_ >= (typeof(iter_))(array_)->base.base; iter_--)

#define DEFINE_ARRAY_TYPE(array_type_, element_type_)                          \
	struct array_type_ {                                                       \
		struct dynarray base;                                                 \
	};                                                                         \
	__attribute__((unused)) static inline int array_type_##_init(              \
		struct array_type_ *array)                                             \
	{                                                                          \
		return dynarray_init((struct dynarray *)array);                      \
	}                                                                          \
	__attribute__((unused)) static inline int array_type_##_reset(             \
		struct array_type_ *array)                                             \
	{                                                                          \
		return dynarray_reset((struct dynarray *)array);                     \
	}                                                                          \
	__attribute__((unused)) static inline element_type_ *array_type_##_append( \
		struct array_type_ *array)                                             \
	{                                                                          \
		return (element_type_ *)dynarray_append((struct dynarray *)array,    \
												  sizeof(element_type_));      \
	}                                                                          \
	__attribute__((unused)) static inline element_type_                        \
		*array_type_##_append0(struct array_type_ *array)                      \
	{                                                                          \
		element_type_ *element = array_type_##_append(array);                  \
																			   \
		if (element)                                                           \
			memset(element, 0, sizeof(*element));                              \
																			   \
		return element;                                                        \
	}                                                                          \
	__attribute__((unused)) static inline void array_type_##_sort(             \
		struct array_type_ *array, int (*cmp)(const void *a, const void *b))   \
	{                                                                          \
		dynarray_sort((struct dynarray *)array, sizeof(element_type_),       \
						cmp);                                                  \
	}


#ifdef __cplusplus
};
#endif

#endif /* _ARRAY_H_ */
