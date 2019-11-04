project(cygwin-utils C)

cmake_minimum_required(VERSION 3.6)

set(PROJECT_DESCRIPTION "Cygwin-based tools for facilitating cross compilation to our Ubuntu servers")
message(STATUS "Running CMake for ${PROJECT_NAME} (${PROJECT_DESCRIPTION})")

set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake;${CMAKE_MODULE_PATH}")

include(CheckCCompilerFlag)
include(CheckCSourceCompiles)
include(CheckFunctionExists)
include(CheckIncludeFile)
include(EnableCFlag)
include(TrySanitizer)

if (NOT CMAKE_BUILD_TYPE)
	message(STATUS "No build type selected, defaulting to Debug")
	set(CMAKE_BUILD_TYPE "Debug")
endif ()

# Configuration
set(CROSS_PROCESSOR
    "x86_64"
    CACHE STRING
    "The name of the CPU your cross-compiler is targeting. (ex: x86_64)"
)
set(CROSS_SYSTEM_NAME
    "Linux"
    CACHE STRING
    "The name of the operating system that your compiler is targeting. (ex: Linux)"
)
set(CROSS_TRIPLE
    "x86_64-ubuntu16.04-linux-gnu"
    CACHE STRING
    "Triple used to describe the target of our cross-compile. (ex: x86_64-linux-gnu)"
)

# Ensure compiler is compatible with GNU99 standard
check_c_compiler_flag(-std=gnu99 HAS_STD_GNU99)
if (NOT HAS_STD_GNU99)
	message(FATAL_ERROR "Compiler does not support -std=gnu99. Consider using a newer compiler")
endif()

set(CMAKE_C_STANDARD 99)

# Check for GCC builtin functions
check_c_source_compiles("int main(void) { __builtin_cpu_init(); }" HAVE_BUILTIN_CPU_INIT)
check_c_source_compiles("int main(void) { __builtin_clzll(0); }" HAVE_BUILTIN_CLZLL)
check_c_source_compiles("int main(void) { unsigned long long p; (void)__builtin_mul_overflow(0, 0, &p); }" HAVE_BUILTIN_MUL_OVERFLOW)
check_c_source_compiles("int main(void) { unsigned long long p; (void)__builtin_add_overflow(0, 0, &p); }" HAVE_BUILTIN_ADD_OVERFLOW)
check_c_source_compiles("int main(void) { _Static_assert(1, \"\"); }" HAVE_STATIC_ASSERT)

# Enable if available
enable_c_flag_if_avail(-fno-plt CMAKE_C_FLAGS HAS_NO_PLT)
enable_c_flag_if_avail(-mtune=native C_FLAGS_REL HAS_MTUNE_NATIVE)

if (${CMAKE_BUILD_TYPE} MATCHES "Rel")
	enable_c_flag_if_avail(-malign-data=abi C_FLAGS_REL HAS_ALIGN_DATA)
	enable_c_flag_if_avail(-fno-asynchronous-unwind-tables C_FLAGS_REL HAS_NO_ASYNC_UNWIND_TABLES)
	enable_c_flag_if_avail(-flto C_FLAGS_REL HAS_LTO)
	enable_c_flag_if_avail(-ffat-lto-objects C_FLAGS_REL HAS_LTO_FAT_OBJS)
	enable_c_flag_if_avail(-mcrc32 C_FLAGS_REL HAVE_BUILTIN_IA32_CRC32)
else ()
	option(SANITIZER "Use sanitizer (undefined, address, thread, none)" "none")
	mark_as_advanced(SANITIZER)
	
	if (${SANITIZER} MATCHES "(undefined|ub|ubsan)")
		try_sanitizer("undefined")
	elseif (${SANITIZER} MATCHES "(address|memory)")
		try_sanitizer("address")
	elseif (${SANITIZER} MATCHES "(thread|race)")
		try_sanitizer("thread")
	else ()
		message(STATUS "Building without a sanitizer")
	endif ()
endif ()

#
# These warnings are only supported by GCC, and some only in newer versions.
#
enable_warning_if_supported(-Wduplicated-cond)
enable_warning_if_supported(-Wduplicated-branches)
enable_warning_if_supported(-Wlogical-op)
enable_warning_if_supported(-Wrestrict)
enable_warning_if_supported(-Wdouble-promotion)
enable_warning_if_supported(-Wno-unused-parameter)

# Set C flags
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra -Wshadow -Wconversion -std=gnu99")
set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} ${C_FLAGS_REL}")
set(CMAKE_C_FLAGS_RELWITHDEBINFO "${CMAKE_C_FLAGS_RELWITHDEBINFO} ${C_FLAGS_REL}")
set(CMAKE_C_FLAGS_MINSIZEREL "${CMAKE_C_FLAGS_MINSIZEREL} ${C_FLAGS_REL}")
add_definitions("-D_FILE_OFFSET_BITS=64")

include_directories(shared)

add_subdirectory(shared)
add_subdirectory(cross-toolchain)
