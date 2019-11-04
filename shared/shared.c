/**
 * @file shared.c
 * @author Charles Grunwald <cgrunwald@gmail.com>
 */
#include <sys/stat.h>
#include "shared.h"

bool is_regular_file(const char *path)
{
	struct stat path_stat;
	stat(path, &path_stat);
	return S_ISREG(path_stat.st_mode) ? true : false; // NOLINT
}

bool is_folder(const char *path)
{
	struct stat path_stat;
	stat(path, &path_stat);
	return S_ISDIR(path_stat.st_mode) ? true : false; // NOLINT
}

ALWAYS_INLINE int proc_path(void* buffer, size_t buffersize)
{
	ssize_t path_len = 0;
	path_len = readlink("/proc/self/exe", buffer, buffersize);
	if(path_len < 0) {
		return -1;
	}
	
	if(path_len < (ssize_t)buffersize) {
		((char*)buffer)[path_len] = '\0';
		return 0;
	}
	
	errno = EOVERFLOW;
	return -1;
}
