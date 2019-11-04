/**
 * @file strbuf.h
 * @brief TODO
 */
#ifndef _STRBUF_H_
#define _STRBUF_H_
#pragma once

#include <stdlib.h>

#ifdef __cplusplus
extern "C" {
#endif

/* string buffer */
struct strbuf
{
	size_t maxlen;
	size_t len;
	char ptr[];
};

typedef struct strbuf strbuf_t;

strbuf_t* strbuf_alloc(size_t maxlen);
void strbuf_free(strbuf_t* buf);
strbuf_t* strbuf_new_with_len(const char* str, size_t len);
strbuf_t* strbuf_new(const char* str);
strbuf_t* strbuf_append_with_len(strbuf_t* buf, const char* str, size_t len);
strbuf_t* strbuf_append(strbuf_t* buf, const char* str);

#ifdef __cplusplus
};
#endif

#include "shared.h"

#endif //_STRBUF_H_
