/**
 * @file strbuf.c
 * @author Charles Grunwald <cgrunwald@gmail.com>
 * @brief TODO: Description for strbuf.c
 */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stddef.h>

#include "shared.h"
#include "strbuf.h"

strbuf_t* strbuf_alloc(size_t maxlen)
{
	strbuf_t* buf = (strbuf_t*)malloc(offsetof(struct strbuf, ptr) + maxlen + 1);
	if(!buf)
		return buf;
	buf->maxlen = maxlen;
	buf->ptr[buf->len = 0] = '\0';
	return buf;
}

void strbuf_free(strbuf_t* buf)
{
	if(buf)
		free(buf);
}

strbuf_t* strbuf_new_with_len(const char* str, size_t len)
{
	strbuf_t* buf = NULL;
	size_t bufsize = offsetof(struct strbuf, ptr) + len + 1;
	buf = (strbuf_t*)malloc(bufsize);
	if(UNLIKELY(!buf)) {
		return buf;
	}
	buf->maxlen = len;
	memcpy(buf->ptr, str, len);
	buf->ptr[buf->len = len] = '\0';
	return buf;
}

strbuf_t* strbuf_new(const char* str)
{
	size_t len = str ? strlen(str) : 0;
	return strbuf_new_with_len(str, len);
}

strbuf_t* strbuf_append_with_len(strbuf_t* buf, const char* str, size_t len)
{
	if(!str || !len)
		return buf;
	if(!buf)
		return strbuf_new_with_len(str, len);
	if(buf->len + len > buf->maxlen) {
		size_t newmaxlen = (buf->len + len) > (buf->maxlen * 2) ? (buf->len + len) : (buf->maxlen * 2);
		buf = realloc(buf, offsetof(struct strbuf, ptr) + newmaxlen + 1);
		if(!buf)
			return buf;
		buf->maxlen = newmaxlen;
	}
	memcpy(buf->ptr + buf->len, str, len);
	buf->ptr[buf->len += len] = '\0';
	return buf;
}

strbuf_t* strbuf_append(strbuf_t* buf, const char* str)
{
	size_t len = str ? strlen(str) : 0;
	return len ? strbuf_append_with_len(buf, str, len) : buf;
}

