#include <stdlib.h>
#include <stdarg.h>
#include <ctype.h>

#include "shared.h"
#include "strbuf.h"

#define ENV_SEP ":"
#define PATH_SEP_STR "/"
#define PATH_SEP_CHR '/'

#define UNAME_SUFFIX "-cmake"
#define DEBUG_ENVNAME "CROSS_DEBUG"
#define TOOLCHAIN_PATH_SUFFIX "-toolchain.cmake"

#define TOOLCHAIN_ARG "-DCMAKE_TOOLCHAIN_FILE="
#define INSTALL_PREFIX_ARG "-DCMAKE_INSTALL_PREFIX="

#define CYGWIN_WIN32_ARG "-DWIN32=0"
#define CYGWIN_LEGACY_ARG "-DCMAKE_LEGACY_CYGWIN_WIN32=0"

#define CMAKE_ARGS_COUNT 3

#define MIN(X, Y) (((X) < (Y)) ? (X) : (Y))
#define MAX(X, Y) (((X) > (Y)) ? (X) : (Y))
#define WHICH_HANDLER(FN) ((which_handler)(FN))

struct strref
{
	size_t len;
	const char* value;
};

struct exe_paths
{
	struct strref abspath;
	struct strref bindir;
	struct strref prefix;
	struct strref uname;
};

struct exec_args
{
	int argc;
	char** argv;
};

typedef bool(*which_handler)(struct strref* path, void* userdata);

static ALWAYS_INLINE char* get_cygwin_win32_arg(void)
{
	return strndup(CYGWIN_WIN32_ARG, sizeof(CYGWIN_WIN32_ARG) - 1);
}

static ALWAYS_INLINE char* get_cygwin_legacy_arg(void)
{
	return strndup(CYGWIN_LEGACY_ARG, sizeof(CYGWIN_LEGACY_ARG) - 1);
}

static NORET fatal_error(int code, const char* label)
{
	printf("ERROR: %s: %s\n", label, strerror(code));
	exit(code);
}

static NORET fatal_message(int code, const char* format, ...)
{
	va_list args;
	printf("ERROR: ");
	va_start(args, format);
	vprintf(format, args);
	va_end(args);
	printf("\n");
	exit(code != 0 ? code : 1);
}

static bool is_debug_mode(void)
{
	static bool is_debug = false;
	static bool first_call = true;
	if(first_call) {
		char* debug_env = NULL;
		first_call = false;
		debug_env = getenv(DEBUG_ENVNAME);
		is_debug = debug_env != NULL && (*debug_env == '1');
	}
	return is_debug;
}



static void debuglog(const char* format, ...)
{
	if(is_debug_mode()) {
		va_list args;
		printf("DEBUG: ");
		va_start(args, format);
		vprintf(format, args);
		va_end(args);
		printf("\n");
	}
}

static ALWAYS_INLINE void strref_init(struct strref* self, const char* value)
{
	self->value = value;
	self->len = (size_t)strlen(value);
}

static ALWAYS_INLINE int strref_cmp(struct strref* self, struct strref* other)
{
	return self->len == other->len ? strcmp(self->value, other->value) : (int)(self->len - other->len);
}

static ALWAYS_INLINE char* strref_strndup(struct strref* self)
{
	return strndup(self->value, self->len);
}

static ALWAYS_INLINE char* strref_strncpy(struct strref* self, char* buffer, size_t *buffer_len)
{
	*buffer_len = MIN(*buffer_len, self->len);
	return strncpy(buffer, self->value, *buffer_len);
}

static ALWAYS_INLINE bool which_handled(struct strref* path, which_handler handler, void* userdata)
{
	return is_regular_file(path->value) && access(path->value, X_OK) == 0 && handler(path, userdata);
}

static char* which_path(const char* name, const char* envpath, which_handler handler, void* userdata)
{
	char filepath[PATH_MAX] = "";
	char *paths, *saveptr, *tok = NULL, *result = NULL;
	struct strref path_value = { 0, (const char*)&filepath[0] };
	
	// Duplicate environment path.
	paths = strdup(envpath);
	if(paths == NULL) {
		fatal_error(errno, "malloc");
	}
	
	tok = strtok_r(paths, ENV_SEP, &saveptr);
	if(tok != NULL) {
		do {
			path_value.len = (size_t)snprintf(filepath, PATH_MAX, "%s" PATH_SEP_STR "%s", tok, name);
			debuglog("  + Checking %s...", filepath);
			if(which_handled(&path_value, handler, userdata)) {
				result = strdup(filepath);
				break;
			}
			
			path_value.len += 4;
			if((path_value.len + 1) > PATH_MAX)
				continue;
			
			strcat(filepath, ".exe");
			debuglog("  + Checking %s...", filepath);
			if(which_handled(&path_value, handler, userdata)) {
				result = strdup(filepath);
				break;
			}
		} while((tok = strtok_r(NULL, ENV_SEP, &saveptr)) != NULL);
	}
	
	free(paths);
	return result;
}

