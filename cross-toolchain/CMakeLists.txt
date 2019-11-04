include_directories(.)

set(CROSS_CONFIGURE "${CROSS_TRIPLE}-configure")
set(CROSS_CMAKE_TARGET "${CROSS_TRIPLE}-cmake")
set(CROSS_CMAKE_TOOLCHAIN "${CROSS_TRIPLE}-toolchain.cmake")

add_executable(${CROSS_CMAKE_TARGET} cross-cmake.c)
target_link_libraries(${CROSS_CMAKE_TARGET} cygshared)

install(PROGRAMS cross-configure
        DESTINATION "bin"
        RENAME ${CROSS_CONFIGURE})

install(TARGETS ${CROSS_CMAKE_TARGET}
        DESTINATION "bin")

configure_file(toolchain.cmake.in toolchain.cmake @ONLY)

install(FILES "${CMAKE_CURRENT_BINARY_DIR}/toolchain.cmake"
        DESTINATION "bin"
        RENAME ${CROSS_CMAKE_TOOLCHAIN})
