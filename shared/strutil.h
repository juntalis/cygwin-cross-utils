/**
 * @file strutil.h
 * @author Charles Grunwald <cgrunwald@gmail.com>
 * @brief TODO: Description for strutil.h
 */
#ifndef _STRUTIL_H_
#define _STRUTIL_H_
#pragma once

#include "shared.h"

#ifdef __cplusplus
extern "C" {
#endif

#if !defined(_WIN32)
inline int _vscprintf(const char* format, va_list pargs);
#endif

char* vsprintf_alloc(const char* format, va_list pargs);
char* sprintf_alloc(const char* format, ...);

ALWAYS_INLINE char* xstrrchr(char* subject, size_t subject_len, char needle); // NOLINT
ALWAYS_INLINE char* xstrrstr(char* subject, size_t subject_len, const char* needle, size_t needle_len); // NOLINT

#ifdef __cplusplus
};
#endif

#endif /* _STRUTIL_H_ */
