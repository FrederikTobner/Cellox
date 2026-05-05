set(_windows_impl "${CMAKE_SOURCE_DIR}/src/conditionals/os/windows/impl")
set(CLX_OS_SOURCES
    ${_windows_impl}/fs.c
    ${_windows_impl}/path.c
    ${_windows_impl}/platform.c
    ${_windows_impl}/stdio.c
    ${_windows_impl}/temp.c
    ${_windows_impl}/time.c
)
add_compile_definitions(OS_WINDOWS)

function(clx_os_check_core_headers)
    CHECK_INCLUDE_FILE("conio.h" CONIO_AVAILABLE)
    CHECK_INCLUDE_FILE("windows.h" WINDOWS_AVAILABLE)

    if(NOT CONIO_AVAILABLE)
        message(FATAL_ERROR "conio.h is required to build the compiler under windows")
    endif()
    if(NOT WINDOWS_AVAILABLE)
        message(FATAL_ERROR "windows.h is required to build the compiler under windows")
    endif()
endfunction()

function(clx_os_check_benchmark_headers)
    CHECK_INCLUDE_FILE("windows.h" WINDOWS_AVAILABLE)

    if(NOT WINDOWS_AVAILABLE)
        message(FATAL_ERROR "windows.h is required to build the benchmark runner under windows")
    endif()
endfunction()