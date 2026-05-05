set(_unix_impl "${CMAKE_SOURCE_DIR}/src/conditionals/os/unix/impl")
set(_linux_impl "${CMAKE_SOURCE_DIR}/src/conditionals/os/unix/linux/impl")
set(CLX_OS_SOURCES
    ${_unix_impl}/fs.c
    ${_unix_impl}/path.c
    ${_unix_impl}/stdio.c
    ${_unix_impl}/temp.c
    ${_unix_impl}/time.c
    ${_linux_impl}/platform.c
)
add_compile_definitions(OS_UNIX_LIKE OS_LINUX)

function(clx_os_check_core_headers)
    CHECK_INCLUDE_FILE("curses.h" CURSES_AVAILABLE)
    CHECK_INCLUDE_FILE("unistd.h" UNISTD_AVAILABLE)

    if(NOT CURSES_AVAILABLE)
        message(FATAL_ERROR "curses.h is required to build the compiler under linux")
    endif()
    if(NOT UNISTD_AVAILABLE)
        message(FATAL_ERROR "unistd.h is required to build the compiler under linux")
    endif()
endfunction()

function(clx_os_check_benchmark_headers)
    CHECK_INCLUDE_FILE("dirent.h" DIRENT_AVAILABLE)
    if(NOT DIRENT_AVAILABLE)
        message(FATAL_ERROR "dirent.h is required to build the benchmark runner under linux")
    endif()
endfunction()