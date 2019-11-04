/**
 * @file cygwin-runner-util.h
 * @brief TODO: Description
 */
#ifndef _SHARED_H_
#define _SHARED_H_
#pragma once

#include <stdlib.h>
#include <stddef.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <assert.h>
#include <limits.h>
#include <ctype.h>
#include <errno.h>
#include <stdarg.h>

#include <fcntl.h>
#include <process.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

// Compiler identification
#if defined(__GNUC__) || defined(__MINGW32__) || defined(__MINGW64__) || defined(__clang__)
#	define COMPILER_GNUCLIKE
#elif defined(_MSC_VER)
#	define COMPILER_MSVC
#else
#	error Unknown compiler!
#endif

/**
 * @def PLATFORM_WIN32
 * Defined when targeting Windows
 */
#if defined(_WIN32) || defined(COMPILER_MSVC) || defined(__MINGW32__) || defined(__MINGW64__)
#	define PLATFORM_WIN32
#endif

/**
 * @def GCC_ATTR(...)
 * Shortcut for GCC __attribute__ annotations.
 */
#ifndef CC_ATTR
#	ifdef COMPILER_GNUCLIKE
#		define CC_ATTR(...) __attribute__(( __VA_ARGS__ ))
#	else
#		define CC_ATTR(...) __declspec( __VA_ARGS__ ) 
#	endif
#endif

// Force inline
#ifndef ALWAYS_INLINE
#	if defined(DISABLE_INLINE_FUNCTIONS)
#		define ALWAYS_INLINE
#	elif defined(__always_inline)
#		define ALWAYS_INLINE __always_inline
#	elif defined()
#		define ALWAYS_INLINE inline GCC_ATTR(__always_inline__)
#	endif
#endif

// Used internally for the the two branch-prediction macros
#ifdef DISABLE_BRANCH_PREDICTION
#	define _LIKELY_IS(x,y) (x)
#else
#	define _LIKELY_IS(x,y) (__builtin_expect((x), (y)))
#endif

#ifndef LIKELY
#	define LIKELY(x) _LIKELY_IS(!!(x), 1)
#endif

#ifndef UNLIKELY
#	define UNLIKELY(x) _LIKELY_IS((x), 0)
#endif

/**
 * @def CPP_IGNORE(...)
 * Helper macro that just ignores its parameters.
 */
#define CPP_IGNORE(...)

/**
 * @def CPP_SINGLE_ARG(...)
 * Helper macro that provides a way to pass argument with commas in it to
 * some other macro whose syntax doesn't allow using extra parentheses.
 * Example:
 *
 *   #define MACRO(type, name) type name
 *   MACRO(CYG_SINGLE_ARG(std::pair<size_t, size_t>), x);
 *
 */
#define CPP_SINGLE_ARG(...) __VA_ARGS__

/**
 * @def CPP_PASTE(A,B)
 * Token pasting macro
 */
#ifdef __CONCAT
#	define CPP_PASTE(A,B) __CONCAT(A,B)
#else
#	define CPP_PASTE(A,B) _CPP_PASTE(A,B)
#	define _CPP_PASTE(A,B) A ## B
#endif

/**
 * @def CC_NORETURN
 * Qualifier for telling the compiler to assume that this function never
 * returns. (program execution ends)
 */
#ifdef _Noreturn
#	define CC_NORETURN void _Noreturn
#elif defined(COMPILER_MSVC)
#	define CC_NORETURN void CC_ATTR(noreturn)
#else
#	define CC_NORETURN void CC_ATTR(__noreturn__)
#endif

/**
 * @def CC_NOINLINE
 * Qualifier for telling the compiler to never inline this function.
 */
#ifdef __attribute_noinline__
#	define CC_NOINLINE static __attribute_noinline__
#elif defined(COMPILER_MSVC)
#	define CC_NOINLINE static CC_ATTR(noinline)
#else
#	define CC_NOINLINE static CC_ATTR(__noinline__)
#endif

/**
 * @def CPP_UNUSED(PARAM)
 * Annotate a parameter as being unused.
 */
#if defined(COMPILER_GNUCLIKE)
#	define CPP_UNUSED(PARAM) PARAM CC_ATTR(__unused__)
#else
#	define CPP_UNUSED(PARAM) PARAM
#endif

/**
 * @def CC_PACKED
 * When used on a struct declaration, disabling padding.
 */
#if defined(COMPILER_MSVC)
#	define PACKED_STRUCT(NAME) __pragma(pack(push, 1)) struct NAME __pragma(pack(pop))
#else
#	define PACKED_STRUCT(NAME) struct CC_ATTR(__packed__) NAME
#endif

#define STATIC_ASSERT(CONDITION,...) \
	((void)sizeof(char[1 - 2 * !(CONDITION)]))

#define STATIC_NEGATE(NUM) (\
	STATIC_ASSERT(((typeof(NUM))(-1)) < 0), \
	(typeof(NUM))((NUM)*-1) \
	)

#define AS_UPTR(PTR) \
	((uintptr_t)(PTR))

#define AS_OFFSET(OFF) \
	((ptrdiff_t)(OFF))

#define OFFSET_PTR(PTR,OFF) \
	(typeof(PTR))( \
		(AS_OFFSET(OFF) < 0) ? \
		(AS_UPTR(PTR) - AS_UPTR(STATIC_NEGATE(AS_OFFSET(OFF)))) : \
		(AS_UPTR(PTR) + AS_UPTR(OFF)) \
	)

#ifdef __cplusplus
extern "C" {
#endif

// Misc utils
bool is_folder(const char *path);
bool is_regular_file(const char *path);
int proc_path(void *buffer, size_t buffersize);

#ifdef __cplusplus
}
#endif

#endif /* _SHARED_H_ */