static ALWAYS_INLINE char* which(const char* name, which_handler handler, void* userdata)
{
	return which_path(name, getenv("PATH"), handler, userdata);
}

static bool handle_cmake_path(struct strref* path, struct strref* exe)
{
	bool result;
	debuglog("Verifying '%s' != '%s'...", path->value, exe->value);
	result = strref_cmp(path, exe) != 0;
	debuglog("  => %s", result ? "true" : "false");
	return result;
}

static ALWAYS_INLINE char* find_cmake_exe(struct strref* exe_path)
{
	debuglog("Attempting to resolve cmake executable path...");
	return which("cmake", WHICH_HANDLER(handle_cmake_path), exe_path);
}

static void exe_paths_reset(struct exe_paths* paths)
{
	if(paths != NULL) {
		if(paths->abspath.value) {
			free((void*)paths->abspath.value);
			memset((void*)&(paths->abspath), 0, sizeof(paths->abspath));
		}
		if(paths->prefix.value) {
			free((void*)paths->prefix.value);
			memset((void*)&(paths->prefix), 0, sizeof(paths->prefix));
		}
		if(paths->bindir.value) {
			free((void*)paths->bindir.value);
			memset((void*)&(paths->bindir), 0, sizeof(paths->bindir));
		}
		if(paths->prefix.value) {
			free((void*)paths->prefix.value);
			memset((void*)&(paths->prefix), 0, sizeof(paths->prefix));
		}
		if(paths->uname.value) {
			free((void*)paths->uname.value);
			memset((void*)&(paths->uname), 0, sizeof(paths->uname));
		}
	}
}

static void exe_paths_init(struct exe_paths* paths, const char* exe)
{
	char buffer[PATH_MAX] = "";
	char *psearch, *pbase, *pdir = &(buffer[0]);
	size_t dir_len, base_len, buffer_len = PATH_MAX - 1;
	
	// Initialize the path containing our executable
	debuglog("Initializing exe abspath...");
	strref_init(&paths->abspath, strdup(exe));
	
	// Copy the path to a local buffer
	strref_strncpy(&paths->abspath, pdir, &buffer_len);
	
	// Resolve the location of the last '/' in the process path.
	debuglog("Locating the path of our bin folder...");
	pbase = xstrrchr(buffer, buffer_len, PATH_SEP_CHR);
	if(pbase == NULL) {
		fatal_message(1, "Failed to resolve parent folder of: %s", buffer);
	}
	
	// Based on that, figure out the length of our bin folder path, then
	// terminate it.
	dir_len = (size_t)pbase - (size_t)pdir;
	*pbase++ = '\0';
	base_len = buffer_len - dir_len - 1;
	debuglog("  => '%s'", pdir);
	
	// Store the bindir
	paths->bindir.len = dir_len;
	paths->bindir.value = (const char*)strndup(pdir, dir_len);
	
	// Resolve the cross compiler's prefix folder.
	debuglog("Locating our executable's prefix path...");
	psearch = xstrrchr(pdir, dir_len, PATH_SEP_CHR);
	if(psearch == NULL) {
		exe_paths_reset(paths);
		fatal_message(1, "Failed to resolve parent folder of: %s", pdir);
	}
	
	*psearch = '\0';
	paths->prefix.len = (size_t)psearch - (size_t)pdir;
	paths->prefix.value = (const char*)strndup(pdir, paths->prefix.len);
	debuglog("  => %s", paths->prefix.value);
	
	// Resolve the uname for tha cross compiler's target.
	debuglog("Resolving the target uname of our cross compiler...");
	psearch = xstrrstr(pbase, base_len, UNAME_SUFFIX, sizeof(UNAME_SUFFIX) - 1);
	if(psearch == NULL) {
		exe_paths_reset(paths);
		fatal_message(1, "Failed to resolve the end of our target uname string!", pdir);
	}
	
	// Set our field.
	paths->uname.len = (size_t)psearch - (size_t)pbase;
	paths->uname.value = (const char*)strndup(pbase, paths->uname.len);
	debuglog("  => %s", paths->uname.value);
}

