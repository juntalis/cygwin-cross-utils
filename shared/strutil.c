/**
 * @file strutil.c
 * @author Charles Grunwald <cgrunwald@gmail.com>
 */
#include "strutil.h"


#define LC_MASK_B 0x20
#define LC_MASK_W 0x2020
#define LC_MASK_D 0x20202020
#define LC_MASK_Q 0x2020202020202020

typedef union 
{
	uint8_t*  pB;
	uint16_t* pW;
	uint32_t* pD;
	uint64_t* pQ;
	uintptr_t pN;
} substr_ptr_t;

char* strlower(char* dest, const char* src, size_t length)
{
	substr_ptr_t isrc, idest;
	uintptr_t pend = AS_UPTR(OFFSET_PTR(src, --length));
	isrc.pB = (uint8_t*)src;
	idest.pB = (uint8_t*)dest;
	
#	define SIZED_COPY(SZ) \
		while( isrc.pN < pend ) { \
			* CPP_PASTE(idest.p,SZ) ++ = *CPP_PASTE(isrc.p,SZ) ++ | CPP_PASTE(LC_MASK_,SZ); \
		}
	
	SIZED_COPY(Q);
	SIZED_COPY(D);
	SIZED_COPY(W);
	SIZED_COPY(B);
#	undef SIZED_COPY
	
	dest[length+1] = '\0';
	return dest;
}

#if !defined(_WIN32)

int _vscprintf(const char* format, va_list pargs)
{
	int retval;
	va_list argcopy;
	va_copy(argcopy, pargs);
	retval = vsnprintf(NULL, 0, format, argcopy);
	va_end(argcopy);
	return retval;
}

#endif

char* vsprintf_alloc(const char* format, va_list pargs)
{
	char* buffer;
	
	// Check the resulting size of the buffer.
	size_t count = (size_t)_vscprintf(format, pargs) + 1;
	
	// Allocate our buffer.
	buffer = (char*)malloc(count);
	if(buffer == NULL) {
		return NULL;
	} else {
		memset((void*)buffer, 0, count);
	}
	
	// Finally, fill in the message.
	vsnprintf(buffer, count, format, pargs);
	return buffer;
}

// Only used once or twice, thus the inline
char* sprintf_alloc(const char* format, ...)
{
	va_list args;
	char* buffer = NULL;
	// Allocate buffer for our resulting format string.
	va_start(args, format);
	buffer = vsprintf_alloc(format, args);
	va_end(args);
	return buffer;
}

char* xstrrchr(char* subject, size_t subject_len, char needle) // NOLINT
{
	ssize_t c = (ssize_t)subject_len;
	if(subject && subject_len > 0) {
		while(--c >= 0) {
			if(subject[c] == needle) {
				return &(subject[c]);
			}
		}
	}
	return NULL;
}

char* xstrrstr(char* subject, size_t subject_len, const char* needle, size_t needle_len) // NOLINT
{
	if(subject && subject_len > 0 && needle && needle_len > 0 && subject_len >= needle_len) {
		ssize_t c = (ssize_t)subject_len - (ssize_t)needle_len;
		do {
			const char* substr = (const char*)&(subject[c]);
			if(strncmp(substr, needle, needle_len) == 0) {
				return (char*)substr;
			}
		} while(--c >= 0);
	}
	return NULL;
}

