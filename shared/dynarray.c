/*
 * lwan - simple web server
 * Copyright (c) 2017 Leandro A. F. Pereira <leandro@hardinfo.org>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301,
 * USA.
 */

#define _GNU_SOURCE

#include <errno.h>
#include <stdint.h>
#include <stdlib.h>

#include "shared.h"
#include "dynarray.h"

#define INCREMENT 16

int dynarray_init(struct dynarray* a)
{
	if(UNLIKELY(!a))
		return -EINVAL;
	
	a->base = NULL;
	a->elements = 0;
	
	return 0;
}

int dynarray_reset(struct dynarray* a)
{
	if(UNLIKELY(!a))
		return -EINVAL;
	
	free(a->base);
	a->base = NULL;
	a->elements = 0;
	
	return 0;
}

#if !defined(HAVE_BUILTIN_ADD_OVERFLOW)

static inline bool add_overflow(size_t a, size_t b, size_t* out)
{
	if(UNLIKELY(a > 0 && b > SIZE_MAX - a))
		return true;
	
	*out = a + INCREMENT;
	return false;
}

#else
#define add_overflow __builtin_add_overflow
#endif

void* dynarray_append(struct dynarray* a, size_t element_size)
{
	if(!(a->elements % INCREMENT)) {
		void* new_base;
		size_t new_cap;
		
		if(UNLIKELY(add_overflow(a->elements, INCREMENT, &new_cap))) {
			errno = EOVERFLOW;
			return NULL;
		}
		
		new_base = reallocarray(a->base, new_cap, element_size);
		if(UNLIKELY(!new_base))
			return NULL;
		
		a->base = new_base;
	}
	
	return ((unsigned char*)a->base) + a->elements++ * element_size;
}

void dynarray_sort(struct dynarray* a,
                    size_t element_size,
                    int (* cmp)(const void* a, const void* b))
{
	if(LIKELY(a->elements))
		qsort(a->base, a->elements - 1, element_size, cmp);
}