static char* resolve_toolchain_arg(struct exe_paths* paths)
{
	size_t path_len, arg_len;
	char path_buffer[PATH_MAX] = "";
	char arg_buffer[PATH_MAX + sizeof(TOOLCHAIN_ARG)] = "";
	
	// First resolve and check the CMake toolchain file..
	debuglog("Resolving CMake toolchain file...");
	path_len = (size_t)snprintf(path_buffer, PATH_MAX, "%s/%s" TOOLCHAIN_PATH_SUFFIX, paths->bindir.value, paths->uname.value);
	debuglog("  => '%s'", path_buffer);
	debuglog("Checking toolchain file...");
	if(access(path_buffer, R_OK) != 0) {
		int code = errno;
		exe_paths_reset(paths);
		fatal_message(code, "Failed to locate cross compiler's CMake toolchain file: %s!", path_buffer);
	}
	debuglog("  => OK");

	// Then format the path into our toolchain definition.
	arg_len = (size_t)snprintf(arg_buffer, sizeof(arg_buffer), TOOLCHAIN_ARG "%s", path_buffer);
	assert(arg_len == path_len + sizeof(TOOLCHAIN_ARG) - 1);
	return strndup((const char*)arg_buffer, arg_len);
}

static char* resolve_install_prefix_arg(struct exe_paths* paths)
{
	size_t path_len, arg_len;
	char path_buffer[PATH_MAX] = "";
	char arg_buffer[PATH_MAX + sizeof(INSTALL_PREFIX_ARG)] = "";
	
	// First resolve and check the install prefix folder
	debuglog("Resolving install prefix...");
	path_len = (size_t)snprintf(path_buffer, PATH_MAX, "%s/%s/sysroot/usr", paths->prefix.value, paths->uname.value);
	debuglog("  => '%s'", path_buffer);
	debuglog("Checking install prefix existence...");
	if(!is_folder(path_buffer)) {
		int code = errno;
		exe_paths_reset(paths);
		fatal_message(code, "Failed to locate install prefix: %s!", path_buffer);
	}
	debuglog("  => OK");

	// Then format the path into our install_prefix definition.
	arg_len = (size_t)snprintf(arg_buffer, sizeof(arg_buffer), INSTALL_PREFIX_ARG "%s", path_buffer);
	assert(arg_len == path_len + sizeof(INSTALL_PREFIX_ARG) - 1);
	return strndup((const char*)arg_buffer, arg_len);
}

static void free_child_args(struct exec_args* args)
{
	if(args) {
		for(int i = 0; i < args->argc; i++) {
			if(args->argv[i] != NULL) {
				free((void*)args->argv[i]);
				args->argv[i] = NULL;
			}
		}
	}
}

static int exec_cmake_generate(const char* exe, int argc, char** argv)
{
	int argi, retcode = 0;
	size_t sz_args = 0;
	char* cmake_path = NULL;
	char* install_def = NULL;
	char* toolchain_def = NULL;
	struct exe_paths exe_paths = { {0}, {0}, {0}, {0} };
	struct exec_args child_args = { argc + CMAKE_ARGS_COUNT + 1, NULL };
	
	exe_paths_init(&exe_paths, exe);
	
	// Locate cmake executable.
	cmake_path = find_cmake_exe(&exe_paths.abspath);
	if(cmake_path == NULL) {
		exe_paths_reset(&exe_paths);
		fatal_message(ENOENT, "Failed to locate cmake executable!");
	}
	
	// Resolve the command line arguments.
	install_def = resolve_install_prefix_arg(&exe_paths);
	toolchain_def = resolve_toolchain_arg(&exe_paths);
	
	// Alloc our child args array
	sz_args = sizeof(char*) * (size_t)child_args.argc;
	child_args.argv = (char**)malloc(sz_args);
	if(child_args.argv == NULL) {
		exe_paths_reset(&exe_paths);
		free((void*)cmake_path);
		free((void*)install_def);
		free((void*)toolchain_def);
		fatal_error(errno, "exec_cmake_generate:malloc");
	} else {
		memset((void*)child_args.argv, 0, sz_args);
	}
	
	// Populate our child args array
	child_args.argv[0] = cmake_path;
	child_args.argv[1] = toolchain_def;
	child_args.argv[2] = install_def;
	child_args.argv[3] = get_cygwin_win32_arg();
//	child_args.argv[4] = get_cygwin_legacy_arg();
	for(argi = 1; argi < argc; argi++) {
		child_args.argv[CMAKE_ARGS_COUNT+argi] = strdup(argv[argi]);
	}
	
	retcode = execv((const char*)child_args.argv[0], child_args.argv);
	assert(retcode != -1);
	
	free_child_args(&child_args);
	exe_paths_reset(&exe_paths);
	return retcode;
}

static int exec_cmake_passthru(const char* exe, int argc, char** argv)
{
	int argi, retcode = 0;
	size_t sz_args = 0;
	char* cmake_path = NULL;
	struct exe_paths exe_paths = { {0}, {0}, {0}, {0} };
	struct exec_args child_args = { argc + 1, NULL };
	
	exe_paths_init(&exe_paths, exe);
	
	// Locate cmake executable.
	cmake_path = find_cmake_exe(&exe_paths.abspath);
	if(cmake_path == NULL) {
		exe_paths_reset(&exe_paths);
		fatal_message(ENOENT, "Failed to locate cmake executable!");
	}
	
	// Alloc our child args array
	sz_args = sizeof(char*) * (size_t)child_args.argc;
	child_args.argv = (char**)malloc(sz_args);
	if(child_args.argv == NULL) {
		exe_paths_reset(&exe_paths);
		free((void*)cmake_path);
		fatal_error(errno, "exec_cmake_passthru:malloc");
	} else {
		memset((void*)child_args.argv, 0, sz_args);
	}
	
	// Populate our child args array
	child_args.argv[0] = cmake_path;
	printf(child_args.argv[0]);
	for(argi = 1; argi < argc; argi++) {
		child_args.argv[argi] = strdup(argv[argi]);
		printf(" %s", child_args.argv[argi]);
	}
	printf("\n");
	
	
	retcode = execv((const char*)child_args.argv[0], child_args.argv);
	assert(retcode != -1);
	
	free_child_args(&child_args);
	exe_paths_reset(&exe_paths);
	return retcode;
}

#define USHIFT(SIZE,VALUE,COUNT) \
	(((uint ## SIZE ## _t)(VALUE)) << ((uint ## SIZE ## _t)(COUNT)))

#define AS_U16_ARG(A,B) \
	( USHIFT(16,B,8) | ((uint16_t)(A)) )

#define AS_U32_ARG(A,B,C,D) \
	( USHIFT(32,D,24) | \
	  USHIFT(32,C,16) | \
	  USHIFT(32,B,8)  | \
	  ((uint32_t)(A))  )


static ALWAYS_INLINE bool is_cmake_command_u16(uint16_t arg)
{
	switch(arg)
	{
		case AS_U16_ARG('-', 'E'): // NOLINT
		case AS_U16_ARG('-', 'L'): // NOLINT
		case AS_U16_ARG('-', 'N'): // NOLINT
		case AS_U16_ARG('-', 'P'): // NOLINT
		case AS_U16_ARG('-', 'h'): // NOLINT
		case AS_U16_ARG('-', 'H'): // NOLINT
		case AS_U16_ARG('/', '?'): // NOLINT
		case AS_U16_ARG('-', 'u'): // NOLINT
		case AS_U16_ARG('-', 'v'): // NOLINT
		case AS_U16_ARG('/', 'v'): // NOLINT
			return true;
		default:
			return false;
	}
}

static ALWAYS_INLINE bool is_cmake_command_u32(uint32_t* parg)
{
	uint32_t arg = *parg;
	switch(arg)
	{
		case AS_U32_ARG('v','e','r','s'):
		case AS_U32_ARG('b','u','i','l'):
		case AS_U32_ARG('f','i','n','d'):
		case AS_U32_ARG('g','r','a','p'):
		case AS_U32_ARG('s','y','s','t'):
		case AS_U32_ARG('c','h','e','c'):
		case AS_U32_ARG('h','e','l','p'):
			return true;
		case AS_U32_ARG('d','e','b','u'): {
			char* postdebug = ((char*)(parg)) + sizeof("debug");
			arg = *((uint32_t*)postdebug);
			return arg == AS_U32_ARG('t','r','y','c');
		}
		default:
			return false;
	}
}

static bool is_cmake_generate(int argc, char* argv[])
{
	int i = 1;
	debuglog("Attempting to identify if cmake was run in generation mode...");
	for(char* arg = argv[i]; i < argc; arg = argv[++i]) {
		size_t arglen;
		uint32_t* u32arg;
		uint16_t* u16arg = (uint16_t*)arg;
		
		debuglog("  + check('%s')...", arg);
		arglen = (size_t)strlen(arg);
		if(arglen >= 2 && is_cmake_command_u16(*u16arg)) {
			debuglog("  => NO");
			return false;
		}
		
		u32arg = (uint32_t*)(arg + 2);
		if(arglen >= 6 && arg[1] == '-' && is_cmake_command_u32(u32arg)) {
			debuglog("  => NO");
			return false;
		}
	}
	debuglog("  => OK");
	return true;
}


int main(int argc, char** argv)
{
	char exe_buffer[PATH_MAX] = {0};
	
	debuglog("Looking up our process's filepath..");
	if(proc_path(exe_buffer, PATH_MAX) != 0) {
		fatal_error(errno, "proc_path");
	} else {
		debuglog("  => %s", exe_buffer);
	}
	
	if(is_cmake_generate(argc, argv)) {
		return exec_cmake_generate((const char*)exe_buffer, argc, argv);
	} else {
		return exec_cmake_passthru((const char*)exe_buffer, argc, argv);
	}
}
